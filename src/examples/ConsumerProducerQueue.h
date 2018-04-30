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
        print_queue(bufferQueue, "AAA wysylam");
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
        print_queue(bufferQueue, "AAA przedOdeb");
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
        print_queue(bufferQueue, "AAA odebrane");
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
        request += getDistributedId();
        d_mutex->d_lock();
        while (isFull())
            d_cvMap["id1"]->d_wait();
        bufferQueue.push(request);

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
