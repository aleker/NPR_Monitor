#ifndef NPR_MONITOR_TEST_H
#define NPR_MONITOR_TEST_H

#include <iostream>
#include "../DistributedMonitor.h"
#include "../mutex/Mutex.h"

class TestSharingValues : public DistributedMonitor {
private:
    int protected_values[2] = {0, 0};

public:
    explicit TestSharingValues(std::unique_ptr<ConnectionManager> connectionManager)
            : DistributedMonitor(std::move(connectionManager)) {
    }
    TestSharingValues(std::unique_ptr<ConnectionManager> connectionManager, int protectedValuesCount)
            : DistributedMonitor(std::move(connectionManager), protectedValuesCount) {
    }

    virtual ~TestSharingValues() {
    }

    void increment() {
        d_lock(0);
        int i = connectionManager->getId();
        this->protected_values[i]+= (i + 5);
        d_unlock(0);
    }

    int getProtectedValues(int i) {
        d_lock(0);
        int value = this->protected_values[i];
        d_unlock(0);
        return value;
    }

    void printProtectedValues() {
        for (int i = 0; i < 2; i++) {
            std::cout << "id=" << connectionManager->getId()
                      << ", val[" << i
                      << "], =" << this->protected_values[i]
                      << std::endl;
        }
    }
};

#endif //NPR_MONITOR_TEST_H





