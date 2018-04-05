#include "DistributedMonitor.h"


int DistributedMonitor::getConnectionId() {
    return this->connectionManager->getId();
}

DistributedMonitor::~DistributedMonitor() {
    delete connectionManager;
}
