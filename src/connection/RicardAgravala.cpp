//
// Created by ola on 25.04.18.
//

#include "RicardAgravala.h"

bool RicardAgravala::updateLamportClock() {
    std::lock_guard<std::mutex> lock(lamportMtx);
    this->lamportClock++;
    return true;
}

bool RicardAgravala::updateLamportClock(int newValue) {
    std::lock_guard<std::mutex> lock(lamportMtx);
    int lamport = this->lamportClock;
    this->lamportClock = std::max(lamport, newValue) + 1;
    return newValue >= lamport;
}

RicardAgravala::myRequest RicardAgravala::getMyNotFulfilledRequest() {
    std::lock_guard<std::mutex> lock(myNotFulfilledRequestMtx);
    myRequest request = myNotFulfilledRequest;
    return request;
}

void RicardAgravala::setMyNotFulfilledRequest(myRequest request) {
    std::lock_guard<std::mutex> lock(myNotFulfilledRequestMtx);
    myNotFulfilledRequest = request;
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
    std::lock_guard<std::mutex> lock(myNotFulfilledRequestMtx);
    if (myNotFulfilledRequest.clock == messageClock) {
        myNotFulfilledRequest.decrementCounter();
        decremented = true;
    }
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

void RicardAgravala::incrementResponsesSentByMeCounter() {
    responsesSentByMeCounter++;
}

void RicardAgravala::decrementResponsesSentByMeCounter() {
    responsesSentByMeCounter--;
}

int RicardAgravala::getResponsesSentByMeCounter() {
    return responsesSentByMeCounter;
}

int RicardAgravala::getLamportClockOfLastReceivedUnlock() {
    return lamportClockOfLastReceivedUnlock;
}

void RicardAgravala::setLamportClockOfLastReceivedUnlock(int newLamport) {
    lamportClockOfLastReceivedUnlock = newLamport;
}

