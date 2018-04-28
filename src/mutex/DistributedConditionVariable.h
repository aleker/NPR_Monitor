#ifndef NPR_MONITOR_DISTRIBUTEDCONDITIONVARIABLE_H
#define NPR_MONITOR_DISTRIBUTEDCONDITIONVARIABLE_H

#include <mutex>
#include "DistributedMutex.h"

class DistributedConditionVariable {
private:
    DistributedMutex *d_mtx;

public:
    explicit DistributedConditionVariable(DistributedMutex *d_mtx) : d_mtx(d_mtx) { }

    void d_wait() {
        d_mtx->distributedMonitor->d_wait();
    }

    void d_notifyAll() {
        d_mtx->distributedMonitor->d_notifyAll();
    }

};

#endif //NPR_MONITOR_DISTRIBUTEDCONDITIONVARIABLE_H
