#ifndef NPR_MONITOR_MUTEX_H
#define NPR_MONITOR_MUTEX_H

#include "../DistributedMonitor.h"

class DistributedMutex {
public:
    DistributedMonitor* distributedMonitor;

    explicit DistributedMutex(DistributedMonitor* distributedMonitor) :
            distributedMonitor(distributedMonitor) {
        this->d_lock();
    }

    ~DistributedMutex() {
        this->d_unlock();
    }

    void d_lock() {
        distributedMonitor->d_lock();
    }

    void d_unlock() {
        distributedMonitor->d_unlock();
    }

};

#endif //NPR_MONITOR_MUTEX_H
