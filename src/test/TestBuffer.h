#ifndef NPR_MONITOR_TEST_H
#define NPR_MONITOR_TEST_H

#include <iostream>
#include "../DistributedMonitor.h"
#include "../mutex/DistributedMutex.h"

class TestBuffer : public DistributedMonitor {
private:
    int protected_values[2] = {0, 0};

public:
    explicit TestBuffer(ConnectionManager* connectionManager)
            : DistributedMonitor(connectionManager) {}

    void increment() {
        DistributedMutex d_mutex(this);
        // STH
        int i = getDistributedClientId();
        int y = getLocalClientId();
        int z = getUniqueConnectionNo();
        // this->protected_values[i]+= (i + y + z);
        this->protected_values[i]+= (i + y);    //  + z);
        //
    }

    int getProtectedValues(int i) {
        DistributedMutex d_mutex(this);
        int value = this->protected_values[i];
        return value;
    }

    void printProtectedValues() {
        std::stringstream str;
        for (int i = 0; i < 2 ; i++) {
            str << " ,val[" << i << "] =" << this->protected_values[i];
        }
        log(str.str());
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





