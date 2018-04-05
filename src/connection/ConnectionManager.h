#ifndef NPR_MONITOR_CONNECTIONMANAGER_H
#define NPR_MONITOR_CONNECTIONMANAGER_H


class ConnectionManager {
protected:
    int id = 0;
    virtual int createConnection(int argc, char **argv) = 0;

public:
    ConnectionManager() = default;
    virtual ~ConnectionManager() = default;
    virtual int getId() const {
        return this->id;
    };
};




#endif //NPR_MONITOR_CONNECTIONMANAGER_H
