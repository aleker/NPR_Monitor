#ifndef NPR_MONITOR_CONNECTIONMANAGER_H
#define NPR_MONITOR_CONNECTIONMANAGER_H

#include <memory>

class ConnectionManager {
protected:

public:
    virtual ~ConnectionManager() = default;
    virtual int getId() = 0;
    virtual int getClientsCount() = 0;

    template<class MT>
    void sendMessage(std::shared_ptr<MT> message) {}
    template<class MT>
    MT receiveMessage() {}
};

#endif //NPR_MONITOR_CONNECTIONMANAGER_H
