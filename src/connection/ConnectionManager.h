#ifndef NPR_MONITOR_CONNECTIONMANAGER_H
#define NPR_MONITOR_CONNECTIONMANAGER_H

#include <mutex>
#include "Message.h"

class ConnectionManager {
public:
    virtual ~ConnectionManager() = default;
    virtual int getId() = 0;
    virtual int getClientsCount() = 0;
    virtual int getProblemNo() = 0;
    virtual std::mutex* getReceiveMutex() = 0;

    virtual void sendMessage(std::shared_ptr<Message> message) {}
    virtual Message receiveMessage() = 0;
    virtual Message receiveMessage(int tag) = 0;
    virtual Message receiveMessage(int tag, int sourceId) = 0;
    virtual bool tryToReceive(int tag) = 0;
    virtual bool tryToReceive(int tag, int sourceId) = 0;
};

#endif //NPR_MONITOR_CONNECTIONMANAGER_H
