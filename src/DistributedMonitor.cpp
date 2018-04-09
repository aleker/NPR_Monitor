#include <iostream>
#include "DistributedMonitor.h"

std::map<std::string, std::mutex> DistributedMonitor::uniqueClassNameToMutexMap;

// TODO setupLogFile
bool printCommonLog = true;
void setupLogFile(const char* filename) {
    std::fclose(fopen(filename, "w"));
    std::freopen(filename, "a+", stdout);
}

void createCommonLog(int id, std::string message, int clock) {
    if (printCommonLog) {
        std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()
        );
        std::cout << std::to_string(ms.count()) << " l:" << clock << " ID: " << id << " " << message << std::endl;
    }
}

DistributedMonitor::DistributedMonitor(std::unique_ptr<ConnectionManager> connectionManager) :
        connectionManager(std::move(connectionManager)) {
     static std::thread thread = std::thread(&DistributedMonitor::listen, this);
     listenThread = &thread;
//    std::map<std::string,std::mutex>::iterator it;
//    std::string name = getClassUniqueName();
//    it = DistributedMonitor::uniqueClassNameToMutexMap.find(name);
//    if (it == DistributedMonitor::uniqueClassNameToMutexMap.end())
//        DistributedMonitor::uniqueClassNameToMutexMap[name];
    // std::stringstream filename;
    // filename << "log" << this->getConnectionId() << ".txt";
    // setupLogFile(filename.str().c_str());
}

DistributedMonitor::~DistributedMonitor() {
    std::cout << "JOIN\n";
    this->listenThread->join();
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

void DistributedMonitor::sendSingleMessage(std::shared_ptr<Message> message, bool waitForReply) {
    updateLamportClock();
    message->setSendersClock(this->lamportClock);
    if (waitForReply)
        addMessageToMyNotFulfilledRequestsVector(std::make_shared<Message>(*message), 1);
    sendMessage(message);
}

int DistributedMonitor::sendMessageOnBroadcast(std::shared_ptr<Message> message, bool waitForReply) {
    updateLamportClock();
    int lamportClock = this->lamportClock;
    message->setSendersClock(lamportClock);
    int myId = connectionManager->getId();
    int clientsCount = connectionManager->getClientsCount();
    if (waitForReply)
        addMessageToMyNotFulfilledRequestsVector(std::make_shared<Message>(*message), (clientsCount-1));
    for (int i = 0; i < clientsCount; i ++) {
        if (myId != i) {
            message->setReceiversId(i);
            sendMessage(message);
        }
    }
    return lamportClock;
}

void DistributedMonitor::addMessageToMyNotFulfilledRequestsVector(std::shared_ptr<Message> message, int counter) {
    myRequest request(message->getSendersClock(), counter);
    myNotFulfilledRequestsVector.push_back(request);
}

/*
 * listen() - function called on listenThread. It manages all received messages.
 */

// TODO DistributedMonitor.listen()
void DistributedMonitor::listen() {
    std::cout << "Listen only once!\n";
    int kasia = 4;
    while (kasia > 0) {
        std::chrono::seconds sec(1);
        std::this_thread::sleep_for(sec);
        kasia--;
    }
    std::cout << "Listen end!\n";

}

/*
 * Conditional Value managing
 */

bool DistributedMonitor::checkIfGotAllReplies(int requestClock) {
    bool condition = true;
    for (myRequest item : myNotFulfilledRequestsVector) {
        if (item.clock == requestClock) {
            condition = false;
            break;
        }
    }
    return condition;
}

void DistributedMonitor::d_lock() {
    std::shared_ptr<Message> msg = std::make_shared<Message>
            (this->getConnectionId(), Message::MessageType::LOCK_MTX, getClassUniqueName());
    int lamportClockOfSendMessages = this->sendMessageOnBroadcast(msg, true);
    while (!checkIfGotAllReplies(lamportClockOfSendMessages)) {
        std::unique_lock<std::mutex> lk(getMutex());
        std::cout << "WAIT\n";
        cv.wait(lk);
    };
}

void DistributedMonitor::d_unlock() {
    std::shared_ptr<Message> msg = std::make_shared<Message>
            (this->getConnectionId(), Message::MessageType::UNLOCK_MTX, getClassUniqueName());
    this->sendMessageOnBroadcast(msg, false);
}

std::mutex& DistributedMonitor::getMutex() {
    std::string name = getClassUniqueName();
    return uniqueClassNameToMutexMap[name];
}





