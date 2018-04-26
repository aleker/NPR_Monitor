//
// Created by ola on 25.04.18.
//

#include "RicardAgravala.h"

bool RicardAgravala::updateLamportClock() {
    this->lamportClock++;
    return true;
}

bool RicardAgravala::updateLamportClock(int newValue) {
    int lamport = this->lamportClock;
    this->lamportClock = std::max(lamport, newValue) + 1;
    return newValue >= lamport;
}

RicardAgravala::myRequest RicardAgravala::getMyNotFulfilledRequest() {
    myNotFulfilledRequestMtx.lock();
    myRequest request = myNotFulfilledRequest;
    myNotFulfilledRequestMtx.unlock();
    return request;
}

void RicardAgravala::setMyNotFulfilledRequest(myRequest request) {
    myNotFulfilledRequestMtx.lock();
    myNotFulfilledRequest = request;
    myNotFulfilledRequestMtx.unlock();
}

void RicardAgravala::setMyNotFulfilledRequest(std::shared_ptr<Message> message, int counter) {
    myRequest request(message->getSendersClock(), counter);
    setMyNotFulfilledRequest(request);
}

void RicardAgravala::changeState(State state) {
    this->state = state;
}

int RicardAgravala::getNotAnsweredRepliesCount(int clock) {
    myRequest myRequest = getMyNotFulfilledRequest();
    if (clock == myRequest.clock) {
        return myRequest.answerCounter;
    }
    else return -1;
}

int RicardAgravala::getLamportClock() {
    return this->lamportClock;
}

RicardAgravala::State RicardAgravala::getState() {
    return this->state;
}

bool RicardAgravala::decrementReplyCounter(int messageClock) {
    bool decremented = false;
    myNotFulfilledRequestMtx.lock();
    if (myNotFulfilledRequest.clock == messageClock) {
        myNotFulfilledRequest.decrementCounter();
        decremented = true;
    }
    myNotFulfilledRequestMtx.unlock();
    return decremented;
}

void RicardAgravala::addReceivedRequestToQueue(Message *msg) {
    requestsFromOthersQueue.push(*msg);
}

Message RicardAgravala::removeReceivedRequestFromQueue() {
    Message message = requestsFromOthersQueue.front();
    requestsFromOthersQueue.pop();
    return message;
}

bool RicardAgravala::isRequestsFromOthersQueueEmpty() {
    return requestsFromOthersQueue.empty();
}

bool RicardAgravala::setLastUnlock(int clock) {
    if (lastUnlock.clock > clock) return false;
    lastUnlock = lastReceivedUpdatedData(clock);
    return true;
}

int RicardAgravala::getLastUnlockClock() {
    return lastUnlock.clock;
}
