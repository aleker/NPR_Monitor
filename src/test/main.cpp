#include <iostream>

#include "Buffer.h"
#include "../MultiprocessDebugHelper.h"
#include "../connection/MPI_Connection.h"

int main(int argc, char *argv[]) {
    auto connection = std::make_shared<MPI_Connection>(argc, argv, 4);
    // auto connectionP = std::make_shared<MPI_Connection>(argc, argv, 2);
    // TESTING:
    // MultiprocessDebugHelper::setup(15000 + connection->getClientId());

    Buffer test(std::move(connection));

    std::chrono::seconds sec(1);
    std::this_thread::sleep_for(sec);
    //Buffer test2(std::move(connectionP), "PPP");


    // TEST MULTITHREADING
    int loopsCount = 4;
    while(loopsCount > 0) {
        test.increment();
        //test2.increment();
        sec = std::chrono::seconds(3);
        //std::this_thread::sleep_for(sec);
        loopsCount--;
        test.printProtectedValues();
        //test2.printProtectedValues();
    }

    return 0;
}