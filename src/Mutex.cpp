#include "Mutex.h"

void Mutex::lock() {
    mutex.lock();
    // TODO powiadom innych że zablokowane? muszą sobie jakoś zablokować
}

void Mutex::unlock() {
    mutex.unlock();
    // TODO powiadom innych że wolne?
}

std::mutex &Mutex::getMutex() {
    return mutex;
}
