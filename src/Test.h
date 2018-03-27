#ifndef NPR_MONITOR_TEST_H
#define NPR_MONITOR_TEST_H

#include "DistributedMonitor.h"

class Test : public DistributedMonitor {
private:
    int protected_value;

public:
    void put(int item);
    int get();

};

#endif //NPR_MONITOR_TEST_H
