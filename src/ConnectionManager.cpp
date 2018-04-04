#include <iostream>
#include "ConnectionManager.h"

ConnectionManager::ConnectionManager(int argc, char *argv[]) {
    int provided = 0;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    if (provided != MPI_THREAD_MULTIPLE) {
        std::cout << "No mpi support!" << std::endl;
        exit(-1);
    }

    MPI_Comm_rank(MPI_COMM_WORLD, &this->id);
    MPI_Comm_size(MPI_COMM_WORLD, &this->mpiClientsCount);
}
