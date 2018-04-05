#ifndef NPR_MONITOR_MUTEXWRAPPER_H
#define NPR_MONITOR_MUTEXWRAPPER_H

#include "Mutex.h"

class MutexWrapper {
private:
    Mutex *d_mutex;

public:
    explicit MutexWrapper(Mutex* mtx) {
        this->d_mutex = mtx;
        this->d_mutex->lock();
    }

    ~MutexWrapper() {
        this->d_mutex->unlock();
    }

    Mutex* getD_mutex() {
        return d_mutex;
    }
};

#endif //NPR_MONITOR_MUTEXWRAPPER_H
