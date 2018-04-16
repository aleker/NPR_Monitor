#ifndef NPR_MONITOR_CONNECTIONMANAGER_H
#define NPR_MONITOR_CONNECTIONMANAGER_H

#include <mutex>
#include "Message.h"

class ConnectionManager {
public:
    virtual ~ConnectionManager() = default;
    virtual int getClientId() = 0;
    virtual int getClientsCount() = 0;
    virtual int getUniqueConnectionNo() = 0;
    virtual std::mutex* getReceiveMutex() = 0;

    virtual void sendMessage(std::shared_ptr<Message> message) = 0;
    virtual void sendMessageOnBroadcast(std::shared_ptr<Message> message) = 0;
    virtual Message receiveMessage() = 0;
    virtual Message receiveMessage(int tag) = 0;
    virtual Message receiveMessage(int tag, int sourceId) = 0;
    virtual bool tryToReceive(int tag) = 0;
    virtual bool tryToReceive(int tag, int sourceId) = 0;
};

#endif //NPR_MONITOR_CONNECTIONMANAGER_H
