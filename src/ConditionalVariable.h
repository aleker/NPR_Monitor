#ifndef NPR_MONITOR_CONDITIONALVARIABLE_H
#define NPR_MONITOR_CONDITIONALVARIABLE_H

#include <condition_variable>
#include "Mutex.h"

class ConditionalVariable {
private:
    std::condition_variable cv;

public:
    void wait(Mutex *mtx);
    template< class Predicate >
    void wait(Mutex *mtx, Predicate condition);
    void notifyAll();
};


#endif //NPR_MONITOR_CONDITIONALVARIABLE_H
