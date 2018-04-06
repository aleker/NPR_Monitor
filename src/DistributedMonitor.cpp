#include "DistributedMonitor.h"


int DistributedMonitor::getConnectionId() {
    return this->connectionManager->getId();
}

void DistributedMonitor::sendMessage(int recvId, MessageType type, const std::string &message) {
    updateLamportClock();
    connectionManager->sendMessage(recvId, type, message);
}

void DistributedMonitor::updateLamportClock() {
    this->lamportClock++;
}

void DistributedMonitor::updateLamportClock(int newValue) {
    this->lamportClock = newValue;
}

