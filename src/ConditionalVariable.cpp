//
// Created by ola on 04.04.18.
//

#include "ConditionalVariable.h"

void ConditionalVariable::wait(Mutex *mtx) {
    // mutex wrapper:
    std::unique_lock<std::mutex> lk(mtx->getMutex());
    cv.wait(lk);

    // TODO powiadom wszystkich, że masz waita na cv w mtx
}

template<class Predicate>
void ConditionalVariable::wait(Mutex *mtx, Predicate condition) {
    // mutex wrapper:
    std::unique_lock<std::mutex> lk(mtx->getMutex());
    cv.wait(lk, condition);

    // TODO powiadom wszystkich, że masz waita na cv w mtx dopóki !condition

}

void ConditionalVariable::notifyAll() {
    cv.notify_all();

    // TODO powiadom wszystkich o cv
}