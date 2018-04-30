//
// Created by ola on 27.04.18.
//

#include <iostream>
#include <thread>
#include "ConnectionManager.h"

ConnectionManager::ConnectionManager(std::shared_ptr<ConnectionInterface>connection) : connection(connection) {
    this->localClientId = this->connection->addNewLocalClient() * clientIdStep;
    std::stringstream str;
    str << "./log" << connection->getUniqueConnectionNo() << ".txt";
    this->logger = std::make_unique<Logger>(str.str());
}

ConnectionManager::~ConnectionManager() {}

int ConnectionManager::getDistributedClientId() {
    return this->connection->getDistributedClientId();
}

int ConnectionManager::getLocalClientId() {
    return this->localClientId;
}

int ConnectionManager::getUniqueConnectionNo() {
    return this->connection->getUniqueConnectionNo();
}


void ConnectionManager::sendMessage(std::shared_ptr<Message> message) {
    if (message->getReceiversId() == NOT_SET) {
        std::cerr << "Receivers distributedClientId not set!\n";
        throw "Receivers distributedClientId not set!\n";
        return;
    }
    connection->sendMessage(message);
}

void ConnectionManager::sendSingleMessage(std::shared_ptr<Message> message, bool waitForReply) {
    algorithm.updateLamportClock();
    message->setSendersClock(algorithm.getLamportClock());
    if (waitForReply)
        algorithm.setMyNotFulfilledRequest(std::make_shared<Message>(*message), 1);
    sendMessage(message);
}

int ConnectionManager::sendMessageOnBroadcast(std::shared_ptr<Message> message, bool waitForReply, int sendersClock) {
    algorithm.updateLamportClock();
    int lamportClock = (sendersClock == -1) ? algorithm.getLamportClock() : sendersClock;
    message->setSendersClock(lamportClock);
    int clientsCount = connection->getDistributedClientsCount() * connection->getLocalClientsCount();
    if (waitForReply)
        algorithm.setMyNotFulfilledRequest(std::make_shared<Message>(*message), (clientsCount - 1));
    for (int globalId = 0; globalId < connection->getDistributedClientsCount(); globalId ++) {
        for (int localIdIndex = 1; localIdIndex <= this->connection->getLocalClientsCount(); localIdIndex++) {
            int localId = localIdIndex * clientIdStep;
            if (getDistributedClientId() == globalId and getLocalClientId() == localId) continue;
            message->setReceiversId(globalId, localId);
            sendMessage(message);
        }
    }
    return lamportClock;
}

void ConnectionManager::sendResponse(int receiverId, int receiversLocalId, int requestClock, std::string data) {
    std::shared_ptr<Message> msg = std::make_shared<Message>
            (this->getDistributedClientId(),
             this->getLocalClientId(),
             Message::MessageType::LOCK_RESPONSE,
             requestClock,
             data);
    msg->setReceiversId(receiverId, receiversLocalId);
    this->sendSingleMessage(msg, false);
}


void ConnectionManager::sendLockResponse(int receiverId, int receiversLocalId, int requestClock, std::string data) {
    algorithm.incrementResponsesSentByMeCounter();      // we have to get unlock for this response
    sendResponse(receiverId, receiversLocalId, requestClock, data);
}

int ConnectionManager::sendUnLockMessages(std::string dataToSend) {
    std::shared_ptr<Message> msg = std::make_shared<Message>
            (this->getDistributedClientId(), this->getLocalClientId(), Message::MessageType::UNLOCK_MTX, dataToSend);
    return this->sendMessageOnBroadcast(msg, true);
}

int ConnectionManager::sendUnLockAndWaitMessages() {
    std::shared_ptr<Message> msg = std::make_shared<Message>
            (this->getDistributedClientId(), this->getLocalClientId(), Message::MessageType::UNLOCK_MTX_WAIT, algorithm.getMyNotFulfilledRequest().clock);
    return this->sendMessageOnBroadcast(msg, false);
}

void ConnectionManager::freeRequests() {
    while (!algorithm.isRequestsFromOthersQueueEmpty()) {
        Message message = algorithm.removeReceivedRequestFromQueue();
        sendLockResponse(message.getSendersDistributedId(), message.getSendersLocalId(), message.getSendersClock());
    }
}

void ConnectionManager::signalIfAllUnlocksReceived() {
    if (algorithm.getResponsesSentByMeCounter() <= 0) {
        mutexMap["unlock-response"].lock();
        mutexMap["unlock-response"].unlock();
        cvMap["receivedAllUnlocks"].notify_all();
    }
}

void ConnectionManager::signalIfAllResponsesReceived(int requestClock) {
    int count = algorithm.getNotAnsweredRepliesCount(requestClock);
    if (count <= 0) {
        // make sure other thread is waiting on mutex:
        mutexMap["response-lock"].lock();
        mutexMap["response-lock"].unlock();
        cvMap["receivedAllReplies"].notify_all();
    }
}

bool ConnectionManager::tryToReceiveMessage(int messageType) {
    return connection->tryToReceive(messageType + localClientId);
}

std::string ConnectionManager::messageTypeToString(int messageType) {
    std::string typeString;
    switch(messageType) {
        case Message::MessageType::SIGNAL : {
            typeString = "SIGNAL";
            break;
        }
        case Message::MessageType::UNLOCK_MTX_WAIT : {
            typeString = "UNLOCK_MTX_WAIT";
            break;
        }
        case Message::MessageType::UNLOCK_MTX : {
            typeString = "UNLOCK_MTX";
            break;
        }
        case Message::MessageType::LOCK_MTX : {
            typeString = "LOCK_MTX";
            break;
        }
        case Message::MessageType::LOCK_RESPONSE : {
            typeString = "LOCK_RESPONSE";
            break;
        }
        case Message::MessageType::COMMUNICATION_END : {
            typeString = "COMMUNICATION_END";
            break;
        }
        default :
            typeString = "";
    }
    return typeString;
}

Message ConnectionManager::receiveMessage(int messageType) {
    std::string typeString = messageTypeToString(messageType);
    Message message = connection->receiveMessage(messageType + localClientId);
    algorithm.updateLamportClock(message.getSendersClock());
    std::stringstream str;
    // TODO remove received log
    str << "received " << typeString << "!: from " << message.getSendersLocalId() << ":" <<
        message.getSendersDistributedId();
    log(str.str());
//    algorithm.updateLamportClock(message.getSendersClock());
    return message;
}

void ConnectionManager::log(std::string log) {
    std::stringstream str;
    str << algorithm.getLamportClock() << " "
        << getLocalClientId() << ":"
        << getDistributedClientId() << ":"
        << getUniqueConnectionNo()
        << ": " << log;
    logger->log(str.str());
}

int ConnectionManager::incrementThreadsThatWantToEndCommunicationCounter() {
    threadsThatWantToEndCommunicationCounter++;
    return threadsThatWantToEndCommunicationCounter;
}

bool ConnectionManager::receivedAllCommunicationEndMessages() {
    int count = connection->getDistributedClientsCount() * connection-> getLocalClientsCount();
//    count = connection->getDistributedClientsCount();
    return (threadsThatWantToEndCommunicationCounter >= count);
}

void ConnectionManager::endConnection() {
    log("--- SEND CONNECTION END ---");
    incrementThreadsThatWantToEndCommunicationCounter();
    std::shared_ptr<Message> msg = std::make_shared<Message>
            (getDistributedClientId(), getLocalClientId(), Message::MessageType::COMMUNICATION_END);
    int clock = sendMessageOnBroadcast(msg, false);
    std::unique_lock<std::mutex> lock(mutexMap["connection-end"]);
    while (!receivedAllCommunicationEndMessages()) {
        std::stringstream str;
        str << "WAIT for CONNECTION END (" << clock << ")";
        // log(str.str());
        cvMap["end"].wait(lock);
    };
}



