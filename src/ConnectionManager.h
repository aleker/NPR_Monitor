#ifndef NPR_MONITOR_CONNECTIONMANAGER_H
#define NPR_MONITOR_CONNECTIONMANAGER_H

#include <mpi.h>

class ConnectionManager {
private:
    int id;
    int mpiClientsCount;
public:
    ConnectionManager(int argc, char *argv[]);
};


#endif //NPR_MONITOR_CONNECTIONMANAGER_H
