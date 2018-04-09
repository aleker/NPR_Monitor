#ifndef NPR_MONITOR_MUTEX_H
#define NPR_MONITOR_MUTEX_H

#include "../DistributedMonitor.h"

class DistributedMutex {
private:
    DistributedMonitor* distributedMonitor;
    std::mutex& mainMutex;

public:
    explicit DistributedMutex(DistributedMonitor* distributedMonitor) :
            distributedMonitor(distributedMonitor),
            mainMutex(distributedMonitor->getMutex()) {
        std::lock_guard<std::mutex> guard(mainMutex);
        distributedMonitor->d_lock();
    }

    ~DistributedMutex() {
        distributedMonitor->d_unlock();
    }
};

#endif //NPR_MONITOR_MUTEX_H
