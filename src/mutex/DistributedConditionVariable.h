#ifndef NPR_MONITOR_DISTRIBUTEDCONDITIONVARIABLE_H
#define NPR_MONITOR_DISTRIBUTEDCONDITIONVARIABLE_H

#include <mutex>
#include "DistributedMutex.h"

class DistributedConditionVariable {
private:
    DistributedMutex *mtx;
    std::condition_variable cond;
    std::unique_lock<std::mutex> lock;

public:
    explicit DistributedConditionVariable(DistributedMutex *mtx) : mtx(mtx) {

    }

    void wait() {
        cond.wait(lock);
    }

    void notifyAll() {
        cond.notify_all();
    }

};

#endif //NPR_MONITOR_DISTRIBUTEDCONDITIONVARIABLE_H
