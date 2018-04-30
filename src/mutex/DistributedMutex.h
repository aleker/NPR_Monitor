#ifndef NPR_MONITOR_MUTEX_H
#define NPR_MONITOR_MUTEX_H

#include <mutex>
#include <vector>
#include <memory>

class DistributedMutex {
private:
    std::string protectedData;
    std::mutex l_mutex;
    int lastRequestThatWeResponsedClock = 0;

public:
    struct WaitInfo {
        int localId = -1;
        int distributedId = -1;
        int originalRequestClock = -1;
        int waitMessageClock = -1;
        WaitInfo(int localId, int distributedId,  int originalRequestClock, int waitMessageClock) :
                localId(localId), distributedId(distributedId), originalRequestClock(originalRequestClock),
                waitMessageClock(waitMessageClock) {}
    };
    std::shared_ptr<ConnectionManager> connectionManager;
    std::vector<WaitInfo> waitingThreadsVector;
    std::mutex waitingThreadsVectorMutex;
    std::mutex stateMutex;

    explicit DistributedMutex(std::shared_ptr<ConnectionManager> connectionManager) :
            connectionManager(connectionManager) {}

    ~DistributedMutex() {}

    void d_lock(int requestClock = -1) {
        // SEND REQUEST FOR CRITICAL SECTION
        stateMutex.lock();
        connectionManager->algorithm.changeState(RicardAgravala::State::WAITING_FOR_REPLIES);
        std::shared_ptr<Message> msg = std::make_shared<Message>
                (connectionManager->getDistributedClientId(), connectionManager->getLocalClientId(), Message::MessageType::LOCK_MTX);
        int lastSentLockClock = connectionManager->sendMessageOnBroadcast(msg, true, requestClock);
        stateMutex.unlock();

        // CRITICAL SECTION ENTRY
        std::unique_lock<std::mutex> lock(connectionManager->mutexMap["global-lock"]);
        while (connectionManager->algorithm.getNotAnsweredRepliesCount(lastSentLockClock) > 0) {
            std::stringstream str;
            str << "WAIT for critical section (" << lastSentLockClock << ")";
            connectionManager->log(str.str());
            connectionManager->cvMap["receivedAllReplies"].wait(lock);
        };

        // WAIT FOR ALL UNLOCK MESSAGES
        std::unique_lock<std::mutex> lock2(connectionManager->mutexMap["unlock-response"]);
        while (connectionManager->algorithm.getResponsesSentByMeCounter() > 0) {
            connectionManager->cvMap["receivedAllUnlocks"].wait(lock2);
        }
        lock2.unlock();

        // NOW IN CRITICAL SECTION
        std::stringstream str;
        str << "---CRITICAL SECTION : START--- ("  << lastSentLockClock << ")";
        connectionManager->log(str.str());
        stateMutex.lock();
        connectionManager->algorithm.changeState(RicardAgravala::State::IN_CRITICAL_SECTION);
        RicardAgravala::myRequest clear;
        connectionManager->algorithm.setMyNotFulfilledRequest(clear);
        connectionManager->algorithm.updateLamportClock();      // for readable logs
        stateMutex.unlock();
    }

    void d_unlock() {
        // SEND MESSAGE WITH CHANGED DATA AND LEAVE CRITICAL SECTION
        connectionManager->log("---CRITICAL SECTION : EXIT ---");

        // send unlock messages with updated data
        int lastSentLockClock = connectionManager->sendUnLockMessages(protectedData);
        // CRITICAL SECTION ENTRY
        // todo global-lock -> response-lock
        std::unique_lock<std::mutex> lock(connectionManager->mutexMap["global-lock"]);
        while (connectionManager->algorithm.getNotAnsweredRepliesCount(lastSentLockClock) > 0) {
            std::stringstream str;
            str << "WAIT for CONFIRMATIONS (" << lastSentLockClock << ")";
            connectionManager->log(str.str());
            connectionManager->cvMap["receivedAllReplies"].wait(lock);
        };

        // send responses from requestsFromOthersQueue:
        stateMutex.lock();
        std::stringstream str;
        str << "--- GOT ALL CONFIRMATIONS ("  << lastSentLockClock << ") ---";
        connectionManager->log(str.str());
        connectionManager->freeRequests();
        connectionManager->algorithm.changeState(RicardAgravala::State::FREE);
        stateMutex.unlock();
    }

    void setDataToSynchronize(std::string newData) {
        protectedData = newData;
    }

    void addSenderToWaitThreadList(int localId, int distributedId, int originalRequestClock, int waitMessageClock) {
        waitingThreadsVectorMutex.lock();
        waitingThreadsVector.push_back(WaitInfo(localId, distributedId, originalRequestClock, waitMessageClock));
        waitingThreadsVectorMutex.unlock();
    }

    void removeThisThreadFromWaitingList(int localId, int distributedId) {
        for (unsigned int i = 0; i < waitingThreadsVector.size(); i++) {
            if (waitingThreadsVector.at(i).localId == localId && waitingThreadsVector.at(i).distributedId == distributedId) {
                waitingThreadsVector.erase(waitingThreadsVector.begin() + i);
            }
        }
    }

    std::mutex* getLocalMutex() {
        return &l_mutex;
    }

    void setLastRequestThatWeResponsedClock(int lastSentResponseClock) {
        this->lastRequestThatWeResponsedClock = std::max(this->lastRequestThatWeResponsedClock, lastSentResponseClock);
    }

    int getLastRequestThatWeResponsedClock() const {
        return lastRequestThatWeResponsedClock;
    }

};

#endif //NPR_MONITOR_MUTEX_H
