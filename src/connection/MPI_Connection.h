#ifndef NPR_MONITOR_MPI_CONNECTION_H
#define NPR_MONITOR_MPI_CONNECTION_H

#include "ConnectionManager.h"


class MPI_Connection : public ConnectionManager {
private:
protected:
    int mpiClientsCount;
    void createConnection(int argc, char **argv) override;

public:
    MPI_Connection(int argc, char *argv[]);
    ~MPI_Connection() override;
    int getMpiClientsCount() const;

    int sendMessage(int recvId, Message* message) override;
};


#endif //NPR_MONITOR_MPI_CONNECTION_H
