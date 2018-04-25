#include <iostream>
#include "DistributedMonitor.h"


DistributedMonitor::DistributedMonitor(ConnectionManager* connectionManager) :
        connectionManager(connectionManager) {
    this->localClientId = this->connectionManager->addNewLocalClient() * clientIdStep;
    std::stringstream str;
    log("");
    this->listenThread = std::thread(&DistributedMonitor::listen, this);
}

DistributedMonitor::~DistributedMonitor() {
    // TODO end connection properly
    log(": JOIN?");
    this->listenThread.join();
    log(": JOIN!");
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


void DistributedMonitor::sendMessage(std::shared_ptr<Message> message) {
    if (message->getReceiversId() == NOT_SET) {
        std::cerr << "Receivers distributedClientId not set!\n";
        throw "Receivers distributedClientId not set!\n";
        return;
    }
    connectionManager->sendMessage(message);
}

void DistributedMonitor::sendSingleMessage(std::shared_ptr<Message> message, bool waitForReply) {
    algorithm.updateLamportClock();
    message->setSendersClock(algorithm.getLamportClock());
    if (waitForReply)
        algorithm.setMyNotFulfilledRequest(std::make_shared<Message>(*message), 1);
    sendMessage(message);
}

int DistributedMonitor::sendMessageOnBroadcast(std::shared_ptr<Message> message, bool waitForReply) {
    algorithm.updateLamportClock();
    int lamportClock = algorithm.getLamportClock();
    message->setSendersClock(lamportClock);
    int clientsCount = connectionManager->getDistributedClientsCount() * connectionManager->getLocalClientsCount();
    if (waitForReply)
        algorithm.setMyNotFulfilledRequest(std::make_shared<Message>(*message), (clientsCount - 1));
    for (int globalId = 0; globalId < connectionManager->getDistributedClientsCount(); globalId ++) {
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

void DistributedMonitor::freeRequests() {
    while (!algorithm.isRequestsFromOthersQueueEmpty()) {
        Message message = algorithm.removeReceivedRequestFromQueue();
        sendLockResponse(message.getSendersDistributedId(), message.getSendersLocalId(), message.getSendersClock());
    }
}

int DistributedMonitor::tryToLock() {
    // SEND REQUEST FOR CRITICAL SECTION
    mutexMap["state"].lock();
    algorithm.changeState(RicardAgravala::State::WAITING_FOR_REPLIES);
    log("tryToLock() = TRY");
    std::shared_ptr<Message> msg = std::make_shared<Message>
            (this->getDistributedClientId(), this->getLocalClientId(), Message::MessageType::LOCK_MTX);
    int thisMessageClock = this->sendMessageOnBroadcast(msg, true);
    mutexMap["state"].unlock();

    // lock

    // goToCriticalSection()
    return thisMessageClock;
}

std::condition_variable* DistributedMonitor::getCriticalConditionVariable() {
    return &cvMap["gotAllReplies"];
}

std::mutex* DistributedMonitor::getCriticalMutex() {
    return &mutexMap["critical-section"];
}

void DistributedMonitor::goToCriticalSection() {
    // NOW IN CRITICAL SECTION
    log("GLOBAL['critical-section'].lock()");
    mutexMap["state"].lock();
    algorithm.changeState(RicardAgravala::State::IN_CRITICAL_SECTION);
    RicardAgravala::myRequest clear;
    algorithm.setMyNotFulfilledRequest(clear);
    mutexMap["state"].unlock();
}

void DistributedMonitor::d_unlock() {
    // SEND MESSAGE WITH CHANGED DATA AND LEAVE CRITICAL SECTION
    mutexMap["critical-section"].lock();
    log("GLOBAL['critical-section'].lock()!!!!!!!!!!!ZZZ!!!!!!!!!!!!!!!!");
//    lock.unlock();
//    log("GLOBAL['critical-section'].unlock()");
    mutexMap["state"].lock();
    // send responses from requestsFromOthersQueue:
    freeRequests();
    algorithm.changeState(RicardAgravala::State::FREE);
    mutexMap["state"].unlock();
    std::string data = returnDataToSend();
    std::shared_ptr<Message> msg = std::make_shared<Message>
            (this->getDistributedClientId(), this->getLocalClientId(), Message::MessageType::UNLOCK_MTX, data);
    this->sendMessageOnBroadcast(msg, false);
}

void DistributedMonitor::sendLockResponse(int receiverId, int receiversLocalId, int requestClock) {
//    mutexMap["critical-section"].lock();
//    log("LOCAL['critical-section'].lock()");
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
    switch (algorithm.getState()) {
        case RicardAgravala::State::WAITING_FOR_REPLIES: {
            RicardAgravala::myRequest myRequest = algorithm.getMyNotFulfilledRequest();
            if (receivedMessage->getSendersClock() < myRequest.clock
                or (receivedMessage->getSendersClock() == myRequest.clock
                     and receivedMessage->getSendersDistributedId() < this->getDistributedClientId())
                or (receivedMessage->getSendersClock() == myRequest.clock
                    and receivedMessage->getSendersDistributedId() == this->getDistributedClientId()
                    and receivedMessage->getSendersLocalId() < this->getLocalClientId())) {
                // our clock is worse (greater) or our distributedClientId is worse or our localClientId is worse : lock mutex
                mutexMap["critical-section"].lock();
                log("LOCAL['critical-section'].lock()");
                sendLockResponse(receivedMessage->getSendersDistributedId(),
                                 receivedMessage->getSendersLocalId(),
                                 receivedMessage->getSendersClock());
            }
            else {
                // we are better
                algorithm.addReceivedRequestToQueue(receivedMessage);
            }
            break;
        }
        case RicardAgravala::State::IN_CRITICAL_SECTION: {
            algorithm.addReceivedRequestToQueue(receivedMessage);
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
    algorithm.decrementReplyCounter(receivedMessage->getRequestClock());
    if (algorithm.checkIfGotAllReplies(receivedMessage->getRequestClock())) {
        // got all responses -> go to critical section
        cvMap["gotAllReplies"].notify_one();
    }
}

void DistributedMonitor::reactForUnlock(Message *receivedMessage) {
    std::string data = receivedMessage->getData();
    manageReceivedData(data);
    cvMap["receivedAllNeededData"].notify_all();
    mutexMap["critical-section"].unlock();
    log("LOCAL['critical-section'].unlock()");
}

/*
 * listen() - function called on listenThread. It manages all received messages.
 */

void DistributedMonitor::listen() {
    log("LISTEN!");
    Message message;
    while(true) {
        // connectionManager->getReceiveMutex()->lock();
        int localId = getLocalClientId();
        if (connectionManager->tryToReceive(Message::MessageType::LOCK_MTX + localId)) {
            message = connectionManager->receiveMessage(Message::MessageType::LOCK_MTX + localId);
            std::stringstream str;
            str << "received LOCK_MTX!: from " << message.getSendersLocalId() << ":" << message.getSendersDistributedId();
            log(str.str());
            algorithm.updateLamportClock(message.getSendersClock());
            reactForLockRequest(&message);
        }
        if (connectionManager->tryToReceive(Message::MessageType::LOCK_RESPONSE + localId)) {
            message = connectionManager->receiveMessage(Message::MessageType::LOCK_RESPONSE + localId);
            std::stringstream str;
            str << "received LOCK_RESPONSE!: from " << message.getSendersLocalId() << ":" << message.getSendersDistributedId();
            log(str.str());
            algorithm.updateLamportClock(message.getSendersClock());
            reactForLockResponse(&message);

        }
        if (connectionManager->tryToReceive(Message::MessageType::UNLOCK_MTX + localId)) {
            message = connectionManager->receiveMessage(Message::MessageType::UNLOCK_MTX + localId);
            std::stringstream str;
            str << "received UNLOCK_MTX! from " << message.getSendersLocalId() << ":" << message.getSendersDistributedId();
            log(str.str());
            algorithm.updateLamportClock(message.getSendersClock());
            reactForUnlock(&message);
        }
        // connectionManager->getReceiveMutex()->unlock();
    }
}

void DistributedMonitor::log(std::string log) {
    std::cout << getLocalClientId() << ":"
              << getDistributedClientId() << ":"
              << getUniqueConnectionNo()
              << ": " << log << std::endl;
}











