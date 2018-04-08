#ifndef NPR_MONITOR_MUTEX_H
#define NPR_MONITOR_MUTEX_H

#include <mutex>

class Mutex {
private:
    std::mutex mutex;

public:
    void lock() {
        mutex.lock();
    }
    void unlock() {
        mutex.unlock();
    }

    std::mutex* getMutex(){
        return &this->mutex;
    }

};

#endif //NPR_MONITOR_MUTEX_H
