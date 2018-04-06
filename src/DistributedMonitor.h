#ifndef NPR_MONITOR_DISTRIBUTEDMONITOR_H
#define NPR_MONITOR_DISTRIBUTEDMONITOR_H

#include <cstring>
#include <memory>
#include "connection/MPI_Connection.h"


class DistributedMonitor {
protected:
    std::unique_ptr<ConnectionManager> connectionManager;
    int lamportClock = 0;

    void updateLamportClock();
    void updateLamportClock(int newValue);

public:
    explicit DistributedMonitor(std::unique_ptr<ConnectionManager> connectionManager) :
            connectionManager(std::move(connectionManager)) { }

    virtual ~DistributedMonitor() = default;
    int getConnectionId();
    void sendMessage(int recvId, MessageType type, const std::string &message);
};

#endif //NPR_MONITOR_DISTRIBUTEDMONITOR_H
