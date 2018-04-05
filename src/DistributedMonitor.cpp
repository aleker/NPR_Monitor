#include "DistributedMonitor.h"


int DistributedMonitor::getConnectionId() {
    return this->connectionManager->getId();
}

DistributedMonitor::~DistributedMonitor() {
    delete connectionManager;
}

void DistributedMonitor::sendMessage(int recvId, Message* message) {
    updateLamportClock();
    connectionManager->sendMessage(recvId, message);
}

void DistributedMonitor::updateLamportClock() {
    this->lamportClock++;
}

void DistributedMonitor::updateLamportClock(int newValue) {
    this->lamportClock = newValue;
}
