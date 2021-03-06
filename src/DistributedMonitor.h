#ifndef NPR_MONITOR_DISTRIBUTEDMONITOR_H
#define NPR_MONITOR_DISTRIBUTEDMONITOR_H

#include <string>
#include <memory>
#include <thread>

#include "connection/ConnectionManager.h"
#include "Message.h"
#include "mutex/DistributedMutex.h"
#include "mutex/DistributedConditionVariable.h"


class DistributedMonitor {
private:
    std::thread listenThread;
    std::thread endingThread;
    std::shared_ptr<ConnectionManager> connectionManager;

    void listen();

    void reactForLockRequest(Message *receivedMessage);

    void reactForLockResponse(Message *receivedMessage);

    void reactForUnlock(Message *receivedMessage);

    void reactForWait(Message *receivedMessage);

    void reactForSignalMessage(Message *receivedMessage);

    void reactForCommunicationEndMessage();

    void endCommunication2();

    void systemLog(std::string log);

protected:
    std::shared_ptr<DistributedMutex> d_mutex;
    std::map<std::string, std::shared_ptr<DistributedConditionVariable>> d_cvMap;

    int getDistributedId();

    virtual std::string returnDataToSend() = 0;

    virtual void manageReceivedData(std::string receivedData) = 0;

public:
    explicit DistributedMonitor(std::shared_ptr<ConnectionInterface> connection);

    ~DistributedMonitor();

    void prepareDataToSend();

    void endCommunication();

    void destruct();

    void log(std::string log);
};


#endif //NPR_MONITOR_DISTRIBUTEDMONITOR_H
