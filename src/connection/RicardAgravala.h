//
// Created by ola on 25.04.18.
//

#ifndef NPR_MONITOR_RICARDAGRAVALA_H
#define NPR_MONITOR_RICARDAGRAVALA_H

#include <queue>
#include <mutex>
#include "Message.h"

class RicardAgravala {
public:
    enum State {
        FREE,
        WAITING_FOR_REPLIES,
        IN_CRITICAL_SECTION
    };

    struct myRequest {
        int clock = -1;
        int answerCounter = -1;
        myRequest() {}
        myRequest(int clock, int answerCounter) : clock(clock), answerCounter(answerCounter) {}
        int decrementCounter() {answerCounter--; return answerCounter;}
    };

    struct lastReceivedUpdatedData {
        int clock = -1;
        lastReceivedUpdatedData() : clock(-1) {}
        lastReceivedUpdatedData(int clock) : clock(clock) {}
    };

private:
    myRequest myNotFulfilledRequest = myRequest();
    lastReceivedUpdatedData lastUnlock = lastReceivedUpdatedData();
    std::queue<Message> requestsFromOthersQueue;
    int lamportClock = 0;
    State state = State::FREE;
    std::mutex myNotFulfilledRequestMtx;

public:
    int getLamportClock();
    bool updateLamportClock();
    bool updateLamportClock(int newValue);

    void setMyNotFulfilledRequest(myRequest request);
    void setMyNotFulfilledRequest(std::shared_ptr<Message> message, int counter);
    myRequest getMyNotFulfilledRequest();
    int getNotAnsweredRepliesCount(int clock);

    void addReceivedRequestToQueue(Message *msg);
    Message removeReceivedRequestFromQueue();
    bool isRequestsFromOthersQueueEmpty();

    State getState();
    void changeState(State state);

    bool decrementReplyCounter(int messageClock);
    bool setLastUnlock(int clock);
    int getLastUnlockClock();

};


#endif //NPR_MONITOR_RICARDAGRAVALA_H
