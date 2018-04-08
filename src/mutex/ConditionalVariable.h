#ifndef NPR_MONITOR_CONDITIONALVARIABLE_H
#define NPR_MONITOR_CONDITIONALVARIABLE_H

#include <condition_variable>
#include "Mutex.h"

class ConditionalVariable {
private:
    std::condition_variable cv;
    std::shared_ptr<Mutex> g_mutex;

public:
    explicit ConditionalVariable(std::shared_ptr<Mutex> g_mutex) : g_mutex(g_mutex) {}

    void wait() {
        // g_mutex wrapper:
        std::mutex* mtx = this->g_mutex->getMutex();
        std::unique_lock<std::mutex> lk(*mtx);
        cv.wait(lk);
    }

    template< class Predicate >
    void wait(Predicate condition) {
        std::mutex* mtx = this->g_mutex->getMutex();
        std::unique_lock<std::mutex> lk(*mtx);
        cv.wait(lk, condition);
    }

    void notifyAll() {
        cv.notify_all();
    }
};


#endif //NPR_MONITOR_CONDITIONALVARIABLE_H



