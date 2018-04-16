#ifndef NPR_MONITOR_MPI_CONNECTION_H
#define NPR_MONITOR_MPI_CONNECTION_H

#include <vector>
#include <mpi.h>
#include <mutex>

#include "ConnectionManager.h"
#include "Message.h"

class MPI_Connection : public ConnectionManager {

private:
    int id;
    int mpiClientsCount;
    MPI_Comm* MPI_communicator = nullptr;
    int problemNo = -1;
    //std::mutex recvMessageMtx;
    static std::mutex recvMessageMtx;
    static bool initialized;

    void initialize(int argc, char **argv);
    bool isMPICommunicatorNotNull();

public:
    MPI_Connection(int argc, char *argv[]);
    MPI_Connection(int argc, char *argv[], int uniqueConnectionNo);
    ~MPI_Connection();

    int getClientId() override;
    int getClientsCount() override;
    int getUniqueConnectionNo() override;

    std::mutex* getReceiveMutex() override;

    void sendMessage(std::shared_ptr<Message> message) override ;
    void sendMessageOnBroadcast(std::shared_ptr<Message> message) override;
    Message receiveMessage() override ;
    Message receiveMessage(int tag) override ;
    Message receiveMessage(int tag, int sourceId) override;
    bool tryToReceive(int tag) override ;
    bool tryToReceive(int tag, int sourceId) override ;
};


#endif //NPR_MONITOR_MPI_CONNECTION_H
