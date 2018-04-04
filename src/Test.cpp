#include "Test.h"
#include "MutexWrapper.h"

void Test::put(int item) {
    MutexWrapper w(this->d_mutex);    // REQUIRED
    // DO SOMETHING
    this->protected_value = item;
}

int Test::get() {
    MutexWrapper w(this->d_mutex);    // REQUIRED
    // DO SOMETHING
    return this->protected_value;
}
