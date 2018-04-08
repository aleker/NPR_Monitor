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

void MPI_Connection::sendMessage(std::shared_ptr<Message> message) {
    std::string serializedMessage = Message::serializeMessage<Message>(*message.get());
    MPI_Send(serializedMessage.c_str(),
             sizeof(serializedMessage.c_str()),
             MPI_BYTE,
             message->getReceiversId(),
             message->getMessageType(),
             MPI_COMM_WORLD);
}

Message MPI_Connection::receiveMessage() {
    return receiveMessage(MPI_ANY_TAG, MPI_ANY_SOURCE);
}

Message MPI_Connection::receiveMessage(int tag) {
    return receiveMessage(tag, MPI_ANY_SOURCE);
}

Message MPI_Connection::receiveMessage(int tag, int receiversId) {
    std::string receivedMessageString;
    MPI_Status senderStatus;
    MPI_Recv(&receivedMessageString,
             MAX_MSG_SIZE,
             MPI_BYTE,
             receiversId,
             tag,
             MPI_COMM_WORLD,
             &senderStatus);
    Message messageObj = Message::deserializeMessage<Message>(receivedMessageString);
    return messageObj;
}





