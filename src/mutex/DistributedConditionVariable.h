#ifndef NPR_MONITOR_DISTRIBUTEDCONDITIONVARIABLE_H
#define NPR_MONITOR_DISTRIBUTEDCONDITIONVARIABLE_H

#include <mutex>
#include <condition_variable>
#include <memory>
#include <utility>
#include "DistributedMutex.h"
#include "../connection/ConnectionManager.h"

class DistributedConditionVariable {
private:
    std::string id_name;
    std::shared_ptr<DistributedMutex> d_mtx;
    std::shared_ptr<ConnectionManager> connectionManager;
    std::condition_variable l_cond;

public:
    DistributedConditionVariable(std::string id_name, std::shared_ptr<DistributedMutex> d_mtx) :
            id_name(std::move(id_name)), d_mtx(std::move(d_mtx)) {
        this->connectionManager = this->d_mtx->connectionManager;
    }

    void d_wait() {
        // SEND MESSAGE WITH CHANGED DATA AND LEAVE CRITICAL SECTION
        connectionManager->log("---CRITICAL SECTION : EXIT (WAIT) ---");

        // send unlock messages with wait info
        connectionManager->sendUnLockAndWaitMessages();

        // send responses from requestsFromOthersQueue:
        d_mtx->stateMutex.lock();
        connectionManager->freeRequests();
        connectionManager->algorithm.changeState(RicardAgravala::State::FREE);
        d_mtx->stateMutex.unlock();

        // wait here!
        std::unique_lock<std::mutex> lock(*d_mtx->getLocalMutex());
        l_cond.wait(lock);
        d_mtx->d_lock(d_mtx->getLastLockClock());
    }

    void d_notifyAll() {
        connectionManager->log("---NOTIFY ---");
        d_mtx->waitingThreadsVectorMutex.lock();
        while (!d_mtx->waitingThreadsVector.empty()) {
            DistributedMutex::WaitInfo wait = d_mtx->waitingThreadsVector.back();
            d_mtx->waitingThreadsVector.pop_back();
            std::shared_ptr<Message> msg = std::make_shared<Message>
                    (connectionManager->getDistributedClientId(), connectionManager->getLocalClientId(), Message::MessageType::SIGNAL, wait.waitMessageClock, id_name);
            msg->setReceiversId(wait.distributedId, wait.localId);
            connectionManager->sendSingleMessage(msg, false);
        }
        d_mtx->waitingThreadsVectorMutex.unlock();

        // notify locally!
        l_notify();
    }

    void l_notify() {
        std::unique_lock<std::mutex> lock(*d_mtx->getLocalMutex());
        lock.unlock();
        l_cond.notify_all();
    }

    std::string getIdName() {
        return id_name;
    }
};

#endif //NPR_MONITOR_DISTRIBUTEDCONDITIONVARIABLE_H
