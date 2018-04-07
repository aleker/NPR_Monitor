#ifndef NPR_MONITOR_CONNECTIONMANAGER_H
#define NPR_MONITOR_CONNECTIONMANAGER_H


class ConnectionManager {
protected:

public:
    virtual ~ConnectionManager() = default;
    virtual int getId() = 0;
    virtual void sendMessage() = 0;
    virtual std::string receiveMessage() = 0;
};




#endif //NPR_MONITOR_CONNECTIONMANAGER_H
