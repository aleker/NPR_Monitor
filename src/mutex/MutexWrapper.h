#ifndef NPR_MONITOR_MUTEX_H
#define NPR_MONITOR_MUTEX_H

#include <mutex>
#include "../MultiprocessDebugHelper.h"


class MutexWrapper {
private:
    DistributedMonitor* distributedMonitor;
    std::mutex* mainMutex;

public:
    explicit MutexWrapper(DistributedMonitor* distributedMonitor) :
        distributedMonitor(distributedMonitor) {
        mainMutex = &distributedMonitor->mainMutex;
        std::lock_guard<std::mutex> guard(*mainMutex);
        distributedMonitor->d_lock();
    }

    ~MutexWrapper() {
        distributedMonitor->d_unlock();
    }
};

#endif //NPR_MONITOR_MUTEX_H
