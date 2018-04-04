#ifndef NPR_MONITOR_MUTEXWRAPPER_H
#define NPR_MONITOR_MUTEXWRAPPER_H

#include "Mutex.h"

class MutexWrapper {
private:
    Mutex d_mutex;

public:
    explicit MutexWrapper(Mutex &m) : d_mutex(m) {
        d_mutex.lock();
    }

    ~MutexWrapper() {
        d_mutex.unlock();
    }
};

#endif //NPR_MONITOR_MUTEXWRAPPER_H
