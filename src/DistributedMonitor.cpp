#include <iostream>
#include "DistributedMonitor.h"

DistributedMonitor::DistributedMonitor(std::shared_ptr<ConnectionInterface> connection) :
        connectionManager(std::make_shared<ConnectionManager>(connection)) {
    this->d_mutex = std::make_shared<DistributedMutex>(this->connectionManager);
    this->listenThread = std::thread(&DistributedMonitor::listen, this);
}

DistributedMonitor::~DistributedMonitor() {
    this->listenThread.join();
    log("JOINED!");
}

/*
 * listen() - function called on listenThread. It manages all received messages.
 */

void DistributedMonitor::listen() {
    Message message;
    while (!connectionManager->receivedAllCommunicationEndMessages()) {
        if (connectionManager->tryToReceiveMessage(Message::MessageType::LOCK_MTX)) {
            message = connectionManager->receiveMessage(Message::MessageType::LOCK_MTX);
            reactForLockRequest(&message);
            d_mutex->removeThisThreadFromWaitingList(message.getSendersLocalId(), message.getSendersDistributedId());
        }
        if (connectionManager->tryToReceiveMessage(Message::MessageType::LOCK_RESPONSE)) {
            message = connectionManager->receiveMessage(Message::MessageType::LOCK_RESPONSE);
            reactForLockResponse(&message);
        }
        if (connectionManager->tryToReceiveMessage(Message::MessageType::UNLOCK_MTX)) {
            message = connectionManager->receiveMessage(Message::MessageType::UNLOCK_MTX);
            reactForUnlock(&message);
        }
        if (connectionManager->tryToReceiveMessage(Message::MessageType::UNLOCK_MTX_WAIT)) {
            message = connectionManager->receiveMessage(Message::MessageType::UNLOCK_MTX_WAIT);
            reactForWait(&message);
        }
        if (connectionManager->tryToReceiveMessage(Message::MessageType::SIGNAL)) {
            message = connectionManager->receiveMessage(Message::MessageType::SIGNAL);
            reactForSignalMessage(&message);
        }
        if (connectionManager->tryToReceiveMessage(Message::MessageType::COMMUNICATION_END)) {
            message = connectionManager->receiveMessage(Message::MessageType::COMMUNICATION_END);
            reactForCommunicationEndMessage();
        }
    }
}

void DistributedMonitor::reactForLockRequest(Message *receivedMessage) {
    d_mutex->stateMutex.lock();
    int currentState = connectionManager->algorithm.getState();
    switch (currentState) {
        case RicardAgravala::State::WAITING_FOR_REPLIES: {
            RicardAgravala::myRequest myRequest = connectionManager->algorithm.getMyNotFulfilledRequest();
            if (receivedMessage->getSendersClock() < myRequest.clock
                or (receivedMessage->getSendersClock() == myRequest.clock
                    and receivedMessage->getSendersDistributedId() < connectionManager->getDistributedClientId())
                or (receivedMessage->getSendersClock() == myRequest.clock
                    and receivedMessage->getSendersDistributedId() == connectionManager->getDistributedClientId()
                    and receivedMessage->getSendersLocalId() < connectionManager->getLocalClientId())) {
                // SEND LOCK_RESPONSE NOW!
                connectionManager->sendLockResponse(receivedMessage->getSendersDistributedId(),
                                                    receivedMessage->getSendersLocalId(),
                                                    receivedMessage->getSendersClock());
                d_mutex->setLastRequestThatWeResponsedClock(receivedMessage->getSendersClock());
            } else {
                // we are better
                connectionManager->algorithm.addReceivedRequestToQueue(receivedMessage);
            }
            break;
        }
        case RicardAgravala::State::IN_CRITICAL_SECTION: {
            connectionManager->algorithm.addReceivedRequestToQueue(receivedMessage);
            break;
        }
        default: {
            // NOT NEEDED: SEND LOCK_RESPONSE NOW!
            connectionManager->sendLockResponse(receivedMessage->getSendersDistributedId(),
                                                receivedMessage->getSendersLocalId(),
                                                receivedMessage->getSendersClock());
            d_mutex->setLastRequestThatWeResponsedClock(receivedMessage->getSendersClock());
            break;
        }
    }
    d_mutex->stateMutex.unlock();
}

void DistributedMonitor::reactForLockResponse(Message *receivedMessage) {
    // decrement response counter
    bool responseForMyCurrentRequest = connectionManager->algorithm.decrementReplyCounter(
            receivedMessage->getRequestClock());
    int count = connectionManager->algorithm.getNotAnsweredRepliesCount(receivedMessage->getRequestClock());
    std::stringstream str;
    str << "Responses counter: " << count;
    //connectionManager->log(str.str());

    if (!responseForMyCurrentRequest) {
        log("Response not for current request!\n");
        return;
    }

    // notify if last needed response
    connectionManager->signalIfAllResponsesReceived(receivedMessage->getRequestClock());
}

void DistributedMonitor::reactForUnlock(Message *receivedMessage) {
    // send received unlock confirmation
    connectionManager->sendResponse(receivedMessage->getSendersDistributedId(), receivedMessage->getSendersLocalId(),
                                    receivedMessage->getSendersClock());

    // decrement required unlocks counter
    connectionManager->algorithm.decrementResponsesSentByMeCounter();

    // update shared data only if later unlock
    if (connectionManager->algorithm.getLamportClockOfLastReceivedUnlock() < receivedMessage->getSendersClock()) {
        std::string data = receivedMessage->getData();
        manageReceivedData(data);
        connectionManager->algorithm.setLamportClockOfLastReceivedUnlock(receivedMessage->getSendersClock());
    }

    // notify if all unlocks received
    connectionManager->signalIfAllUnlocksReceived();
}

void DistributedMonitor::reactForWait(Message *receivedMessage) {
    // decrement required unlocks counter
    connectionManager->algorithm.decrementResponsesSentByMeCounter();

    d_mutex->addSenderToWaitThreadList(receivedMessage->getSendersLocalId(), receivedMessage->getSendersDistributedId(),
                                       receivedMessage->getRequestClock(), receivedMessage->getSendersClock());

    // notify if all unlocks received
    connectionManager->signalIfAllUnlocksReceived();
}

void DistributedMonitor::reactForSignalMessage(Message *receivedMessage) {
    // TODO remove
//    std::unique_lock<std::mutex> lock(*d_mutex->getLocalMutex());
//    lock.unlock();
    std::string cvName = receivedMessage->getData();
    d_cvMap[cvName]->l_notify();
}

void DistributedMonitor::reactForCommunicationEndMessage() {
    connectionManager->incrementThreadsThatWantToEndCommunicationCounter();
    if (connectionManager->receivedAllCommunicationEndMessages()) {
        std::unique_lock<std::mutex> lock(connectionManager->mutexMap["connection-end"]);
        lock.unlock();
        connectionManager->cvMap["end"].notify_all();
    }
}

void DistributedMonitor::prepareDataToSend() {
    d_mutex->setDataToSynchronize(returnDataToSend());
}

int DistributedMonitor::getDistributedId() {
    return connectionManager->getDistributedClientId();
}

void DistributedMonitor::log(std::string log) {
    connectionManager->log(log);
}

void DistributedMonitor::endCommunication2() {
    connectionManager->endConnection();
}

void DistributedMonitor::endCommunication() {
    this->endingThread = std::thread(&DistributedMonitor::endCommunication2, this);
}

void DistributedMonitor::destruct() {
    this->endingThread.join();
}
