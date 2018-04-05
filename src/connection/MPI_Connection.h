#ifndef NPR_MONITOR_MPI_CONNECTION_H
#define NPR_MONITOR_MPI_CONNECTION_H


class MPI_Connection {
private:
    int id;
    int mpiClientsCount;

    int createConnection(int argc, char *argv[]);

public:
    MPI_Connection(int argc, char *argv[]);
    ~MPI_Connection();

    int getId() const;
    int getMpiClientsCount() const;
};


#endif //NPR_MONITOR_MPI_CONNECTION_H
