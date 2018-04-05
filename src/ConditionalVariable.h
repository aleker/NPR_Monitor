#ifndef NPR_MONITOR_CONDITIONALVARIABLE_H
#define NPR_MONITOR_CONDITIONALVARIABLE_H

#include <condition_variable>
#include "Mutex.h"

class ConditionalVariable {
private:
    std::condition_variable cv;
    Mutex* g_mutex;

public:
    explicit ConditionalVariable(Mutex *mutex) {}

    void wait();
    template< class Predicate >
    void wait(Predicate condition);
    void notifyAll();
};


#endif //NPR_MONITOR_CONDITIONALVARIABLE_H
