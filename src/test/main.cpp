#include <iostream>

#include "TestSharingValues.h"
#include "../MultiprocessDebugHelper.h"
#include "../connection/MPI_Connection.h"

int main(int argc, char *argv[]) {
    auto connection = std::make_shared<MPI_Connection>(argc, argv, 1);
    // auto connectionP = std::make_shared<MPI_Connection>(argc, argv, 2);
    // TESTING:
    // MultiprocessDebugHelper::setup(15000 + connection->getId());

    TestSharingValues test(std::move(connection), "AAA");

    std::chrono::seconds sec(1);
    std::this_thread::sleep_for(sec);
    //TestSharingValues test2(std::move(connectionP), "PPP");


    // TEST MULTITHREADING
    int loopsCount = 4;
    while(loopsCount > 0) {
        test.increment();
        //test2.increment();
        sec = std::chrono::seconds(3);
        std::this_thread::sleep_for(sec);
        loopsCount--;
        test.printProtectedValues();
        //test2.printProtectedValues();
    }

    return 0;
}