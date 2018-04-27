#include "MPI_Connection.h"

bool MPI_Connection::initialized;
// std::mutex MPI_Connection::recvMessageMtx;


MPI_Connection::MPI_Connection(int argc, char **argv, int uniqueConnectionNo) {
    this->problemNo = uniqueConnectionNo;
    initialize(argc, argv);
    int worldRank;
    MPI_communicator = new MPI_Comm();
    MPI_Comm_rank(MPI_COMM_WORLD, &worldRank);
    MPI_Comm_split(MPI_COMM_WORLD, uniqueConnectionNo, worldRank, MPI_communicator);
    MPI_Comm_rank(*MPI_communicator, &this->distributedClientId);
    MPI_Comm_size(*MPI_communicator, &this->mpiDistributedClientsCount);
}

MPI_Connection::MPI_Connection(int argc, char **argv) {
    initialize(argc, argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &this->distributedClientId);
    MPI_Comm_size(MPI_COMM_WORLD, &this->mpiDistributedClientsCount);
}

MPI_Connection::~MPI_Connection() {
    printf("Exit! Id: %d.\n", this->distributedClientId);
    MPI_Comm_free(MPI_communicator);
    MPI_Finalize();
}

int MPI_Connection::getDistributedClientId() {
    return this->distributedClientId;
}

int MPI_Connection::getDistributedClientsCount() {
    return this->mpiDistributedClientsCount;
}

void MPI_Connection::initialize(int argc, char **argv) {
    if (MPI_Connection::initialized) return;
    int provided = 0;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    if (provided != MPI_THREAD_MULTIPLE) {
        std::cerr << "No multiple thread support!" << std::endl;
        throw;
    }
    MPI_Connection::initialized = true;
}

void MPI_Connection::sendMessage(std::shared_ptr<Message> message) {
    std::string serializedMessage = Message::serializeMessage<Message>(*message.get());
    if (isMPICommunicatorNotNull()) {
        MPI_Send(serializedMessage.c_str(),
                 serializedMessage.length(),
                 MPI_BYTE,
                 message->getReceiversId(),
                 message->getMessageTypeId(),
                 *MPI_communicator);
    }
    else {
        MPI_Send(serializedMessage.c_str(),
                 serializedMessage.length(),
                 MPI_BYTE,
                 message->getReceiversId(),
                 message->getMessageTypeId(),
                 MPI_COMM_WORLD);
    }
}

void MPI_Connection::sendMessageOnBroadcast(std::shared_ptr<Message> message) {
    std::string serializedMessage = Message::serializeMessage<Message>(*message.get());
    if (isMPICommunicatorNotNull()) {
        MPI_Bcast((void *) serializedMessage.c_str(),
                  serializedMessage.length(),
                  MPI_BYTE,
                  message->getSendersDistributedId(),
                  *MPI_communicator);
    }
    else {
        MPI_Bcast((void *) serializedMessage.c_str(),
                  serializedMessage.length(),
                  MPI_BYTE,
                  message->getSendersDistributedId(),
                  MPI_COMM_WORLD);
    }
}

Message MPI_Connection::receiveMessage() {
    return receiveMessage(MPI_ANY_TAG, MPI_ANY_SOURCE);
}

Message MPI_Connection::receiveMessage(int tag) {
    return receiveMessage(tag, MPI_ANY_SOURCE);
}

Message MPI_Connection::receiveMessage(int tag, int sourceId) {
    Message messageObj;
    char receivedMessage[MAX_MSG_SIZE];
    char* receivedMessagePointer = receivedMessage;
    MPI_Status senderStatus;
    // blocking receive
    if (isMPICommunicatorNotNull()) {
        MPI_Recv(receivedMessagePointer,
                 MAX_MSG_SIZE,
                 MPI_BYTE,
                 sourceId,
                 tag,
                 *MPI_communicator,
                 &senderStatus);
    }
    else {
        MPI_Recv(receivedMessagePointer,
                 MAX_MSG_SIZE,
                 MPI_BYTE,
                 sourceId,
                 tag,
                 MPI_COMM_WORLD,
                 &senderStatus);
    }
    std::string receivedMessageString(receivedMessage);
    messageObj = Message::deserializeMessage<Message>(receivedMessageString);
    return messageObj;
}

int MPI_Connection::getUniqueConnectionNo() {
    return problemNo;
}

bool MPI_Connection::tryToReceive(int tag, int sourceId) {
    MPI_Status senderStatus;
    int flag;
    // nonblocking test for data
    if (isMPICommunicatorNotNull())
        MPI_Iprobe(sourceId, tag, *MPI_communicator, &flag,  &senderStatus);
    else
        MPI_Iprobe(sourceId, tag, MPI_COMM_WORLD, &flag,  &senderStatus);
    return (flag == 1);
}

bool MPI_Connection::tryToReceive(int tag) {
    return tryToReceive(tag, MPI_ANY_SOURCE);
}


bool MPI_Connection::isMPICommunicatorNotNull() {
    return (MPI_communicator != nullptr);
}

int MPI_Connection::getLocalClientsCount() {
    return localClientsIdsCount;
}

int MPI_Connection::addNewLocalClient() {
    this->recvMessageMtx.lock();
    this->localClientsIdsCounter += 1;
    int newId = this->localClientsIdsCounter;
    this->localClientsIdsCount = this->localClientsIdsCounter;
    this->recvMessageMtx.unlock();
    return newId;
}


