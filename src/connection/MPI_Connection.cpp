#include <mpi.h>
#include "MPI_Connection.h"


MPI_Connection::MPI_Connection(int argc, char **argv) {
    createConnection(argc, argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &this->id);
    MPI_Comm_size(MPI_COMM_WORLD, &this->mpiClientsCount);
    std::cout << "My id: " << this->id << " of " << this->mpiClientsCount << "\n";
}

int MPI_Connection::getMpiClientsCount() const {
    return this->mpiClientsCount;
}


MPI_Connection::~MPI_Connection() {
    printf("Exit! Id: %d.\n", this->id);
    MPI_Finalize();
}

void MPI_Connection::createConnection(int argc, char **argv) {
    int provided = 0;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    if (provided != MPI_THREAD_MULTIPLE) {
        std::cerr << "No multiple thread support!" << std::endl;
        throw;
    }
}

int MPI_Connection::sendMessage(int recvId, MessageType type, const std::string &message) {
    MPI_Send(message.c_str(), static_cast<int>(message.length()), MPI_BYTE, recvId, type, MPI_COMM_WORLD);
}

