#ifndef NPR_MONITOR_CONNECTIONMANAGER_H
#define NPR_MONITOR_CONNECTIONMANAGER_H

enum MessageType {
    REQUEST,
    REPLY
};

class ConnectionManager {
protected:
    int id = 0;
    virtual void createConnection(int argc, char **argv) = 0;

public:
    ConnectionManager() = default;
    virtual ~ConnectionManager() = default;
    virtual int getId() const {
        return this->id;
    };
    virtual int sendMessage(int recvId, MessageType type,  const std::string &message) = 0;
};




#endif //NPR_MONITOR_CONNECTIONMANAGER_H
