//
// Created by ola on 25.04.18.
//

#include "RicardAgravala.h"

void RicardAgravala::updateLamportClock() {
    this->lamportClock++;
}

void RicardAgravala::updateLamportClock(int newValue) {
    this->lamportClock = std::max(this->lamportClock, newValue) + 1;
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

bool RicardAgravala::checkIfGotAllReplies(int clock) {
    myRequest myRequest = getMyNotFulfilledRequest();
    if (clock == myRequest.clock) {
        return (myRequest.answerCounter <= 0);
    }
    else return true;
}

int RicardAgravala::getLamportClock() {
    return this->lamportClock;
}

RicardAgravala::State RicardAgravala::getState() {
    return this->state;
}

void RicardAgravala::decrementReplyCounter(int messageClock) {
    myNotFulfilledRequestMtx.lock();
    if (myNotFulfilledRequest.clock == messageClock) {
        myNotFulfilledRequest.decrementCounter();
    }
    myNotFulfilledRequestMtx.unlock();
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
