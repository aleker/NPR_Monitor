#include <iostream>

#include "Buffer.h"
#include "../MultiprocessDebugHelper.h"
#include "../connection/MPI_Connection.h"

int main(int argc, char *argv[]) {
    auto connection = std::make_shared<MPI_Connection>(argc, argv, 4);
    // auto connectionP = std::make_shared<MPI_Connection>(argc, argv, 2);
    // TESTING:
    // MultiprocessDebugHelper::setup(15000 + connection->getDistributedClientId());

    Buffer test(std::move(connection));

    std::chrono::seconds sec(1);
    std::this_thread::sleep_for(sec);
    //Buffer test2(std::move(connectionP), "PPP");


    // TEST MULTITHREADING
    int loopsCount = 10;
    while(loopsCount > 0) {
        test.increment();
        loopsCount--;
    }

    sec = std::chrono::seconds(6);
    std::this_thread::sleep_for(sec);
    test.printProtectedValues();

    return 0;
}