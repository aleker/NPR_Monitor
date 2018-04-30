#ifndef NPR_MONITOR_CONSUMERPRODUCERQUEUE_H
#define NPR_MONITOR_CONSUMERPRODUCERQUEUE_H

#include <iostream>
#include "../DistributedMonitor.h"

/*
 * Example of DistributedMonitor implementation
 *
 * bufferQueue - SHARED queue
 */
class ConsumerProducerQueue : public DistributedMonitor {
private:
    std::queue<int> bufferQueue;
    int maxSize;
    bool producer = false;

    std::string returnDataToSend() override {
        print_queue(bufferQueue, "AFTER SENDING");
        if (producer) {
            std::stringstream ss;
            // get last added value
            int value = bufferQueue.back();
            ss << value;
            return ss.str();
        } else {
            return "";
        }
    }

    void manageReceivedData(std::string receivedData) override {
        print_queue(bufferQueue, "BEFORE RECEIVING");
        std::stringstream ss;
        ss.str(receivedData);
        std::string value;
        ss >> value;
        if (value == "") {
            assert(!bufferQueue.empty());
            bufferQueue.pop();             // value consumed
        } else
            bufferQueue.push(std::stoi(value));        // value produced
        print_queue(bufferQueue, "BEFORE RECEIVING");
    }

public:
    ConsumerProducerQueue(std::shared_ptr<ConnectionInterface> connection, int maxSize) : DistributedMonitor(
            connection), maxSize(maxSize) {
        /*
         * All new DistributedConditionVariables must be elements of d_cvMap to work properly.
         * Every DistributedConditionVariable has unique name (key in d_cvMap) by which it is identified.
         * Name is the first argument in declaration ("id1" in example below).
         */
        d_cvMap["id1"] = std::make_shared<DistributedConditionVariable>("id1", d_mutex);
    }

    ~ConsumerProducerQueue() {
        /*
         * destruct() - this function must be called in destructor so that connection could end properly
         */
        destruct();
    }

    /*
     * isProducer() - ConsumerProducerQueue with id=0 will be producing.
     * Rest will be consuming.
     * It's only an example of role assignment.
     */
    bool isProducer() {
        return (getDistributedId() == 0);
    }

    void print_queue(std::queue<int> q, std::string logMes) {
        std::stringstream str;
        str << logMes << " '";
        while (!q.empty()) {
            str << q.front() << " ";
            q.pop();
        }
        str << "'";
        log(str.str());
    }

    void produce(int request) {
        /*
         * d_mutex->d_lock() - distributed mutex lock
         */
        d_mutex->d_lock();
        /*
         * CRITICAL SECTION ENTRY
         */
        while (isFull()) {
            /*
             * d_cvMap["id1"]->d_wait() - distributed condition variable wait
             */
            d_cvMap["id1"]->d_wait();
        }
        bufferQueue.push(request);
        producer = true;
        /*
        * prepareDataToSend() - this function must be called in critical section after applying changes
        */
        prepareDataToSend();
        /*
         * d_mutex->d_unlock() - distributed mutex unlock
         */
        d_mutex->d_unlock();
        /*
         * d_cvMap["id1"]->d_notifyAll() - distributed condition variable notifyAll()
         */
        d_cvMap["id1"]->d_notifyAll();
    }

    void consume() {
        d_mutex->d_lock();
        while (isEmpty())
            d_cvMap["id1"]->d_wait();
        bufferQueue.pop();

        producer = false;
        prepareDataToSend();

        d_mutex->d_unlock();
        d_cvMap["id1"]->d_notifyAll();
    }

    bool isFull() const {
        return (int) bufferQueue.size() >= maxSize;
    }

    bool isEmpty() const {
        return bufferQueue.empty();
    }
};

#endif //NPR_MONITOR_CONSUMERPRODUCERQUEUE_H
