#ifndef NPR_MONITOR_DISTRIBUTEDMONITOR_H
#define NPR_MONITOR_DISTRIBUTEDMONITOR_H

#include<cstring>
#include "connection/MPI_Connection.h"


class DistributedMonitor {
protected:
    ConnectionManager* connectionManager;
    int lamportClock = 0;

    void updateLamportClock();
    void updateLamportClock(int newValue);

public:
    explicit DistributedMonitor(ConnectionManager* connectionManager) {
        this->connectionManager = connectionManager;
    }

    virtual ~DistributedMonitor();
    int getConnectionId();
    void sendMessage(int recvId, Message* message);
};

#endif //NPR_MONITOR_DISTRIBUTEDMONITOR_H
