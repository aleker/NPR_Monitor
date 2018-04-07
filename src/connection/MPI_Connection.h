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

public:
    MPI_Connection(int argc, char *argv[]);

    ~MPI_Connection();

    int getId() override;

    int getClientsCount() override;

    void sendMessage(std::shared_ptr<MPI_Msg> message);

    MPI_Msg receiveMessage();
};


#endif //NPR_MONITOR_MPI_CONNECTION_H
