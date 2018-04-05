#include <mpi.h>
#include "MPI_Connection.h"


int MPI_Connection::createConnection(int argc, char **argv) {
    int provided = 0;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    if (provided != MPI_THREAD_MULTIPLE) {
        std::cout << "No multiple thread support!" << std::endl;
        return -1;
    }
    return 0;
}

MPI_Connection::MPI_Connection(int argc, char **argv) {
    createConnection(argc, argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &this->id);
    MPI_Comm_size(MPI_COMM_WORLD, &this->mpiClientsCount);
    std::cout << "My id: " << this->id << " of " << this->mpiClientsCount << std::endl;
}

int MPI_Connection::getMpiClientsCount() const {
    return this->mpiClientsCount;
}

int MPI_Connection::getId() const {
    return this->id;
}

MPI_Connection::~MPI_Connection() {
    printf("Exit! Rank: %d.\n", this->id);
    MPI_Finalize();
}
