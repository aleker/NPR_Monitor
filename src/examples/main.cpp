#include <iostream>
#include <chrono>

#include "TestBuffer.h"
#include "../MultiprocessDebugHelper.h"
#include "../connection/MPI_Connection.h"
#include "ConsumerProducerQueue.h"


int main(int argc, char *argv[]) {
    MPI_Connection connection(argc, argv, 4);
    // TESTING:
    // MultiprocessDebugHelper::setup(15000 + connection.getDistributedClientId());

    ConsumerProducerQueue buffer(&connection, 5);

    if (buffer.isProducent()) {
        for (int i = 0; i < 1; i++) {
            buffer.produce(i);
            buffer.produce(i);
        }
    }
    else {
        for (int i = 0; i < 2; i++) {
            std::chrono::seconds sec = std::chrono::seconds(3);
            std::this_thread::sleep_for(sec);
            buffer.consume();
            sec = std::chrono::seconds(2);
            std::this_thread::sleep_for(sec);
        }
    }

    buffer.endCommunication();

    return 0;
}