#include <iostream>
#include "RicartAgrawala.h"

bool RicartAgrawala::updateLamportClock() {
    std::lock_guard<std::mutex> lock(lamportMtx);
    this->lamportClock++;
    return true;
}

bool RicartAgrawala::updateLamportClock(int newValue) {
    std::lock_guard<std::mutex> lock(lamportMtx);
    int lamport = this->lamportClock;
    this->lamportClock = std::max(lamport, newValue) + 1;
    return newValue >= lamport;
}

RicartAgrawala::myRequest RicartAgrawala::getMyNotFulfilledRequest() {
    std::lock_guard<std::mutex> lock(myNotFulfilledRequestMtx);
    myRequest request = myNotFulfilledRequest;
    return request;
}

void RicartAgrawala::setMyNotFulfilledRequest(myRequest request) {
    std::lock_guard<std::mutex> lock(myNotFulfilledRequestMtx);
    myNotFulfilledRequest = request;
}

void RicartAgrawala::setMyNotFulfilledRequest(std::shared_ptr<Message> message, int counter) {
    myRequest request(message->getSendersClock(), counter);
    setMyNotFulfilledRequest(request);
}

void RicartAgrawala::changeState(State state) {
    this->state = state;
}

int RicartAgrawala::getNotAnsweredRepliesCount(int clock) {
    myRequest myRequest = getMyNotFulfilledRequest();
    if (clock == myRequest.clock) {
        return myRequest.answerCounter;
    } else return -1;
}

int RicartAgrawala::getLamportClock() {
    return this->lamportClock;
}

RicartAgrawala::State RicartAgrawala::getState() {
    return this->state;
}

bool RicartAgrawala::decrementReplyCounter(int messageClock) {
    bool decremented = false;
    std::lock_guard<std::mutex> lock(myNotFulfilledRequestMtx);
    if (myNotFulfilledRequest.clock == messageClock) {
        myNotFulfilledRequest.decrementCounter();
        decremented = true;
    }
    return decremented;
}

void RicartAgrawala::addReceivedRequestToQueue(Message *msg) {
    requestsFromOthersQueue.push(*msg);
}

Message RicartAgrawala::removeReceivedRequestFromQueue() {
    Message message = requestsFromOthersQueue.front();
    requestsFromOthersQueue.pop();
    return message;
}

bool RicartAgrawala::isRequestsFromOthersQueueEmpty() {
    return requestsFromOthersQueue.empty();
}

void RicartAgrawala::incrementResponsesSentByMeCounter() {
    responsesSentByMeCounter++;
}

void RicartAgrawala::decrementResponsesSentByMeCounter() {
    responsesSentByMeCounter--;
}

int RicartAgrawala::getResponsesSentByMeCounter() {
    return responsesSentByMeCounter;
}

int RicartAgrawala::getLamportClockOfLastReceivedUnlock() {
    return lamportClockOfLastReceivedUnlock;
}

void RicartAgrawala::setLamportClockOfLastReceivedUnlock(int newLamport) {
    lamportClockOfLastReceivedUnlock = newLamport;
}
