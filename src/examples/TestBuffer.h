#ifndef NPR_MONITOR_TEST_H
#define NPR_MONITOR_TEST_H

#include <iostream>
#include <random>
#include "../DistributedMonitor.h"
#include "../mutex/DistributedMutex.h"

/*
 * Example of DistributedMonitor implementation
 *
 * protected_values[2] - two dimensional SHARED array
 */

class TestBuffer : public DistributedMonitor {
private:
    int protected_values[2] = {0, 0};

public:
    explicit TestBuffer(ConnectionInterface* connection)
            : DistributedMonitor(connection) {}

    void increment() {
        /*
         * d_mutex->d_lock() - distributed mutex lock
         */
        d_mutex->d_lock();
        /*
         * critical section entry
         */
        std::random_device rd;
        int i = getId();
        int randVal = rd();
        this->protected_values[i]+= (randVal);
        /*
         * prepareDataToSend() - REQUIRED after critical section, between d_lock() and d_unlock()
         */
        prepareDataToSend();
        /*
         * d_mutex->d_unlock() - distributed mutex unlock
         */
        d_mutex->d_unlock();
    }

    int getProtectedValues(int i) {
        d_mutex->d_lock();
        int value = this->protected_values[i];
        d_mutex->d_unlock();
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

/*
 * USAGE EXAMPLE
 * MPI_Connection connection(argc, argv, 4);
 *
 * TestBuffer test(&connection);
 * TestBuffer test2(&connection);
 *
 * test.increment();
 * test2.increment();
 *
 * test.printProtectedValues();
 * test2.printProtectedValues();
 *
 */

#endif //NPR_MONITOR_TEST_H





