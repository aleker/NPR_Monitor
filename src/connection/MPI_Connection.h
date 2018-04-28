#ifndef NPR_MONITOR_MPI_CONNECTION_H
#define NPR_MONITOR_MPI_CONNECTION_H

#include <vector>
#include <mpi.h>
#include <mutex>

#include "ConnectionInterface.h"
#include "../Message.h"

class MPI_Connection : public ConnectionInterface {

private:
    int localClientsIdsCounter = 0;
    int localClientsIdsCount = 0;
    int distributedClientId;
    int mpiDistributedClientsCount;
    MPI_Comm* MPI_communicator = nullptr;
    int problemNo = -1;
    std::mutex recvMessageMtx;
    static bool initialized;

    void initialize(int argc, char **argv);
    bool isMPICommunicatorNotNull();

public:
    MPI_Connection(int argc, char *argv[]);
    MPI_Connection(int argc, char *argv[], int uniqueConnectionNo);
    ~MPI_Connection();

    int getDistributedClientId() override;
    int getDistributedClientsCount() override;
    int getUniqueConnectionNo() override;
    int getLocalClientsCount() override;
    int addNewLocalClient() override;

    void sendMessage(std::shared_ptr<Message> message) override ;
    Message receiveMessage() override ;
    Message receiveMessage(int tag) override ;
    Message receiveMessage(int tag, int sourceId) override;
    bool tryToReceive(int tag) override ;
    bool tryToReceive(int tag, int sourceId) override ;
};


#endif //NPR_MONITOR_MPI_CONNECTION_H
