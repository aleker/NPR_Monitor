#ifndef NPR_MONITOR_DISTRIBUTEDMONITOR_H
#define NPR_MONITOR_DISTRIBUTEDMONITOR_H

#include <cstring>
#include <memory>
#include <thread>
#include <condition_variable>
#include <map>
#include <queue>
#include "connection/ConnectionManager.h"
#include "connection/Message.h"


class DistributedMonitor {
public:
    enum State {
        FREE,
        WAITING_FOR_REPLIES,
        IN_CRITICAL_SECTION
    };
private:
    std::shared_ptr<ConnectionManager> connectionManager;
    std::thread listenThread;
    std::map<std::string, std::mutex> mutexMap;
    std::map<std::string, std::condition_variable> cvMap;

    struct myRequest {
        int clock = -1;
        int answerCounter = -1;
        myRequest() {}
        myRequest(int clock, int answerCounter) : clock(clock), answerCounter(answerCounter) {}
        int decrementCounter() {answerCounter--; return answerCounter;}
    } myNotFulfilledRequest = myRequest();
    std::queue<Message> requestsFromOthersQueue;
    State state = State::FREE;
    int lamportClock = 0;

    void updateLamportClock();
    void updateLamportClock(int newValue);

    void listen();
    void setMessageAsMyNotFulfilledRequest(std::shared_ptr<Message> message, int counter);
    void reactForLockRequest(Message *receivedMessage);
    void reactForLockResponse(Message *receivedMessage);
    void reactForUnlock(Message * receivedMessage);
    void sendLockResponse(int receiverId, int requestClock);
    void freeRequests();
    bool checkIfGotAllReplies(int clock);
    void changeState(State state);

    void sendMessage(std::shared_ptr<Message> message);
    int sendMessageOnBroadcast(std::shared_ptr<Message> message, bool waitForReply);
    void sendSingleMessage(std::shared_ptr<Message> message, bool waitForReply);

protected:
    int getClientId();

public:
    explicit DistributedMonitor(std::shared_ptr<ConnectionManager> connectionManager);
    virtual ~DistributedMonitor();
    virtual std::string returnDataToSend() = 0;
    virtual void manageReceivedData(std::string receivedData) = 0;
    int getUniqueConnectionNo();

    void d_lock();
    void d_unlock();
};

#endif //NPR_MONITOR_DISTRIBUTEDMONITOR_H
