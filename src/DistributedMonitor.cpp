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
        str << "WAIT FOR CRITICAL SECTION (" << thisMessageClock << ")";
        log(str.str());
        cvMap["receivedAllReplies"].wait(lock);
    };

    // NOW IN CRITICAL SECTION (mutexMap["global-lock"] is locked)
    std::stringstream str;
    str << "GLOBAL['global-lock'].lock() ("  << thisMessageClock << ")";
    log(str.str());
    mutexMap["state"].lock();
    algorithm.changeState(RicardAgravala::State::IN_CRITICAL_SECTION);
    RicardAgravala::myRequest clear;
    algorithm.setMyNotFulfilledRequest(clear);
    mutexMap["state"].unlock();
}

void DistributedMonitor::d_unlock() {
    // SEND MESSAGE WITH CHANGED DATA AND LEAVE CRITICAL SECTION
    log("GLOBAL['global-lock'].unlock() - udawany");

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
    str << "Count: " << count;
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
        // got all responses -> go to critical section
        log("NOTIFY receivedAllReplies lock");
        // make sure other thread is waiting on mutex:
        mutexMap["global-lock"].lock();
        mutexMap["global-lock"].unlock();
        cvMap["receivedAllReplies"].notify_all();
    }
}

void DistributedMonitor::reactForUnlock(Message *receivedMessage) {
    std::string data = receivedMessage->getData();
    // TODO manage only last data
    manageReceivedData(data);
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
            str << "received LOCK_MTX!: from " << message.getSendersLocalId() << ":" << message.getSendersDistributedId()
                << " sent on " << message.getSendersClock();
            log(str.str());
            algorithm.updateLamportClock(message.getSendersClock());
            reactForLockRequest(&message);
        }
        if (connectionManager->tryToReceive(Message::MessageType::LOCK_RESPONSE + localId)) {
            message = connectionManager->receiveMessage(Message::MessageType::LOCK_RESPONSE + localId);
            std::stringstream str;
            str << "received LOCK_RESPONSE!(for wait " << message.getRequestClock() << "): from " << message.getSendersLocalId() << ":" << message.getSendersDistributedId()
                << " sent on " << message.getSendersClock();
            log(str.str());
            algorithm.updateLamportClock(message.getSendersClock());
            reactForLockResponse(&message);

        }
        if (connectionManager->tryToReceive(Message::MessageType::UNLOCK_MTX + localId)) {
            message = connectionManager->receiveMessage(Message::MessageType::UNLOCK_MTX + localId);
            std::stringstream str;
            str << "received UNLOCK_MTX! from " << message.getSendersLocalId() << ":" << message.getSendersDistributedId()
                    << " sent on " << message.getSendersClock();
            log(str.str());
            bool updated = algorithm.updateLamportClock(message.getSendersClock());
            //if (updated) reactForUnlock(&message);
            reactForUnlock(&message);
            // reactForLockResponse(&message);
        }
        // connectionManager->getReceiveMutex()->unlock();
    }
}

void DistributedMonitor::log(std::string log) {
    std::stringstream str;
    str << algorithm.getLamportClock() << " "
              << getLocalClientId() << ":"
              << getDistributedClientId() << ":"
              << getUniqueConnectionNo()
              << ": " << log;
    logger.log(str.str());
}














