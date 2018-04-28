#ifndef NPR_MONITOR_DISTRIBUTEDMONITOR_H
#define NPR_MONITOR_DISTRIBUTEDMONITOR_H

#include <cstring>
#include <memory>
#include <thread>

#include "connection/ConnectionManager.h"
#include "Message.h"
#include "mutex/DistributedMutex.h"


class DistributedMonitor {
private:
    std::thread listenThread;
    std::shared_ptr<ConnectionManager> connectionManager;

    void listen();
    void reactForLockRequest(Message *receivedMessage);
    void reactForLockResponse(Message *receivedMessage);
    void reactForUnlock(Message * receivedMessage);
    void reactForWait(Message * receivedMessage);
    void reactForSignalMessage(Message * receivedMessage);

protected:
    std::shared_ptr<DistributedMutex> d_mutex;
    // TODO delete
    int getId();
    void log(std::string log);

    virtual std::string returnDataToSend() = 0;
    virtual void manageReceivedData(std::string receivedData) = 0;

public:
    explicit DistributedMonitor(ConnectionInterface* connection);
    ~DistributedMonitor();
    void prepareDataToSend();

};


#endif //NPR_MONITOR_DISTRIBUTEDMONITOR_H
