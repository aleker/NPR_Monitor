#ifndef NPR_MONITOR_DISTRIBUTEDMONITOR_H
#define NPR_MONITOR_DISTRIBUTEDMONITOR_H

#include<cstring>
#include "connection/MPI_Connection.h"


class DistributedMonitor {
protected:
    MPI_Connection* connectionManager;

public:
    explicit DistributedMonitor(MPI_Connection* connectionManager) {
        this->connectionManager = connectionManager;
    }

    int getConnectionId();
};

#endif //NPR_MONITOR_DISTRIBUTEDMONITOR_H
