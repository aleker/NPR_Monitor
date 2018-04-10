#ifndef NPR_MONITOR_TEST_H
#define NPR_MONITOR_TEST_H

#include <iostream>
#include "../DistributedMonitor.h"
#include "../mutex/DistributedMutex.h"

class TestSharingValues : public DistributedMonitor {
private:
    int protected_values[2] = {0, 0};
    std::string caseName = "";

public:
    explicit TestSharingValues(std::shared_ptr<ConnectionManager> connectionManager, std::string caseName)
            : DistributedMonitor(std::move(connectionManager)), caseName(caseName) {
    }
    explicit TestSharingValues(std::shared_ptr<ConnectionManager> connectionManager)
            : DistributedMonitor(std::move(connectionManager)) {
    }

    virtual ~TestSharingValues() {
    }

    void increment() {
        DistributedMutex d_mutex(this);
        // STH
        int i = getConnectionId();
        this->protected_values[i]+= (i + 5);
        //
    }

    int getProtectedValues(int i) {
        DistributedMutex d_mutex(this);
        int value = this->protected_values[i];
        return value;
    }

    void printProtectedValues() {
        for (int i = 0; i < 2 ; i++) {
            std::cout << "id=" << getConnectionId()
                      << ", val[" << i
                      << "], =" << this->protected_values[i]
                      << std::endl;
        }
    }

    std::string returnDataToSend() override {
        std::stringstream ss;
        for (int i = 0; i < 2; i++) {
            ss << protected_values[i] << " ";
        }
        return ss.str();
    }

    void manageReceivedData(std::string receivedData) override {
        std::stringstream ss;
        ss.str(receivedData);
        for (int i = 0; i < 2; i++) {
            ss >> protected_values[i];
        }
    }

    std::string getCaseName() override {
        return caseName;
    }
};

#endif //NPR_MONITOR_TEST_H





