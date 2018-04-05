#ifndef NPR_MONITOR_DISTRIBUTEDMONITOR_H
#define NPR_MONITOR_DISTRIBUTEDMONITOR_H

#include<cstring>
#include "connection/MPI_Connection.h"


class DistributedMonitor {
protected:
    ConnectionManager* connectionManager;

public:
    explicit DistributedMonitor(ConnectionManager* connectionManager) {
        this->connectionManager = connectionManager;
    }

    virtual ~DistributedMonitor();

    int getConnectionId();
};

#endif //NPR_MONITOR_DISTRIBUTEDMONITOR_H
