#include <iostream>
#include "DistributedMonitor.h"


DistributedMonitor::DistributedMonitor(std::shared_ptr<ConnectionManager> connectionManager) :
        connectionManager(std::move(connectionManager)) {
    this->localClientId = this->connectionManager->addNewLocalClient() * clientIdStep;
    this->listenThread = std::thread(&DistributedMonitor::listen, this);
}

DistributedMonitor::~DistributedMonitor() {
    std::cout << getDistributedClientId() << getUniqueConnectionNo() << ": JOIN?\n";
    this->listenThread.join();
    std::cout << getDistributedClientId() << getUniqueConnectionNo() << ": JOIN!\n";

}

int DistributedMonitor::getDistributedClientId() {
    return this->connectionManager->getDistributedClientId();
}

int DistributedMonitor::getLocalClientId() {
    return this->localClientId;
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
    std::cout << getDistributedClientId() << getUniqueConnectionNo() << ": state " << state << std::endl;
}

DistributedMonitor::myRequest DistributedMonitor::getMyNotFulfilledRequest() {
    mutexMap["myNotFulfilledRequest"].lock();
    myRequest request = myNotFulfilledRequest;
    mutexMap["myNotFulfilledRequest"].unlock();
    return request;
}

void DistributedMonitor::setMyNotFulfilledRequest(DistributedMonitor::myRequest request) {
    mutexMap["myNotFulfilledRequest"].lock();
    myNotFulfilledRequest = request;
    mutexMap["myNotFulfilledRequest"].unlock();
}

void DistributedMonitor::setMyNotFulfilledRequest(std::shared_ptr<Message> message, int counter) {
    myRequest request(message->getSendersClock(), counter);
    setMyNotFulfilledRequest(request);
}

void DistributedMonitor::sendMessage(std::shared_ptr<Message> message) {
    if (message->getReceiversId() == NOT_SET) {
        std::cerr << "Receivers distributedClientId not set!\n";
        throw "Receivers distributedClientId not set!\n";
        return;
    }
    connectionManager->sendMessage(message);
}

void DistributedMonitor::sendSingleMessage(std::shared_ptr<Message> message, bool waitForReply) {
    updateLamportClock();
    message->setSendersClock(this->lamportClock);
    if (waitForReply)
        setMyNotFulfilledRequest(std::make_shared<Message>(*message), 1);
    sendMessage(message);
}

int DistributedMonitor::sendMessageOnBroadcast(std::shared_ptr<Message> message, bool waitForReply) {
    updateLamportClock();
    int lamportClock = this->lamportClock;
    message->setSendersClock(lamportClock);
    int clientsCount = connectionManager->getDistributedClientsCount() * connectionManager->getLocalClientsCount();
    if (waitForReply)
        setMyNotFulfilledRequest(std::make_shared<Message>(*message), (clientsCount - 1));
    for (int globalId = 0; globalId < connectionManager->getDistributedClientsCount(); globalId ++) {
        std::cout << "clientsCount: " << this->connectionManager->getLocalClientsCount() << std::endl;
        for (int localIdIndex = 1; localIdIndex <= this->connectionManager->getLocalClientsCount(); localIdIndex++) {
            int localId = localIdIndex * clientIdStep;
            if (getDistributedClientId() == globalId and getLocalClientId() == localId) continue;
            message->setReceiversId(globalId, localId);
            sendMessage(message);
        }

    }
    // connectionManager->sendMessageOnBroadcast(message);
    return lamportClock;
}

/*
 * listen() - function called on listenThread. It manages all received messages.
 */

void DistributedMonitor::d_lock() {
    // SEND REQUEST FOR CRITICAL SECTION
    mutexMap["state"].lock();
    changeState(State::WAITING_FOR_REPLIES);
    std::cout << getDistributedClientId() << getUniqueConnectionNo() << ": d_lock() - TRY\n";
    std::shared_ptr<Message> msg = std::make_shared<Message>
            (this->getDistributedClientId(), this->getLocalClientId(), Message::MessageType::LOCK_MTX);
    int thisMessageClock = this->sendMessageOnBroadcast(msg, true);
    mutexMap["state"].unlock();


    // TODO if!! bo jak sprawdza to check if got all replies już jest wyczyszczony!!!
    // TODO
    while (!checkIfGotAllReplies(thisMessageClock)) {
        std::unique_lock<std::mutex> lk(mutexMap["critical-section"]);
        std::cout << getDistributedClientId() << getUniqueConnectionNo() << ": WAIT\n";
        if (!checkIfGotAllReplies(thisMessageClock))
            // todo jak teraz zrobi decrement to kupa więc jest w critical-section
            cvMap["gotAllReplies"].wait(lk);    // odpuszcza critical-section
    };
    std::cout << getDistributedClientId() << getUniqueConnectionNo() << ": GLOBAL['critical-section'].lock()\n";

    // NOW IN CRITICAL SECTION
    mutexMap["state"].lock();
    changeState(State::IN_CRITICAL_SECTION);
    myRequest clear;
    setMyNotFulfilledRequest(clear);
    mutexMap["state"].unlock();
}

void DistributedMonitor::d_unlock() {
    // SEND MESSAGE WITH CHANGED DATA AND LEAVE CRITICAL SECTION
    // send responses from requestsFromOthersQueue
    // TODO tutaj?
    mutexMap["critical-section"].unlock();
    std::cout << getDistributedClientId() << getUniqueConnectionNo() <<": GLOBAL[\"critical-section\"].unlock()\n";
    mutexMap["state"].lock();
    freeRequests();
    changeState(State::FREE);
    mutexMap["state"].unlock();
    std::string data = returnDataToSend();
    std::shared_ptr<Message> msg = std::make_shared<Message>
            (this->getDistributedClientId(), this->getLocalClientId(), Message::MessageType::UNLOCK_MTX, data);
    this->sendMessageOnBroadcast(msg, false);
}

void DistributedMonitor::sendLockResponse(int receiverId, int receiversLocalId, int requestClock) {
    mutexMap["critical-section"].lock();
    std::cout << getDistributedClientId() << getUniqueConnectionNo() <<": LOCAL['critical-section'].lock()\n";
    std::shared_ptr<Message> msg = std::make_shared<Message>
            (this->getDistributedClientId(),
             this->getLocalClientId(),
             Message::MessageType::LOCK_RESPONSE,
             requestClock);
    msg->setReceiversId(receiverId, receiversLocalId);
    this->sendSingleMessage(msg, false);
}

void DistributedMonitor::reactForLockRequest(Message *receivedMessage) {
    mutexMap["state"].lock();
    switch (this->state) {
        case State::WAITING_FOR_REPLIES: {
            assert(myNotFulfilledRequest.clock != -1);
            myRequest myRequest = getMyNotFulfilledRequest();
            // TODO remove cout
//            std::cout << getDistributedClientId() << getUniqueConnectionNo() << ": " << receivedMessage->getSendersClock()
//                      << " <? " << myRequest.clock << "\n";
            if (receivedMessage->getSendersClock() < myRequest.clock
                or (receivedMessage->getSendersClock() == myRequest.clock
                     and receivedMessage->getSendersDistributedId() < this->getDistributedClientId())
                or (receivedMessage->getSendersClock() == myRequest.clock
                    and receivedMessage->getSendersDistributedId() == this->getDistributedClientId()
                    and receivedMessage->getSendersLocalId() < this->getLocalClientId())) {
                // our clock is worse (greater) or our distributedClientId is worse or our localClientId is worse : lock mutex
                sendLockResponse(receivedMessage->getSendersDistributedId(), receivedMessage->getSendersLocalId(), receivedMessage->getSendersClock());
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
            sendLockResponse(receivedMessage->getSendersDistributedId(), receivedMessage->getSendersLocalId(), receivedMessage->getSendersClock());
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
    std::cout << getDistributedClientId() << getUniqueConnectionNo() << ": LOCAL['critical-section'].unlock()\n";
}

void DistributedMonitor::listen() {
    std::cout << getDistributedClientId() << getUniqueConnectionNo() << ": LISTEN!\n";
    Message message;
    while(true) {
        // connectionManager->getReceiveMutex()->lock();
        int localId = getLocalClientId();
        if (connectionManager->tryToReceive(Message::MessageType::LOCK_MTX + localId)) {
            message = connectionManager->receiveMessage(Message::MessageType::LOCK_MTX + localId);
            std::cout << getDistributedClientId() << getUniqueConnectionNo() << ": received LOCK_MTX!\n";
            updateLamportClock(message.getSendersClock());
            reactForLockRequest(&message);
        }
        if (connectionManager->tryToReceive(Message::MessageType::LOCK_RESPONSE + localId)) {
            message = connectionManager->receiveMessage(Message::MessageType::LOCK_RESPONSE + localId);
            std::cout << getDistributedClientId() << getUniqueConnectionNo() << ": received LOCK_RESPONSE!\n";
            updateLamportClock(message.getSendersClock());
            reactForLockResponse(&message);

        }
        if (connectionManager->tryToReceive(Message::MessageType::UNLOCK_MTX + localId)) {
            message = connectionManager->receiveMessage(Message::MessageType::UNLOCK_MTX + localId);
            std::cout << getDistributedClientId() << getUniqueConnectionNo() << ": received UNLOCK_MTX!\n";
            updateLamportClock(message.getSendersClock());
            reactForUnlock(&message);
        }

//        std::chrono::seconds sec(3);
//        std::this_thread::sleep_for(sec);

        // connectionManager->getReceiveMutex()->unlock();
    }
}

bool DistributedMonitor::checkIfGotAllReplies(int clock) {
    myRequest myRequest = getMyNotFulfilledRequest();
    if (clock == myRequest.clock)
        return (myRequest.answerCounter <= 0);
    else return true;
}

void DistributedMonitor::freeRequests() {
    // TODO in new thread
    while (!requestsFromOthersQueue.empty()) {
        Message message = requestsFromOthersQueue.front();
        sendLockResponse(message.getSendersDistributedId(), message.getSendersLocalId(), message.getSendersClock());
        requestsFromOthersQueue.pop();
    }
}











