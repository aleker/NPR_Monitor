#include "DistributedMonitor.h"


int DistributedMonitor::getConnectionId() {
    return this->connectionManager->getId();
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
    // TODO co z tymi argumentami
    // connectionManager->sendMessage(message);
    connectionManager->sendMessage();
}


