#ifndef NPR_MONITOR_CONNECTIONMANAGER_H
#define NPR_MONITOR_CONNECTIONMANAGER_H

class ConnectionManager {
protected:
    int id;

    virtual int createConnection() = 0;
public:
    int getId() const {
        return id;
    }
};




#endif //NPR_MONITOR_CONNECTIONMANAGER_H
