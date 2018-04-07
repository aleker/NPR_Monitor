#include <iostream>
#include <thread>

#include <cstdio>
#include <iostream>

#include "DistributedMonitor.h"

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
    // TODO setup logfile
    // setupLogFile(filename.str().c_str());
    this->listenThread = std::thread(&DistributedMonitor::listen, this);
}

DistributedMonitor::~DistributedMonitor() {
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
void DistributedMonitor::listen() {
    std::cout << "Listen!" << std::endl;
}





