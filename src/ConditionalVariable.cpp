//
// Created by ola on 04.04.18.
//

#include "ConditionalVariable.h"
#include <mutex>

void ConditionalVariable::wait() {
    // g_mutex wrapper:
    std::mutex* mtx = this->g_mutex->getMutex();
    std::unique_lock<std::mutex> lk(*mtx);
    cv.wait(lk);

    // TODO powiadom wszystkich, że masz waita na cv w mtx
}

template<class Predicate>
void ConditionalVariable::wait(Predicate condition) {
    std::mutex* mtx = this->g_mutex->getMutex();
    std::unique_lock<std::mutex> lk(*mtx);
    cv.wait(lk, condition);

    // TODO powiadom wszystkich, że masz waita na cv w mtx dopóki !condition

}

void ConditionalVariable::notifyAll() {
    cv.notify_all();

    // TODO powiadom wszystkich o cv
}

