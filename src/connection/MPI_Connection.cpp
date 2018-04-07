#include <mpi.h>

#include "MPI_Connection.h"


MPI_Connection::MPI_Connection(int argc, char **argv) {
    createConnection(argc, argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &this->id);
    MPI_Comm_size(MPI_COMM_WORLD, &this->mpiClientsCount);
    std::cout << "My id: " << this->id << " of " << this->mpiClientsCount << "\n";
}

MPI_Connection::~MPI_Connection() {
    printf("Exit! Id: %d.\n", this->id);
    MPI_Finalize();
}

int MPI_Connection::getId() {
    return this->id;
}

int MPI_Connection::getClientsCount() {
    return this->mpiClientsCount;
}

void MPI_Connection::createConnection(int argc, char **argv) {
    int provided = 0;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    if (provided != MPI_THREAD_MULTIPLE) {
        std::cerr << "No multiple thread support!" << std::endl;
        throw;
    }
}

void MPI_Connection::sendMessage(std::shared_ptr<MPI_Msg> message) {
    std::string serializedMessage = Message::serializeMessage<MPI_Msg>(*message.get());
    MPI_Send(serializedMessage.c_str(),
             sizeof(serializedMessage.c_str()),
             MPI_BYTE,
             message->getReceiversId(),
             message->getMessageType(),
             MPI_COMM_WORLD);
}

MPI_Msg MPI_Connection::receiveMessage() {
    std::string receivedMessageString;
    MPI_Status senderStatus;
    MPI_Recv(&receivedMessageString,
             MAX_MPI_MSG_SIZE,
             MPI_BYTE,
             MPI_ANY_SOURCE,
             MPI_ANY_TAG,
             MPI_COMM_WORLD,
             &senderStatus);
    MPI_Msg messageObj = Message::deserializeMessage<MPI_Msg>(receivedMessageString);
    return messageObj;
}





