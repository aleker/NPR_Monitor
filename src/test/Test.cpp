#include "Test.h"
#include "../MutexWrapper.h"


void Test::increment() {
    this->d_mutex->lock();
    this->protected_value = this->protected_value + 1;
    this->d_mutex->unlock();
}

int Test::getProtectedValue() {
    this->d_mutex->lock();
    int value = this->protected_value;
    this->d_mutex->unlock();
    return value;
}

Test::Test(MPI_Connection *connectionManager) : DistributedMonitor(connectionManager) {
    this->d_mutex = new Mutex();
}




