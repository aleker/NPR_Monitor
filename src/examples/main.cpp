#include <iostream>
#include <chrono>

#include "TestBuffer.h"
#include "../MultiprocessDebugHelper.h"
#include "../connection/MPI_Connection.h"
#include "ConsumerProducerQueue.h"

void test1(int argc, char *argv[]) {
    std::shared_ptr<ConnectionInterface> connection = std::make_shared<MPI_Connection>(argc, argv, 4);
    std::shared_ptr<ConnectionInterface> connectionP = std::make_shared<MPI_Connection>(argc, argv, 2);

    TestBuffer test(connection);
    TestBuffer test2(connection);
    TestBuffer test3(connectionP);
    TestBuffer test4(connection);

    int loopsCount = 400;
    while (loopsCount > 0) {
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

    test.endCommunication();
    test2.endCommunication();
    test3.endCommunication();
    test4.endCommunication();
}

void test2(int argc, char *argv[]) {
    std::shared_ptr<ConnectionInterface> connection = std::make_shared<MPI_Connection>(argc, argv, 4);

    ConsumerProducerQueue producer(connection, 5);
    ConsumerProducerQueue consumer(connection, 5);
    ConsumerProducerQueue consumer2(connection, 5);

    int howManyProduce = 3000;
    if (producer.isProducer()) {
        for (int i = 0; i < howManyProduce; i++) {
            producer.produce(i);
        }
    } else {
        for (int i = 0; i < howManyProduce / 2; i++) {
            // consume with delay to test WAIT/NOTIFY
            std::chrono::seconds sec = std::chrono::seconds(3);
            std::this_thread::sleep_for(sec);
            consumer.consume();
            consumer2.consume();
        }
    }

    producer.endCommunication();
    consumer.endCommunication();
    consumer2.endCommunication();
}

int main(int argc, char *argv[]) {
    // test1(argc, argv);
    test2(argc, argv);

    MPI_Connection::endConnection();
    return 0;
}