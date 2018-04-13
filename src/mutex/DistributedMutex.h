#ifndef NPR_MONITOR_MUTEX_H
#define NPR_MONITOR_MUTEX_H

#include "../DistributedMonitor.h"

class DistributedMutex {
private:
    DistributedMonitor* distributedMonitor;

public:
    explicit DistributedMutex(DistributedMonitor* distributedMonitor) :
            distributedMonitor(distributedMonitor) {
        distributedMonitor->d_lock();
    }

    ~DistributedMutex() {
        distributedMonitor->d_unlock();
    }
};

#endif //NPR_MONITOR_MUTEX_H
