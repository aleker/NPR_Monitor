#include <iostream>
#include "DistributedMonitor.h"


// TODO setupLogFile
bool printCommonLog = true;
void setupLogFile(const char* filename) {
    std::fclose(fopen(filename, "w"));
    std::freopen(filename, "a+", stdout);
}

void createCommonLog(int id, std::string message, int clock) {
    if (printCommonLog) {
        std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()
        );
        std::cout << std::to_string(ms.count()) << " l:" << clock << " ID: " << id << " " << message << std::endl;
    }
}

DistributedMonitor::DistributedMonitor(std::shared_ptr<ConnectionManager> connectionManager) :
        connectionManager(std::move(connectionManager)) {
     this->listenThread = std::thread(&DistributedMonitor::listen, this);;
//    std::map<std::string,std::mutex>::iterator it;
//    std::string name = getClassUniqueName();
//    it = DistributedMonitor::uniqueClassNameToMutexMap.find(name);
//    if (it == DistributedMonitor::uniqueClassNameToMutexMap.end())
//        DistributedMonitor::uniqueClassNameToMutexMap[name];
    // std::stringstream filename;
    // filename << "log" << this->getClientId() << ".txt";
    // setupLogFile(filename.str().c_str());
}

DistributedMonitor::~DistributedMonitor() {
    std::cout << "JOIN\n";
    this->listenThread.join();
}

int DistributedMonitor::getClientId() {
    return this->connectionManager->getClientId();
}

int DistributedMonitor::getUniqueConnectionNo() {
    return this->connectionManager->getUniqueConnectionNo();
}

void DistributedMonitor::updateLamportClock() {
    this->lamportClock++;
}

void DistributedMonitor::updateLamportClock(int newValue) {
    this->lamportClock = std::max(this->lamportClock, newValue) + 1;
}

void DistributedMonitor::changeState(State state) {
    this->state = state;
    std::cout << getClientId() << getUniqueConnectionNo() << ": state " << state << std::endl;
}

void DistributedMonitor::sendMessage(std::shared_ptr<Message> message) {
    if (message->getReceiversId() == NOT_SET) {
        std::cerr << "Receivers id not set!\n";
        throw "Receivers id not set!\n";
        return;
    }
    connectionManager->sendMessage(message);
}

void DistributedMonitor::sendSingleMessage(std::shared_ptr<Message> message, bool waitForReply) {
    updateLamportClock();
    message->setSendersClock(this->lamportClock);
    if (waitForReply)
        setMessageAsMyNotFulfilledRequest(std::make_shared<Message>(*message), 1);
    sendMessage(message);
}

int DistributedMonitor::sendMessageOnBroadcast(std::shared_ptr<Message> message, bool waitForReply) {
    updateLamportClock();
    int lamportClock = this->lamportClock;
    message->setSendersClock(lamportClock);
    int myId = connectionManager->getClientId();
    int clientsCount = connectionManager->getClientsCount();
    if (waitForReply)
        setMessageAsMyNotFulfilledRequest(std::make_shared<Message>(*message), (clientsCount - 1));
    for (int i = 0; i < clientsCount; i ++) {
        if (myId != i) {
            message->setReceiversId(i);
            sendMessage(message);
        }
    }
    // connectionManager->sendMessageOnBroadcast(message);
    return lamportClock;
}

void DistributedMonitor::setMessageAsMyNotFulfilledRequest(std::shared_ptr<Message> message, int counter) {
    myRequest request(message->getSendersClock(), counter);
    myNotFulfilledRequest = request;
}

/*
 * listen() - function called on listenThread. It manages all received messages.
 */

void DistributedMonitor::d_lock() {
    // SEND REQUEST FOR CRITICAL SECTION
    mutexMap["state"].lock();
    changeState(State::WAITING_FOR_REPLIES);
    std::cout << getClientId() << getUniqueConnectionNo() << ": d_lock() - TRY\n";
    std::shared_ptr<Message> msg = std::make_shared<Message>
            (this->getClientId(), Message::MessageType::LOCK_MTX);
    int thisMessageClock = this->sendMessageOnBroadcast(msg, true);
    mutexMap["state"].unlock();

    // TODO if!! bo jak sprawdza to check if got all replies już jest wyczyszczony!!!
    // TODO
    while (!checkIfGotAllReplies(thisMessageClock)) {
        std::unique_lock<std::mutex> lk(mutexMap["critical-section"]);
        std::cout << getClientId() << getUniqueConnectionNo() << " WAIT\n";
        cvMap["gotAllReplies"].wait(lk);
    };
    std::cout << getClientId() << getUniqueConnectionNo() << ": GLOBAL['critical-section'].lock()\n";
    // NOW IN CRITICAL SECTION
    mutexMap["state"].lock();
    changeState(State::IN_CRITICAL_SECTION);
    mutexMap["state"].unlock();
}

void DistributedMonitor::d_unlock() {
    // SEND MESSAGE WITH CHANGED DATA AND LEAVE CRITICAL SECTION
    // send responses from requestsFromOthersQueue
    // TODO tutaj?
    std::cout << getClientId() << getUniqueConnectionNo() <<": GLOBAL[\"critical-section\"].unlock()\n";
    mutexMap["critical-section"].unlock();
    mutexMap["state"].lock();
    freeRequests();
    changeState(State::FREE);
    mutexMap["state"].unlock();
    std::string data = returnDataToSend();
    std::shared_ptr<Message> msg = std::make_shared<Message>
            (this->getClientId(), Message::MessageType::UNLOCK_MTX, data);
    this->sendMessageOnBroadcast(msg, false);
}

void DistributedMonitor::sendResponse(int receiverId, int requestClock) {
    std::shared_ptr<Message> msg = std::make_shared<Message>
            (this->getClientId(),
             Message::MessageType::LOCK_RESPONSE,
             receiverId,
             requestClock);
    this->sendSingleMessage(msg, false);
}

void DistributedMonitor::sendLockResponse(int receiverId, int requestClock) {
    mutexMap["critical-section"].lock();
    std::cout << getClientId() << getUniqueConnectionNo() <<": LOCAL['critical-section'].lock()\n";
    sendResponse(receiverId, requestClock);
}

void DistributedMonitor::reactForLockRequest(Message *receivedMessage) {
    mutexMap["state"].lock();
    switch (this->state) {
        case State::WAITING_FOR_REPLIES: {
            if (receivedMessage->getSendersClock() < myNotFulfilledRequest.clock
                or (receivedMessage->getSendersClock() == myNotFulfilledRequest.clock
                     and receivedMessage->getSendersId() < this->getClientId())) {
                // our clock is worse (greater) or our id is worse: lock mutex
                sendLockResponse(receivedMessage->getSendersId(), receivedMessage->getSendersClock());
            }
            else {
                // we are better
                requestsFromOthersQueue.push(*receivedMessage);
            }
            break;
        }
        case State::IN_CRITICAL_SECTION: {
            requestsFromOthersQueue.push(*receivedMessage);
            break;
        }
        default: {
            // NOT NEEDED: SEND LOCK_RESPONSE
            sendLockResponse(receivedMessage->getSendersId(), receivedMessage->getSendersClock());
            break;
        }
    }
    mutexMap["state"].unlock();
}

void DistributedMonitor::reactForLockResponse(Message *receivedMessage) {
    if (myNotFulfilledRequest.clock == receivedMessage->getRequestClock()) {
        myNotFulfilledRequest.decrementCounter();
    }
    if (checkIfGotAllReplies(receivedMessage->getRequestClock())) {
        // got all responses -> go to critical section
        cvMap["gotAllReplies"].notify_one();
        myRequest clear;
        myNotFulfilledRequest = clear;
    }
}

void DistributedMonitor::reactForUnlock(Message *receivedMessage) {
    std::string data = receivedMessage->getData();
    manageReceivedData(data);
    mutexMap["critical-section"].unlock();
    std::cout << getClientId() << getUniqueConnectionNo() << ": LOCAL['critical-section'].unlock()\n";
}

void DistributedMonitor::listen() {
    std::cout << getClientId() << getUniqueConnectionNo() << ": LISTEN!\n";
    Message message;
    while(true) {
        connectionManager->getReceiveMutex()->lock();
        if (connectionManager->tryToReceive(Message::MessageType::LOCK_MTX)) {
            message = connectionManager->receiveMessage(Message::MessageType::LOCK_MTX);
            std::cout << getClientId() << getUniqueConnectionNo() << ": received LOCK_MTX!\n";
            updateLamportClock(message.getSendersClock());
            reactForLockRequest(&message);
        }
        if (connectionManager->tryToReceive(Message::MessageType::LOCK_RESPONSE)) {
            message = connectionManager->receiveMessage(Message::MessageType::LOCK_RESPONSE);
            std::cout << getClientId() << getUniqueConnectionNo() << ": received LOCK_RESPONSE!\n";
            updateLamportClock(message.getSendersClock());
            reactForLockResponse(&message);

        }
        if (connectionManager->tryToReceive(Message::MessageType::UNLOCK_MTX)) {
            message = connectionManager->receiveMessage(Message::MessageType::UNLOCK_MTX);
            std::cout << getClientId() << getUniqueConnectionNo() << ": received UNLOCK_MTX!\n";
            updateLamportClock(message.getSendersClock());
            reactForUnlock(&message);
        }

        std::chrono::seconds sec(3);
        std::this_thread::sleep_for(sec);

        connectionManager->getReceiveMutex()->unlock();
    }
}

bool DistributedMonitor::checkIfGotAllReplies(int clock) {
    if (clock == myNotFulfilledRequest.clock)
        return (myNotFulfilledRequest.answerCounter <= 0);
    else return true;
}

void DistributedMonitor::freeRequests() {
    // TODO in new thread
    while (!requestsFromOthersQueue.empty()) {
        Message message = requestsFromOthersQueue.front();
        sendResponse(message.getSendersId(), message.getSendersClock());
        requestsFromOthersQueue.pop();
    }
}









