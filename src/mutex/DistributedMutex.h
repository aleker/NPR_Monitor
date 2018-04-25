#ifndef NPR_MONITOR_MUTEX_H
#define NPR_MONITOR_MUTEX_H

#include "../DistributedMonitor.h"

class DistributedMutex {
private:
    DistributedMonitor* distributedMonitor;
    std::condition_variable* gotAllRepliesCV;
    std::unique_lock<std::mutex> criticalMtxLock;

public:
    explicit DistributedMutex(DistributedMonitor* distributedMonitor) :
            distributedMonitor(distributedMonitor) {
        criticalMtxLock = std::unique_lock<std::mutex>(*distributedMonitor->getCriticalMutex(), std::defer_lock);
        gotAllRepliesCV = distributedMonitor->getCriticalConditionVariable();
        this->d_lock();
    }

    ~DistributedMutex() {
        this->d_unlock();
    }

    void d_lock() {
        int tryToLockClock = distributedMonitor->tryToLock();
        criticalMtxLock.lock(); // = std::unique_lock<std::mutex>(mutexMap["critical-section"]);
        while (!distributedMonitor->algorithm.checkIfGotAllReplies(tryToLockClock)) {
            distributedMonitor->log("WAIT");
            gotAllRepliesCV->wait(criticalMtxLock);
        };
        distributedMonitor->goToCriticalSection();
    }

    void d_unlock() {
        criticalMtxLock.unlock();
        distributedMonitor->d_unlock();
    }

};

#endif //NPR_MONITOR_MUTEX_H
