#ifndef NPR_MONITOR_CONSUMERPRODUCERQUEUE_H
#define NPR_MONITOR_CONSUMERPRODUCERQUEUE_H

#include <iostream>
#include "../DistributedMonitor.h"

class ConsumerProducerQueue : public DistributedMonitor {
private:
    std::queue<int> bufferQueue;
    int maxSize;
    bool producer = false;

    std::string returnDataToSend() override {
        if (producer) {
            std::stringstream ss;
            // get last added value
            int value = bufferQueue.back();
            ss << value;
            return ss.str();
        }
        else {
            return "";
        }
    }

    void manageReceivedData(std::string receivedData) override {
        std::stringstream ss;
        ss.str(receivedData);
        std::string value;
        ss >> value;
        if (value == "") {
            assert(!bufferQueue.empty());
            bufferQueue.pop();             // value consumed
        }
        else
            bufferQueue.push(std::stoi(value));        // value produced
    }

public:
    ConsumerProducerQueue(std::shared_ptr<ConnectionInterface>connection, int maxSize) : DistributedMonitor(connection), maxSize(maxSize) {
        d_cvMap["id1"] = std::make_shared<DistributedConditionVariable>("id1", d_mutex);
    }

    ~ConsumerProducerQueue() {
        endCommunication();
    }

    bool isProducer() {
        return (getDistributedId() == 0) ;
    }

    void produce(int request) {
        request += getDistributedId();
        d_mutex->d_lock();
        while (isFull())
            d_cvMap["id1"]->d_wait();
        bufferQueue.push(request);
        std::stringstream str;
        str << "AAAA PRODUCED " << request;
        log(str.str());
        producer = true;

        prepareDataToSend();

        d_mutex->d_unlock();
        d_cvMap["id1"]->d_notifyAll();
    }

    void consume() {
        d_mutex->d_lock();
        while (isEmpty())
            d_cvMap["id1"]->d_wait();
        int request = bufferQueue.front();
        bufferQueue.pop();
        std::stringstream str;
        str << "AAAA CONSUMED " << request;
        log(str.str());
        producer = false;

        prepareDataToSend();

        d_mutex->d_unlock();
        d_cvMap["id1"]->d_notifyAll();
    }

    bool isFull() const {
        return (int)bufferQueue.size() >= maxSize;
    }

    bool isEmpty() const {
        return bufferQueue.empty();
    }

    int length() const {
        return static_cast<int>(bufferQueue.size());
    }

    void clear() {
        d_mutex->d_lock();
        while (!isEmpty()) {
            bufferQueue.pop();
        }
        d_mutex->d_unlock();
        d_cvMap["id1"]->d_notifyAll();
    }
};

#endif //NPR_MONITOR_CONSUMERPRODUCERQUEUE_H
