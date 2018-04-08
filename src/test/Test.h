#ifndef NPR_MONITOR_TEST_H
#define NPR_MONITOR_TEST_H

#include "../DistributedMonitor.h"
#include "../mutex/Mutex.h"

class Test : public DistributedMonitor {
private:
    int protected_value = 0;
    Mutex* d_mutex;

public:
    explicit Test(std::unique_ptr<ConnectionManager> connectionManager);
    void increment();
    int getProtectedValue();
};

#endif //NPR_MONITOR_TEST_H
