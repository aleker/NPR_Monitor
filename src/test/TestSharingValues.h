#ifndef NPR_MONITOR_TEST_H
#define NPR_MONITOR_TEST_H

#include <iostream>
#include "../DistributedMonitor.h"
#include "../mutex/MutexWrapper.h"

class TestSharingValues : public DistributedMonitor {
private:
    int protected_values[2] = {0, 0};

public:
    explicit TestSharingValues(std::unique_ptr<ConnectionManager> connectionManager)
            : DistributedMonitor(std::move(connectionManager)) {
    }

    virtual ~TestSharingValues() {
    }

    void increment() {
        MutexWrapper mutexWrapper(this);
        // STH
        int i = getConnectionId();
        this->protected_values[i]+= (i + 5);
        //
    }

    int getProtectedValues(int i) {
        MutexWrapper mutexWrapper(this);
        int value = this->protected_values[i];
        return value;
    }

    void printProtectedValues() {
        for (int i = 0; i < 2; i++) {
            std::cout << "id=" << getConnectionId()
                      << ", val[" << i
                      << "], =" << this->protected_values[i]
                      << std::endl;
        }
    }
};

#endif //NPR_MONITOR_TEST_H





