#ifndef NPR_MONITOR_DISTRIBUTEDMONITOR_H
#define NPR_MONITOR_DISTRIBUTEDMONITOR_H

#include <cstring>
#include <memory>
#include "connection/MPI_Connection.h"
#include "connection/MPI_Msg.h"


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
    void sendMessage(std::shared_ptr<Message> message);
    std::string serializeMessage(std::shared_ptr<Message> message);
};

#endif //NPR_MONITOR_DISTRIBUTEDMONITOR_H
