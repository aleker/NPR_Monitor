#ifndef NPR_MONITOR_TEST_H
#define NPR_MONITOR_TEST_H

#include <iostream>
#include "../DistributedMonitor.h"
#include "../mutex/DistributedMutex.h"

class Buffer : public DistributedMonitor {
private:
    int protected_values[2] = {0, 0};

public:
    explicit Buffer(std::shared_ptr<ConnectionManager> connectionManager)
            : DistributedMonitor(std::move(connectionManager)) {
    }

    virtual ~Buffer() {}

    void increment() {
        DistributedMutex d_mutex(this);
        // STH
        int i = getClientId();
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
            std::cout << "id=" << getClientId()
                      << ", val[" << i
                      << "], =" << this->protected_values[i]
                      << std::endl;
        }
    }

    std::string returnDataToSend() override {
        std::stringstream ss;
        for (int protected_value : protected_values) {
            ss << protected_value << " ";
        }
        return ss.str();
    }

    void manageReceivedData(std::string receivedData) override {
        std::stringstream ss;
        ss.str(receivedData);
        for (int &protected_value : protected_values) {
            ss >> protected_value;
        }
    }

};

#endif //NPR_MONITOR_TEST_H





