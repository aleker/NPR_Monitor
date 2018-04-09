#ifndef NPR_MONITOR_DISTRIBUTEDMONITOR_H
#define NPR_MONITOR_DISTRIBUTEDMONITOR_H

#include <cstring>
#include <memory>
#include <thread>
#include <condition_variable>
#include <map>
#include <vector>
#include "connection/ConnectionManager.h"
#include "connection/Message.h"


class DistributedMonitor {
private:
    std::unique_ptr<ConnectionManager> connectionManager;
    std::thread* listenThread;
    static std::map<std::string, std::mutex> uniqueClassNameToMutexMap;
    std::condition_variable cv;

    struct myRequest {
        int clock;
        int answerCounter;
        myRequest(int clock, int answerCounter) : clock(clock), answerCounter(answerCounter) {}
        int decrementCounter() {answerCounter--; return answerCounter;}
    };
    std::vector<myRequest> myNotFulfilledRequestsVector;
    std::vector<Message> requestsFromOthersVector;
    int lamportClock = 0;

    void updateLamportClock();
    void updateLamportClock(int newValue);
    int getLamportClock() const;

    void listen();
    void addMessageToMyNotFulfilledRequestsVector(std::shared_ptr<Message> message, int counter);
    bool checkIfGotAllReplies(int requestClock);

    void sendMessage(std::shared_ptr<Message> message);
    int sendMessageOnBroadcast(std::shared_ptr<Message> message, bool waitForReply);
    void sendSingleMessage(std::shared_ptr<Message> message, bool waitForReply);

protected:
    int getConnectionId();

public:
    explicit DistributedMonitor(std::unique_ptr<ConnectionManager> connectionManager);
    virtual ~DistributedMonitor();

    /*
    void put() {
        d_lock(&m);
        while (flag == 1)
            l_wait(&c, &m);
        l_signal(&c);
        d_unlock(&m);
    }
     */
    void d_lock();
    void d_unlock();
    virtual std::string getClassUniqueName() = 0;
    std::mutex& getMutex();
};

#endif //NPR_MONITOR_DISTRIBUTEDMONITOR_H
