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
    // filename << "log" << this->getConnectionId() << ".txt";
    // setupLogFile(filename.str().c_str());
}

DistributedMonitor::~DistributedMonitor() {
    std::cout << "JOIN\n";
    this->listenThread.join();
}

int DistributedMonitor::getConnectionId() {
    return this->connectionManager->getId();
}

std::mutex& DistributedMonitor::getMutex() {
    return mutex;
}

std::condition_variable &DistributedMonitor::getCv() {
    return cv;
}

void DistributedMonitor::updateLamportClock() {
    this->lamportClock++;
}

void DistributedMonitor::updateLamportClock(int newValue) {
    this->lamportClock = std::max(this->lamportClock, newValue) + 1;
}

void DistributedMonitor::changeState(State state) {
    mutexMap["state"].lock();
    this->state = state;
    std::cout << getConnectionId() << getCaseName() << ": state " << state << std::endl;
    mutexMap["state"].unlock();
}

void DistributedMonitor::sendMessage(std::shared_ptr<Message> message) {
    if (message->getReceiversId() == NOT_SET) {
        std::cerr << "Receivers id not set!\n";
        throw "Receivers id not set!\n";
        return;
    }
    connectionManager->sendMessage(message);
    std::cout << "Message sent!" << std::endl;
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
    int myId = connectionManager->getId();
    int clientsCount = connectionManager->getClientsCount();
    if (waitForReply)
        setMessageAsMyNotFulfilledRequest(std::make_shared<Message>(*message), (clientsCount - 1));
    for (int i = 0; i < clientsCount; i ++) {
        if (myId != i) {
            message->setReceiversId(i);
            sendMessage(message);
        }
    }
    return lamportClock;
}

void DistributedMonitor::setMessageAsMyNotFulfilledRequest(std::shared_ptr<Message> message, int counter) {
    mutexMap["myNotFulfilledRequest"].lock();
    myRequest request(message->getSendersClock(), counter);
    myNotFulfilledRequest = request;
    mutexMap["myNotFulfilledRequest"].unlock();
}

/*
 * listen() - function called on listenThread. It manages all received messages.
 */

void DistributedMonitor::sendResponse(int receiverId, int requestClock) {
    std::shared_ptr<Message> msg = std::make_shared<Message>
            (this->getConnectionId(),
             Message::MessageType::LOCK_RESPONSE,
             receiverId,
             requestClock);
    this->sendSingleMessage(msg, false);
}

void DistributedMonitor::l_lock(int receiverId, int requestClock) {
    std::cout << getConnectionId() << getCaseName() << ": l_lock()?\n";
    this->mutex.lock();
    std::cout << getConnectionId() << getCaseName() << ": l_lock OK\n";
    sendResponse(receiverId, requestClock);
}

void DistributedMonitor::addToRequestsFromOthersQueue(Message *receivedMessage) {
    mutexMap["requestsFromOthersQueue"].lock();
    requestsFromOthersQueue.push(*receivedMessage);
    mutexMap["requestsFromOthersQueue"].unlock();
}


void DistributedMonitor::reactForLockRequest(Message *receivedMessage) {
    mutexMap["state"].lock();
    switch (this->state) {
        case State::WAITING_FOR_REPLIES: {
            if (receivedMessage->getSendersClock() < myNotFulfilledRequest.clock
                or (receivedMessage->getSendersClock() == myNotFulfilledRequest.clock
                     and receivedMessage->getSendersId() < this->getConnectionId())) {
                // our clock is worse (greater) or our id is worse: lock mutex
                l_lock(receivedMessage->getSendersId(), receivedMessage->getSendersClock());
            }
            else {
                // we are better
                addToRequestsFromOthersQueue(receivedMessage);
            }
            break;
        }
        case State::IN_CRITICAL_SECTION: {
            addToRequestsFromOthersQueue(receivedMessage);
            break;
        }
        default: {
            // NOT NEEDED: SEND LOCK_RESPONSE
            l_lock(receivedMessage->getSendersId(), receivedMessage->getSendersClock());
            break;
        }
    }
    mutexMap["state"].unlock();
}

void DistributedMonitor::reactForLockResponse(Message *receivedMessage) {
    if (myNotFulfilledRequest.clock == receivedMessage->getRequestClock())
        myNotFulfilledRequest.decrementCounter();
    if (myNotFulfilledRequest.answerCounter <= 0) {
        // got all responses -> go to critical section
        cv.notify_all();
        myRequest clear;
        myNotFulfilledRequest = clear;
    }
}

void DistributedMonitor::l_unlock() {
    this->mutex.unlock();
}

void DistributedMonitor::reactForUnlock(Message *receivedMessage) {
    std::string data = receivedMessage->getData();
    manageReceivedData(data);
    l_unlock();
}

void DistributedMonitor::listen() {
    std::cout << getConnectionId() << getCaseName() << ": LISTEN!\n";
    Message message;
    while(true) {
        connectionManager->getReceiveMutex()->lock();
        if (connectionManager->tryToReceive(Message::MessageType::LOCK_MTX)) {
            message = connectionManager->receiveMessage(Message::MessageType::LOCK_MTX);
            std::cout << getConnectionId() << getCaseName() << ": received LOCK_MTX!\n";
            updateLamportClock(message.getSendersClock());
            reactForLockRequest(&message);
        }
        if (connectionManager->tryToReceive(Message::MessageType::LOCK_RESPONSE)) {
            message = connectionManager->receiveMessage(Message::MessageType::LOCK_RESPONSE);
            std::cout << getConnectionId() << getCaseName() << ": received LOCK_RESPONSE!\n";
            updateLamportClock(message.getSendersClock());
            reactForLockResponse(&message);

        }
        if (connectionManager->tryToReceive(Message::MessageType::UNLOCK_MTX)) {
            message = connectionManager->receiveMessage(Message::MessageType::UNLOCK_MTX);
            std::cout << getConnectionId() << getCaseName() << ": received UNLOCK_MTX!\n";
            updateLamportClock(message.getSendersClock());
            reactForUnlock(&message);
        }

        std::chrono::seconds sec(3);
        std::this_thread::sleep_for(sec);

        connectionManager->getReceiveMutex()->unlock();
    }
}

/*
 * Conditional Value managing
 */

bool DistributedMonitor::checkIfGotAllReplies() {
    return (myNotFulfilledRequest.clock < 0);
}

void DistributedMonitor::d_lock() {
    // SEND REQUEST FOR CRITICAL SECTION
    changeState(State::WAITING_FOR_REPLIES);
    std::cout << getConnectionId() << ": d_lock()";
    std::shared_ptr<Message> msg = std::make_shared<Message>
            (this->getConnectionId(), Message::MessageType::LOCK_MTX);
    this->sendMessageOnBroadcast(msg, true);
//    while (!checkIfGotAllReplies()) {
//        std::unique_lock<std::mutex> lk(mutex);
//        std::cout << "WAIT\n";
//        cv.wait(lk);
//    };
//    // NOW IN CRITICAL SECTION
//    changeState(State::IN_CRITICAL_SECTION);
}

void DistributedMonitor::d_unlock() {
    // SEND MESSAGE WITH CHANGED DATA AND LEAVE CRITICAL SECTION
    // send responses from requestsFromOthersQueue
    freeRequests();
    std::cout << getConnectionId() << ": d_unlock()";
    std::string data = returnDataToSend();
    changeState(State::FREE);
    std::shared_ptr<Message> msg = std::make_shared<Message>
            (this->getConnectionId(), Message::MessageType::UNLOCK_MTX, data);
    this->sendMessageOnBroadcast(msg, false);
}

void DistributedMonitor::freeRequests() {
    // TODO in new thread
    while (!requestsFromOthersQueue.empty()) {
        Message message = requestsFromOthersQueue.front();
        sendResponse(message.getSendersId(), message.getSendersClock());
        mutexMap["requestsFromOthersQueue"].lock();
        requestsFromOthersQueue.pop();
        mutexMap["requestsFromOthersQueue"].unlock();
    }
}









