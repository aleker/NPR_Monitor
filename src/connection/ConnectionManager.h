#ifndef NPR_MONITOR_CONNECTIONMANAGER_H
#define NPR_MONITOR_CONNECTIONMANAGER_H

enum MessageType {
    REQUEST,
    REPLY
};

struct MessageHeader {
    int id;
    int lamportClock;
    MessageType state;
};

class ConnectionManager {
protected:

public:
    virtual ~ConnectionManager() = default;
    virtual int getId() = 0;
    virtual void sendMessage(int recvId, MessageType type, const std::string &message) = 0;
    virtual std::string receiveMessage() = 0;
};




#endif //NPR_MONITOR_CONNECTIONMANAGER_H
