#include <iostream>
#include <chrono>

#include "TestBuffer.h"
#include "../MultiprocessDebugHelper.h"
#include "../connection/MPI_Connection.h"
#include "ConsumerProducerQueue.h"

std::shared_ptr<ConsumerProducerQueue> buffer;
std::shared_ptr<ConsumerProducerQueue> buffer2;

int main(int argc, char *argv[]) {
    MPI_Connection connection(argc, argv, 4);
    // TESTING:
    // MultiprocessDebugHelper::setup(15000 + connection.getDistributedClientId());

    buffer = std::make_shared<ConsumerProducerQueue>(&connection, 5);
    buffer2 = std::make_shared<ConsumerProducerQueue>(&connection, 10);

    if (buffer->isProducent()) {
        for (int i = 0; i < 6; i++) {
            buffer->produce(i);
            buffer2->produce(i);
        }
    }
    else {
        for (int i = 0; i < 12; i++) {
            std::chrono::seconds sec = std::chrono::seconds(3);
            std::this_thread::sleep_for(sec);
            buffer->consume();
            sec = std::chrono::seconds(2);
            std::this_thread::sleep_for(sec);
            buffer2->consume();
        }
    }

    std::cout << "END\n";

    std::chrono::seconds sec = std::chrono::seconds(6);
    std::this_thread::sleep_for(sec);

    std::cout << "END2\n";


    return 0;
}