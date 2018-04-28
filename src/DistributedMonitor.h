#ifndef NPR_MONITOR_DISTRIBUTEDMONITOR_H
#define NPR_MONITOR_DISTRIBUTEDMONITOR_H

#include <cstring>
#include <memory>
#include <thread>
#include <condition_variable>
#include <map>
#include <queue>

#include "connection/ConnectionInterface.h"
#include "Message.h"
#include "RicardAgravala.h"
#include "Logger.h"


class DistributedMonitor {
    // TODO change step to smaller
    const int clientIdStep = 100;
public:
    struct WaitInfo {
        int localId = -1;
        int distributedId = -1;
        int originalRequestClock = -1;
        int waitMessageClock = -1;
        WaitInfo(int localId, int distributedId,  int originalRequestClock, int waitMessageClock) :
                localId(localId), distributedId(distributedId), originalRequestClock(originalRequestClock),
                waitMessageClock(waitMessageClock) {}
    };

private:
    int localClientId;
    ConnectionInterface* connectionManager;
    std::thread listenThread;
    std::map<std::string, std::mutex> mutexMap;
    std::map<std::string, std::condition_variable> cvMap;
    std::vector<WaitInfo> waitingThreadsVector;
    RicardAgravala algorithm;
    std::unique_ptr<Logger> logger;

    void sendMessage(std::shared_ptr<Message> message);
    int sendMessageOnBroadcast(std::shared_ptr<Message> message, bool waitForReply, int sendersClock = -1);
    void sendSingleMessage(std::shared_ptr<Message> message, bool waitForReply);

    void listen();
    void reactForLockRequest(Message *receivedMessage);
    void reactForLockResponse(Message *receivedMessage);
    void reactForUnlock(Message * receivedMessage);
    void reactForWait(Message * receivedMessage);
    void reactForSignalMessage(Message * receivedMessage);

    void sendUnLockMessages();
    void sendUnLockAndWaitMessages();
    void sendLockResponse(int receiverId, int receiversLocalId, int requestClock, std::string data = "");
    void freeRequests();

    void signalIfAllUnlocksReceived();
    void addSenderToWaitThreadList(int localId, int distributedId, int originalRequestClock, int waitMessageClock);
    void removeThisThreadFromWaitingList(int localId, int distributedId);

protected:
    // TODO move to private
    int getUniqueConnectionNo();
    int getDistributedClientId();
    int getLocalClientId();
    void log(std::string log);

public:
    explicit DistributedMonitor(ConnectionInterface* connectionManager);
    ~DistributedMonitor();
    virtual std::string returnDataToSend() = 0;
    virtual void manageReceivedData(std::string receivedData) = 0;

    void d_unlock();
    void d_lock(int sendersClock = -1);
    void d_wait();
    void d_notifyAll();
};


#endif //NPR_MONITOR_DISTRIBUTEDMONITOR_H
