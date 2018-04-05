#ifndef NPR_MONITOR_MPI_CONNECTION_H
#define NPR_MONITOR_MPI_CONNECTION_H

#include "ConnectionManager.h"


class MPI_Connection : public ConnectionManager {
private:
protected:
    int mpiClientsCount;
    int createConnection(int argc, char **argv) override;

public:
    MPI_Connection(int argc, char *argv[]);
    ~MPI_Connection() override;
    int getMpiClientsCount() const;
};


#endif //NPR_MONITOR_MPI_CONNECTION_H
