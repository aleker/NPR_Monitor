#ifndef NPR_MONITOR_MUTEX_H
#define NPR_MONITOR_MUTEX_H

#include "../DistributedMonitor.h"

class DistributedMutex {
private:
    DistributedMonitor* distributedMonitor;
    std::mutex& mainMutex;
    std::mutex mt;
    std::condition_variable& cv;
    std::unique_lock<std::mutex> lk;

public:
    explicit DistributedMutex(DistributedMonitor* distributedMonitor) :
            distributedMonitor(distributedMonitor),
            mainMutex(distributedMonitor->getMutex()),
            cv(distributedMonitor->getCv()) {
        // std::lock_guard<std::mutex> guard(mainMutex);
        // lk = std::unique_lock<std::mutex>(mainMutex);
        distributedMonitor->d_lock();
        while (!distributedMonitor->checkIfGotAllReplies()) {
            std::unique_lock<std::mutex> lk(mainMutex);
            std::cout << "WAIT\n";
            cv.wait(lk);
        };
        std::cout << "WAIT END\n";
        // NOW IN CRITICAL SECTION
        distributedMonitor->changeState(DistributedMonitor::State::IN_CRITICAL_SECTION);
    }

    ~DistributedMutex() {
        lk.unlock();
        // cv.notify_one();
    }
};

#endif //NPR_MONITOR_MUTEX_H
