#ifndef NPR_MONITOR_DISTRIBUTEDMONITOR_H
#define NPR_MONITOR_DISTRIBUTEDMONITOR_H

#include <cstring>
//#include <memory>
#include <thread>
#include <condition_variable>
#include <map>
#include <queue>
#include "connection/ConnectionManager.h"
#include "connection/Message.h"
#include "connection/RicardAgravala.h"
#include "Logger.h"


class DistributedMonitor {
    const int clientIdStep = 100;
public:

private:
    int localClientId;
    ConnectionManager* connectionManager;
    std::thread listenThread;
    std::map<std::string, std::mutex> mutexMap;
    std::map<std::string, std::condition_variable> cvMap;
    RicardAgravala algorithm;
    Logger logger;

    void sendMessage(std::shared_ptr<Message> message);
    int sendMessageOnBroadcast(std::shared_ptr<Message> message, bool waitForReply);
    void sendSingleMessage(std::shared_ptr<Message> message, bool waitForReply);

    void listen();
    void reactForLockRequest(Message *receivedMessage);
    void reactForLockResponse(Message *receivedMessage);
    void reactForUnlock(Message * receivedMessage);
    void sendLockResponse(int receiverId, int receiversLocalId, int requestClock, std::string data = "");
    void freeRequests();

protected:
    // TODO move to private
    int getUniqueConnectionNo();
    int getDistributedClientId();
    int getLocalClientId();
    void log(std::string log);



public:
    explicit DistributedMonitor(ConnectionManager* connectionManager);
    ~DistributedMonitor();
    virtual std::string returnDataToSend() = 0;
    virtual void manageReceivedData(std::string receivedData) = 0;

    void d_unlock();
    void d_lock();
};


#endif //NPR_MONITOR_DISTRIBUTEDMONITOR_H
