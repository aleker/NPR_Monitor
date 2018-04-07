#ifndef NPR_MONITOR_DISTRIBUTEDMONITOR_H
#define NPR_MONITOR_DISTRIBUTEDMONITOR_H

#include <cstring>
#include <memory>
#include <thread>
#include "connection/MPI_Connection.h"
#include "connection/MPI_Msg.h"


class DistributedMonitor {
protected:
    std::unique_ptr<ConnectionManager> connectionManager;
    int lamportClock = 0;
    std::thread listenThread;

    void updateLamportClock();
    void updateLamportClock(int newValue);
    void listen();

public:

    explicit DistributedMonitor(std::unique_ptr<ConnectionManager> connectionManager);
    virtual ~DistributedMonitor();
    int getConnectionId();
    void sendMessage(std::shared_ptr<Message> message);
    void sendMessageOnBroadcast(std::shared_ptr<Message> message);
    int getLamportClock() const;

};

#endif //NPR_MONITOR_DISTRIBUTEDMONITOR_H
