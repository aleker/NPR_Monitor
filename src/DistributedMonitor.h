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
    std::vector<std::shared_ptr<Mutex>> mutexesVector;
    // connection:
    int lamportClock = 0;
    std::vector<Message> requestsFromOthersVector;
    struct myRequest {
        int clock;
        int answerCounter;

        myRequest(int clock, int answerCounter) : clock(clock), answerCounter(answerCounter) {}
        int decrementCounter() {answerCounter--; return answerCounter;}
    };
    std::vector<myRequest> myNotFulfilledRequestsVector;

    void updateLamportClock();
    void updateLamportClock(int newValue);
    int getConnectionId();
    int getLamportClock() const;
    void listen();
    void addMessageToMyNotFulfilledRequestsVector(std::shared_ptr<Message> message, int counter);
    void sendMessage(std::shared_ptr<Message> message);
    void sendMessageOnBroadcast(std::shared_ptr<Message> message);
    void sendSingleMessage(std::shared_ptr<Message> message);
    bool checkIfMtxPosAvailable(int mtxPos);

public:
    explicit DistributedMonitor(std::unique_ptr<ConnectionManager> connectionManager);
    DistributedMonitor(std::unique_ptr<ConnectionManager> connectionManager, int protectedValuesCount);
    virtual ~DistributedMonitor();

    /*
    void put() {
        d_lock(&m);
        while (flag == 1)
            d_wait(&c, &m);
        d_signal(&c);
        d_unlock(&m);
    }
     */
    void d_lock(int mtxPos);
    void d_unlock(int mtxPos);
    void d_wait(std::shared_ptr<ConditionalVariable> cvar);
    template< class Predicate >
    void d_wait(std::shared_ptr<ConditionalVariable> cvar, Predicate condition);
    void d_signal(std::shared_ptr<ConditionalVariable> cvar);
};

#endif //NPR_MONITOR_DISTRIBUTEDMONITOR_H
