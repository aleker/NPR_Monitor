#include <iostream>
#include <thread>

#include "DistributedMonitor.h"

DistributedMonitor::DistributedMonitor(std::unique_ptr<ConnectionManager> connectionManager) :
        connectionManager(std::move(connectionManager)) {
    this->listenThread = this->spawn();
}

DistributedMonitor::~DistributedMonitor() {
    this->listenThread.join();
}

int DistributedMonitor::getConnectionId() {
    return this->connectionManager->getId();
}

void DistributedMonitor::updateLamportClock() {
    this->lamportClock = this->lamportClock + 1;
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
    int kasia = 4;
    while (kasia > 0) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "W PROCESIE LISTEN: lamport = " << this->getLamportClock() << "\n";
        kasia--;
    }
}

int DistributedMonitor::getLamportClock() const {
    return lamportClock;
}

std::thread DistributedMonitor::spawn() {
    return std::thread(&DistributedMonitor::listen, this);
}


