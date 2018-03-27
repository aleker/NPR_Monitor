#ifndef NPR_MONITOR_MUTEXWRAPPER_H
#define NPR_MONITOR_MUTEXWRAPPER_H

#include "Mutex.h"

class MutexWrapper {
private:
    Mutex mtx;

public:
    explicit MutexWrapper(Mutex &m) : mtx(m) {
        mtx.lock();
    }

    ~MutexWrapper() {
        mtx.unlock();
    }
};

#endif //NPR_MONITOR_MUTEXWRAPPER_H
