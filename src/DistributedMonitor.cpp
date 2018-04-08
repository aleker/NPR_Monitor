#include <iostream>
#include "DistributedMonitor.h"

// TODO setupLogFile
bool printCommonLog = true;
void setupLogFile(const char* filename) {
    std::fclose(fopen(filename, "w"));
    std::freopen(filename, "a+", stdout);
}

void createCommonLog(int id, std::string message, int clock) {
    if (printCommonLog) {
        using namespace std::chrono;
        milliseconds ms = duration_cast<milliseconds>(
                system_clock::now().time_since_epoch()
        );
        std::cout << std::to_string(ms.count()) << " l:" << clock << " ID: " << id << " " << message << std::endl;
    }
}

DistributedMonitor::DistributedMonitor(std::unique_ptr<ConnectionManager> connectionManager) :
        connectionManager(std::move(connectionManager)) {
    std::stringstream filename;
    filename << "log" << this->getConnectionId() << ".txt";
    std::cout << "filename= " << filename.str() << "\n";
    // setupLogFile(filename.str().c_str());
    this->listenThread = std::thread(&DistributedMonitor::listen, this);
}

DistributedMonitor::DistributedMonitor(std::unique_ptr<ConnectionManager> connectionManager, int protectedValuesCount) :
        connectionManager(std::move(connectionManager)) {
    std::stringstream filename;
    filename << "log" << this->getConnectionId() << ".txt";
    std::cout << "filename= " << filename.str() << "\n";
    // setupLogFile(filename.str().c_str());
    this->listenThread = std::thread(&DistributedMonitor::listen, this);
    if (protectedValuesCount < 0) {
        std::cerr << "Wrong argument value! Must be positive!\n";
        throw;
    }
    for (int i = 0; i < protectedValuesCount; i++) {
        mutexesVector.push_back(std::make_shared<Mutex>());
    }
}

DistributedMonitor::~DistributedMonitor() {
    std::cout << "JOIN\n";
    this->listenThread.join();
}

int DistributedMonitor::getConnectionId() {
    return this->connectionManager->getId();
}

int DistributedMonitor::getLamportClock() const {
    return lamportClock;
}

void DistributedMonitor::updateLamportClock() {
    this->lamportClock++;
}

void DistributedMonitor::updateLamportClock(int newValue) {
    this->lamportClock = newValue;
}

void DistributedMonitor::sendMessage(std::shared_ptr<Message> message) {
    if (message->getReceiversId() == NOT_SET) {
        std::cerr << "Receivers id not set!\n";
        throw "Receivers id not set!\n";
        return;
    }
    connectionManager->sendMessage(message);
    std::cout << "Message sent!" << std::endl;
}

void DistributedMonitor::sendSingleMessage(std::shared_ptr<Message> message) {
    updateLamportClock();
    message->setSendersClock(this->lamportClock);
    addMessageToMyNotFulfilledRequestsVector(std::make_shared<Message>(*message), 1);
    sendMessage(message);
}

void DistributedMonitor::sendMessageOnBroadcast(std::shared_ptr<Message> message) {
    updateLamportClock();
    message->setSendersClock(this->lamportClock);
    int myId = connectionManager->getId();
    int clientsCount = connectionManager->getClientsCount();
    addMessageToMyNotFulfilledRequestsVector(std::make_shared<Message>(*message), (clientsCount-1));
    for (int i = 0; i < clientsCount; i ++) {
        if (myId != i) {
            message->setReceiversId(i);
            sendMessage(message);
        }
    }
}

void DistributedMonitor::addMessageToMyNotFulfilledRequestsVector(std::shared_ptr<Message> message, int counter) {
    myRequest request(message->getSendersClock(), counter);
    myNotFulfilledRequestsVector.push_back(request);
}

/*
 * listen() - function called on listenThread
 */

// TODO DistributedMonitor.listen()
void DistributedMonitor::listen() {
//    int kasia = 2;
//    while(kasia > 0) {
//        std::cout << "Listening...." << std::endl;
//        std::chrono::seconds sec(1);
//        std::this_thread::sleep_for(sec);
//        kasia--;
//    }
}

/*
 * Mutex managing
 */

bool DistributedMonitor::checkIfMtxPosAvailable(int mtxPos) {
    return ((unsigned int)mtxPos < this->mutexesVector.size());
}


// TODO DistributedMonitor.d_lock()
void DistributedMonitor::d_lock(int mtxPos) {
    if (!checkIfMtxPosAvailable(mtxPos)) return;
    std::shared_ptr<Mutex> mtx = this->mutexesVector.at(mtxPos);
    // send request for lock
    std::shared_ptr<Message> msg = std::make_shared<Message>
            (this->getConnectionId(), Message::MessageType::REQUEST_MTX);
    this->sendMessageOnBroadcast(msg);
    // TODO wait for all answers (local wait)
    mtx->lock();
}

// TODO DistributedMonitor.d_unlock()
void DistributedMonitor::d_unlock(int mtxPos) {
    if (!checkIfMtxPosAvailable(mtxPos)) return;
    std::shared_ptr<Mutex> mtx = this->mutexesVector.at(mtxPos);
    mtx->unlock();
    // cdn
}

// TODO DistributedMonitor.d_wait()
void DistributedMonitor::d_wait(std::shared_ptr<ConditionalVariable> cvar) {
    cvar->wait();
    // cdn

}

template<class Predicate>
void
DistributedMonitor::d_wait(std::shared_ptr<ConditionalVariable> cvar, Predicate condition) {
    cvar->wait(condition);
    // cdn
}

// TODO DistributedMonitor.d_signal()
void DistributedMonitor::d_signal(std::shared_ptr<ConditionalVariable> cvar) {
    cvar->notifyAll();
    // cdn
}





