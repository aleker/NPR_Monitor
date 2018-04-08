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
        sendRequest();
    }

    ~MutexWrapper() {
        distributedMonitor->d_unlock();
    }

    void sendRequest() {
        std::cout << "Send request!\n";
        distributedMonitor->d_lock();
    }
};

#endif //NPR_MONITOR_MUTEX_H
