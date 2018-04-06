#ifndef NPR_MONITOR_MPI_CONNECTION_H
#define NPR_MONITOR_MPI_CONNECTION_H

#include "ConnectionManager.h"


class MPI_Connection : public ConnectionManager {
private:
protected:
    int id;
    int mpiClientsCount;
    void createConnection(int argc, char **argv);

public:
    MPI_Connection(int argc, char *argv[]);
    ~MPI_Connection();
    int getMpiClientsCount() const;

    int getId() override;
    // TODO message 
    void sendMessage(int recvId, MessageType type,  const std::string &message) override;
    std::string receiveMessage() override;
    std::string receiveMessage(MessageType type);
};


#endif //NPR_MONITOR_MPI_CONNECTION_H
