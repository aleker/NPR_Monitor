#include <iostream>

#include "TestBuffer.h"
#include "../MultiprocessDebugHelper.h"
#include "../connection/MPI_Connection.h"

int main(int argc, char *argv[]) {
    MPI_Connection connection(argc, argv, 4);
    MPI_Connection connectionP(argc, argv, 2);
    // TESTING:
    // MultiprocessDebugHelper::setup(15000 + connection->getDistributedClientId());

    TestBuffer test(&connection);
    TestBuffer test2(&connectionP);
    TestBuffer test3(&connectionP);

    // TEST MULTITHREADING
    int loopsCount = 400;
    while(loopsCount > 0) {
        test.increment();
        test2.increment();
        test3.increment();
        loopsCount--;
    }

    std::chrono::seconds sec = std::chrono::seconds(6);
    std::this_thread::sleep_for(sec);
    test.printProtectedValues();

    sec = std::chrono::seconds(1);
    std::this_thread::sleep_for(sec);
    test2.printProtectedValues();

    sec = std::chrono::seconds(1);
    std::this_thread::sleep_for(sec);
    test3.printProtectedValues();

    return 0;
}