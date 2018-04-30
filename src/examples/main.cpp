#include <iostream>
#include <chrono>

#include "TestBuffer.h"
#include "../MultiprocessDebugHelper.h"
#include "../connection/MPI_Connection.h"
#include "ConsumerProducerQueue.h"

void test1(int argc, char *argv[]) {
    std::shared_ptr<ConnectionInterface>connection = std::make_shared<MPI_Connection>(argc, argv, 4);
    std::shared_ptr<ConnectionInterface>connectionP = std::make_shared<MPI_Connection>(argc, argv, 2);
    // TESTING:
    // MultiprocessDebugHelper::setup(15000 + connection.getDistributedClientId());

    TestBuffer test(connection);
    TestBuffer test2(connection);
    TestBuffer test3(connectionP);
    TestBuffer test4(connection);

    // TEST MULTITHREADING
    int loopsCount = 2000;
    while(loopsCount > 0) {
        test.increment();
        test2.increment();
        test3.increment();

        loopsCount--;
    }

    std::cout << "END\n";
    std::chrono::seconds sec = std::chrono::seconds(6);
    std::this_thread::sleep_for(sec);
    test.printProtectedValues();

    sec = std::chrono::seconds(1);
    std::this_thread::sleep_for(sec);
    test2.printProtectedValues();

    sec = std::chrono::seconds(1);
    std::this_thread::sleep_for(sec);
    test3.printProtectedValues();

    std::thread endThread(&TestBuffer::endCommunication, &test);
    std::thread endThread2(&TestBuffer::endCommunication, &test2);
    std::thread endThread3(&TestBuffer::endCommunication, &test3);
    std::thread endThread4(&TestBuffer::endCommunication, &test4);
    endThread.join();
    endThread2.join();
    endThread3.join();
    endThread4.join();
}

void test2(int argc, char *argv[]) {
    std::shared_ptr<ConnectionInterface>connection = std::make_shared<MPI_Connection>(argc, argv, 4);
    // TESTING:
    // MultiprocessDebugHelper::setup(15000 + connection->getDistributedClientId());

    ConsumerProducerQueue producer(connection, 5);
    ConsumerProducerQueue consumer(connection, 5);
    ConsumerProducerQueue consumer2(connection, 5);

    int howManyProduce = 200;
    if (producer.isProducer()) {
        for (int i = 0; i < howManyProduce; i++) {
            producer.produce(i);
        }
    }
    else {
        for (int i = 0; i < howManyProduce/2; i++) {
            // consume with delay to test WAIT/NOTIFY
            std::chrono::seconds sec = std::chrono::seconds(3);
            std::this_thread::sleep_for(sec);
            consumer.consume();
            consumer2.consume();
        }
    }
    std::thread endThread(&TestBuffer::endCommunication, &producer);
    std::thread endThread2(&TestBuffer::endCommunication, &consumer);
    std::thread endThread3(&TestBuffer::endCommunication, &consumer2);
    endThread.join();
    endThread2.join();
    endThread3.join();
}

int main(int argc, char *argv[]) {
    //test1(argc, argv);
    test2(argc, argv);

    MPI_Connection::endConnection();
    return 0;
}