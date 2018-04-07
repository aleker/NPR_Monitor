#ifndef NPR_MONITOR_MPI_CONNECTION_H
#define NPR_MONITOR_MPI_CONNECTION_H

#include "ConnectionManager.h"
#include "MPI_Msg.h"

enum MessageType {
    EMPTY,
    REQUEST,
    REPLY
};

class MPI_Connection : public ConnectionManager {

private:
protected:
    int id;
    int mpiClientsCount;
    void createConnection(int argc, char **argv);
    int getMpiClientsCount() const;

public:
    MPI_Connection(int argc, char *argv[]);
    ~MPI_Connection();

    int getId() override;
    void sendMessage() override;
    void sendMessage(std::shared_ptr<MPI_Msg> message);
    std::string receiveMessage() override;
    std::string serializeMessage(std::shared_ptr<MPI_Msg> message);
};


#endif //NPR_MONITOR_MPI_CONNECTION_H
