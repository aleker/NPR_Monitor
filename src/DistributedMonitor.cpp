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
    updateLamportClock();
    message->setSendersClock(this->lamportClock);
    connectionManager->sendMessage(message);
    std::cout << "Message sent!" << std::endl;
}

void DistributedMonitor::sendMessageOnBroadcast(std::shared_ptr<Message> message) {
    int myId = connectionManager->getId();
    for (int i = 0; i < connectionManager->getClientsCount(); i ++) {
        if (myId != i) {
            sendMessage(message);
        }
    }
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
    return (mtxPos < this->mutexesVector.size());
}


// TODO DistributedMonitor.lock()
void DistributedMonitor::lock(int mtxPos) {
    if (!checkIfMtxPosAvailable(mtxPos)) return;
    std::shared_ptr<Mutex> mtx = this->mutexesVector.at(mtxPos);
    mtx->lock();
    // cdn
}

// TODO DistributedMonitor.unlock()
void DistributedMonitor::unlock(int mtxPos) {
    if (!checkIfMtxPosAvailable(mtxPos)) return;
    std::shared_ptr<Mutex> mtx = this->mutexesVector.at(mtxPos);
    mtx->unlock();
    // cdn
}

// TODO DistributedMonitor.wait()
void DistributedMonitor::wait(std::shared_ptr<ConditionalVariable> cvar) {
    cvar->wait();
    // cdn

}

template<class Predicate>
void
DistributedMonitor::wait(std::shared_ptr<ConditionalVariable> cvar, Predicate condition) {
    cvar->wait(condition);
    // cdn
}

// TODO DistributedMonitor.signal()
void DistributedMonitor::signal(std::shared_ptr<ConditionalVariable> cvar) {
    cvar->notifyAll();
    // cdn
}

