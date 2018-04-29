//
// Created by ola on 27.04.18.
//

#ifndef NPR_MONITOR_CONNECTIONMANAGER_H
#define NPR_MONITOR_CONNECTIONMANAGER_H


#include <mutex>
#include <condition_variable>
#include "ConnectionInterface.h"
#include "../RicardAgravala.h"
#include "../Logger.h"

class ConnectionManager {
private:
    // TODO change step to smaller
    const int clientIdStep = 100;
    int localClientId;
    int threadsThatWantToEndCommunicationCounter = 0;

    ConnectionInterface* connection;
    std::unique_ptr<Logger> logger;

    std::string messageTypeToString(int messageType);
public:
    RicardAgravala algorithm;
    std::map<std::string, std::condition_variable> cvMap;
    std::map<std::string, std::mutex> mutexMap;

    explicit ConnectionManager(ConnectionInterface *connection);

    virtual ~ConnectionManager();

    // TODO private cos?
    void sendMessage(std::shared_ptr<Message> message);
    int sendMessageOnBroadcast(std::shared_ptr<Message> message, bool waitForReply, int sendersClock = -1);
    void sendSingleMessage(std::shared_ptr<Message> message, bool waitForReply);

    int getUniqueConnectionNo();
    int getDistributedClientId();
    int getLocalClientId();

    bool tryToReceiveMessage(int messageType);
    Message receiveMessage(int messageType);

    void sendLockResponse(int receiverId, int receiversLocalId, int requestClock, std::string data = "");
    void sendUnLockMessages(std::string dataToSend);
    void sendUnLockAndWaitMessages();
    void waitForCommunicationEnd();
    void freeRequests();
    void signalIfAllUnlocksReceived();
    void signalIfAllResponsesReceived(int requestClock);

    int incrementThreadsThatWantToEndCommunicationCounter();
    bool receivedAllCommunicationEndMessages();

    void log(std::string log);

};


#endif //NPR_MONITOR_CONNECTIONMANAGER_H
