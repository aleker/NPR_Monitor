#include <iostream>
#include "DistributedMonitor.h"


DistributedMonitor::DistributedMonitor(ConnectionInterface* connectionManager) :
        connectionManager(connectionManager) {
    this->localClientId = this->connectionManager->addNewLocalClient() * clientIdStep;
    std::stringstream str;
    str << "./log" << connectionManager->getUniqueConnectionNo() << ".txt";
    this->logger = std::make_unique<Logger>(str.str());
    this->listenThread = std::thread(&DistributedMonitor::listen, this);
}

DistributedMonitor::~DistributedMonitor() {
    // TODO end connection properly
    log(": JOIN?");
    this->listenThread.join();
    log(": JOIN!");
}

void DistributedMonitor::log(std::string log) {
    std::stringstream str;
    str << algorithm.getLamportClock() << " "
        << getLocalClientId() << ":"
        << getDistributedClientId() << ":"
        << getUniqueConnectionNo()
        << ": " << log;
    logger->log(str.str());
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
        std::string data = returnDataToSend();
        sendLockResponse(message.getSendersDistributedId(), message.getSendersLocalId(), message.getSendersClock(), data);
    }
}

void DistributedMonitor::d_lock() {
    // SEND REQUEST FOR CRITICAL SECTION
    mutexMap["state"].lock();
    algorithm.changeState(RicardAgravala::State::WAITING_FOR_REPLIES);
    std::shared_ptr<Message> msg = std::make_shared<Message>
            (this->getDistributedClientId(), this->getLocalClientId(), Message::MessageType::LOCK_MTX);
    int thisMessageClock = this->sendMessageOnBroadcast(msg, true);
    mutexMap["state"].unlock();

    // CRITICAL SECTION ENTRY
    std::unique_lock<std::mutex> lock(mutexMap["global-lock"]);
    while (algorithm.getNotAnsweredRepliesCount(thisMessageClock) > 0) {
        std::stringstream str;
        str << "WAIT for critical section (" << thisMessageClock << ")";
        log(str.str());
        cvMap["receivedAllReplies"].wait(lock);
    };

    // WAIT FOR ALL UNLOCK MESSAGES
    std::unique_lock<std::mutex> lock2(mutexMap["unlock-response"]);
    while (algorithm.getResponsesSentByMeCounter() > 0) {
        cvMap["receivedAllUnlocks"].wait(lock2);
    }
    lock2.unlock();

    // NOW IN CRITICAL SECTION
    std::stringstream str;
    str << "---CRITICAL SECTION : START--- ("  << thisMessageClock << ")";
    log(str.str());
    mutexMap["state"].lock();
    algorithm.changeState(RicardAgravala::State::IN_CRITICAL_SECTION);
    RicardAgravala::myRequest clear;
    algorithm.setMyNotFulfilledRequest(clear);
    mutexMap["state"].unlock();
}

void DistributedMonitor::d_unlock() {
    // SEND MESSAGE WITH CHANGED DATA AND LEAVE CRITICAL SECTION
    log("---CRITICAL SECTION : EXIT ---");

    // send unlock messages with updated data
    std::string data = returnDataToSend();
    std::shared_ptr<Message> msg = std::make_shared<Message>
            (this->getDistributedClientId(), this->getLocalClientId(), Message::MessageType::UNLOCK_MTX, data);
    this->sendMessageOnBroadcast(msg, false);

    // send responses from requestsFromOthersQueue:
    mutexMap["state"].lock();
    freeRequests();
    algorithm.changeState(RicardAgravala::State::FREE);
    mutexMap["state"].unlock();
}

void DistributedMonitor::sendLockResponse(int receiverId, int receiversLocalId, int requestClock, std::string data) {
    std::shared_ptr<Message> msg = std::make_shared<Message>
            (this->getDistributedClientId(),
             this->getLocalClientId(),
             Message::MessageType::LOCK_RESPONSE,
             requestClock,
             data);
    msg->setReceiversId(receiverId, receiversLocalId);
    algorithm.incrementResponsesSentByMeCounter();      // we have to get unlock for this response
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
                // SEND LOCK_RESPONSE NOW!
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
            // NOT NEEDED: SEND LOCK_RESPONSE NOW!
            sendLockResponse(receivedMessage->getSendersDistributedId(), receivedMessage->getSendersLocalId(), receivedMessage->getSendersClock());
            break;
        }
    }
    mutexMap["state"].unlock();
}

void DistributedMonitor::reactForLockResponse(Message *receivedMessage) {
    // decrement response counter
    bool responseForMyCurrentRequest = algorithm.decrementReplyCounter(receivedMessage->getRequestClock());
    int count = algorithm.getNotAnsweredRepliesCount(receivedMessage->getRequestClock());
    std::stringstream str;
    str << "Responses counter: " << count;
    log(str.str());

    if (!responseForMyCurrentRequest) {
        std::cerr << "Response not for current request!\n";
        return;
    }

    // check if response with updated data
    if (!receivedMessage->getData().compare("")) {
        // response with data
        manageReceivedData(receivedMessage->getData());
    }

    // notify if last needed response
    if (count <= 0) {
        // make sure other thread is waiting on mutex:
        mutexMap["global-lock"].lock();
        mutexMap["global-lock"].unlock();
        cvMap["receivedAllReplies"].notify_all();
    }
}

void DistributedMonitor::reactForUnlock(Message *receivedMessage) {
    // decrement required unlocks counter
    algorithm.decrementResponsesSentByMeCounter();

    // update shared data only if later unlock
    if (algorithm.getLamportClockOfLastReceivedUnlock() < receivedMessage->getSendersClock()) {
        std::string data = receivedMessage->getData();
        manageReceivedData(data);
        algorithm.setLamportClockOfLastReceivedUnlock(receivedMessage->getSendersClock());
    }

    // notify if all unlocks received
    if (algorithm.getResponsesSentByMeCounter() <= 0) {
        mutexMap["unlock-response"].lock();
        mutexMap["unlock-response"].unlock();
        cvMap["receivedAllUnlocks"].notify_all();
    }
}

/*
 * listen() - function called on listenThread. It manages all received messages.
 */

void DistributedMonitor::listen() {
    Message message;
    while(true) {
        int localId = getLocalClientId();
        if (connectionManager->tryToReceive(Message::MessageType::LOCK_MTX + localId)) {
            message = connectionManager->receiveMessage(Message::MessageType::LOCK_MTX + localId);
            std::stringstream str;
            str << "received LOCK_MTX!: from " << message.getSendersLocalId() << ":" <<
                message.getSendersDistributedId();
            log(str.str());
            algorithm.updateLamportClock(message.getSendersClock());
            reactForLockRequest(&message);
        }
        if (connectionManager->tryToReceive(Message::MessageType::LOCK_RESPONSE + localId)) {
            message = connectionManager->receiveMessage(Message::MessageType::LOCK_RESPONSE + localId);
            std::stringstream str;
            str << "received LOCK_RESPONSE! (wait in " << message.getRequestClock() << "): from " <<
                message.getSendersLocalId() << ":" << message.getSendersDistributedId();
            log(str.str());
            algorithm.updateLamportClock(message.getSendersClock());
            reactForLockResponse(&message);

        }
        if (connectionManager->tryToReceive(Message::MessageType::UNLOCK_MTX + localId)) {
            message = connectionManager->receiveMessage(Message::MessageType::UNLOCK_MTX + localId);
            std::stringstream str;
            str << "received UNLOCK_MTX! from " << message.getSendersLocalId() << ":" <<
                message.getSendersDistributedId();
            log(str.str());
            algorithm.updateLamportClock(message.getSendersClock());
            reactForUnlock(&message);
        }
    }
}















