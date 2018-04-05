#ifndef NPR_MONITOR_MUTEX_H
#define NPR_MONITOR_MUTEX_H

#include <mutex>

class Mutex {
private:
    std::mutex mutex;

public:
    void lock();
    void unlock();
    std::mutex* getMutex();


};

#endif //NPR_MONITOR_MUTEX_H
