#include <iostream>
#include "DistributedMonitor.h"


DistributedMonitor::DistributedMonitor(std::shared_ptr<ConnectionManager> connectionManager) :
        connectionManager(std::move(connectionManager)) {
     this->listenThread = std::thread(&DistributedMonitor::listen, this);
}

DistributedMonitor::~DistributedMonitor() {
    std::cout << getClientId() << getUniqueConnectionNo() << ": JOIN?\n";
    this->listenThread.join();
    std::cout << getClientId() << getUniqueConnectionNo() << ": JOIN!\n";

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
    mutexMap["myNotFulfilledRequest"].lock();
    assert(request.clock >= 0);
    myNotFulfilledRequest = request;
    mutexMap["myNotFulfilledRequest"].unlock();
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
        std::cout << getClientId() << getUniqueConnectionNo() << ": WAIT\n";
        if (!checkIfGotAllReplies(thisMessageClock))
            // todo jak teraz zrobi decrement to kupa więc jest w critical-section
            cvMap["gotAllReplies"].wait(lk);    // odpuszcza critical-section
    };
    std::cout << getClientId() << getUniqueConnectionNo() << ": GLOBAL['critical-section'].lock()\n";

    // NOW IN CRITICAL SECTION
    mutexMap["state"].lock();
    changeState(State::IN_CRITICAL_SECTION);
    myRequest clear;
    // TODO myNot niepotrzebny???
    mutexMap["myNotFulfilledRequest"].lock();
    myNotFulfilledRequest = clear;
    std::cout << getClientId() << getUniqueConnectionNo() << ": clearing myNotFullfilledRequest!\n";
    mutexMap["myNotFulfilledRequest"].unlock();
    mutexMap["state"].unlock();
}

void DistributedMonitor::d_unlock() {
    // SEND MESSAGE WITH CHANGED DATA AND LEAVE CRITICAL SECTION
    // send responses from requestsFromOthersQueue
    // TODO tutaj?
    mutexMap["critical-section"].unlock();
    std::cout << getClientId() << getUniqueConnectionNo() <<": GLOBAL[\"critical-section\"].unlock()\n";
    mutexMap["state"].lock();
    freeRequests();
    changeState(State::FREE);
    mutexMap["state"].unlock();
    std::string data = returnDataToSend();
    std::shared_ptr<Message> msg = std::make_shared<Message>
            (this->getClientId(), Message::MessageType::UNLOCK_MTX, data);
    this->sendMessageOnBroadcast(msg, false);
}

void DistributedMonitor::sendLockResponse(int receiverId, int requestClock) {
    mutexMap["critical-section"].lock();
    std::cout << getClientId() << getUniqueConnectionNo() <<": LOCAL['critical-section'].lock()\n";
    std::shared_ptr<Message> msg = std::make_shared<Message>
            (this->getClientId(),
             Message::MessageType::LOCK_RESPONSE,
             receiverId,
             requestClock);
    this->sendSingleMessage(msg, false);
}

void DistributedMonitor::reactForLockRequest(Message *receivedMessage) {
    mutexMap["state"].lock();
    switch (this->state) {
        case State::WAITING_FOR_REPLIES: {
            assert(myNotFulfilledRequest.clock != -1);
            mutexMap["myNotFulfilledRequest"].lock();
            myRequest myRequest = myNotFulfilledRequest;
            mutexMap["myNotFulfilledRequest"].unlock();

            std::cout << getClientId() << getUniqueConnectionNo() << ": " << receivedMessage->getSendersClock()
                      << " <? " << myRequest.clock << "\n";
            if (receivedMessage->getSendersClock() < myRequest.clock
                or (receivedMessage->getSendersClock() == myRequest.clock
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
    mutexMap["myNotFulfilledRequest"].lock();
    if (myNotFulfilledRequest.clock == receivedMessage->getRequestClock()) {
        myNotFulfilledRequest.decrementCounter();
        // TODO
    }
    mutexMap["myNotFulfilledRequest"].unlock();
    if (checkIfGotAllReplies(receivedMessage->getRequestClock())) {
        // got all responses -> go to critical section
        cvMap["gotAllReplies"].notify_one();
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

//        std::chrono::seconds sec(3);
//        std::this_thread::sleep_for(sec);

        connectionManager->getReceiveMutex()->unlock();
    }
}

bool DistributedMonitor::checkIfGotAllReplies(int clock) {
    mutexMap["myNotFulfilledRequest"].lock();
    myRequest myRequest = myNotFulfilledRequest;
    mutexMap["myNotFulfilledRequest"].unlock();
    if (clock == myRequest.clock)
        return (myRequest.answerCounter <= 0);
    else return true;
}

void DistributedMonitor::freeRequests() {
    // TODO in new thread
    while (!requestsFromOthersQueue.empty()) {
        Message message = requestsFromOthersQueue.front();

        sendLockResponse(message.getSendersId(), message.getSendersClock());
        requestsFromOthersQueue.pop();
    }
}









