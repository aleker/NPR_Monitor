#ifndef NPR_MONITOR_CONNECTIONMANAGER_H
#define NPR_MONITOR_CONNECTIONMANAGER_H

#include <memory>

class ConnectionManager {
protected:

private:
    //ConnectionManager() {}
    //ConnectionManager(ConnectionManager const&);       // Don't Implement.
    //void operator=(ConnectionManager const&);          // Don't Implement.
public:
//    static ConnectionManager& getInstance() {
//        static ConnectionManager instance; // Guaranteed to be destroyed.
//        return instance;
//    }

    virtual ~ConnectionManager() = default;
    virtual int getId() = 0;
    virtual int getClientsCount() = 0;

    template<class MT>
    void sendMessage(std::shared_ptr<MT> message) {}
    template<class MT>
    MT receiveMessage() {}
};

#endif //NPR_MONITOR_CONNECTIONMANAGER_H
