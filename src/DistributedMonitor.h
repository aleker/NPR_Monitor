#ifndef NPR_MONITOR_DISTRIBUTEDMONITOR_H
#define NPR_MONITOR_DISTRIBUTEDMONITOR_H

#include <cstring>
#include <memory>
#include <thread>
#include "connection/MPI_Connection.h"
#include "mutex/Mutex.h"
#include "mutex/ConditionalVariable.h"


class DistributedMonitor {
protected:
    std::unique_ptr<ConnectionManager> connectionManager;
    std::thread listenThread;
    int lamportClock = 0;
    std::vector<Message> requestsFromOthers;
    struct myRequest {
        int clock;
        int answerCounter;

        myRequest(int clock, int answerCounter) : clock(clock), answerCounter(answerCounter) {}
        int decrementCounter() {answerCounter--; return answerCounter;}
    };
    std::vector<myRequest> myNotFulfilledRequests;

    void updateLamportClock();
    void updateLamportClock(int newValue);
    void listen();
    int getConnectionId();
    void sendMessage(std::shared_ptr<Message> message);
    void sendMessageOnBroadcast(std::shared_ptr<Message> message);
    int getLamportClock() const;

public:
    explicit DistributedMonitor(std::unique_ptr<ConnectionManager> connectionManager);
    virtual ~DistributedMonitor();

    /*
    void put() {
        lock(&m);
        while (flag == 1)
            wait(&c, &m);
        signal(&c);
        unlock(&m);
    }
     */
    void lock(std::shared_ptr<Mutex> mtx);
    void unlock(std::shared_ptr<Mutex> mtx);
    void wait(std::shared_ptr<ConditionalVariable> cvar);
    template< class Predicate >
    void wait(std::shared_ptr<ConditionalVariable> cvar, Predicate condition);
    void signal(std::shared_ptr<ConditionalVariable> cvar);
};

#endif //NPR_MONITOR_DISTRIBUTEDMONITOR_H
