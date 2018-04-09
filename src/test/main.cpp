#include <iostream>

#include "TestSharingValues.h"
#include "../MultiprocessDebugHelper.h"
#include "../connection/MPI_Connection.h"

int main(int argc, char *argv[]) {
    auto connection = std::make_unique<MPI_Connection>(argc, argv);
    TestSharingValues test(std::move(connection));
    TestSharingValues test2(std::move(connection));

    // TESTING:
    // MultiprocessDebugHelper::setup(15000 + connection->getId());

    // TEST MULTITHREADING
    int loopsCount = 4;
    while(loopsCount > 0) {
        test.increment();
        std::chrono::seconds sec(2);
        std::this_thread::sleep_for(sec);
        loopsCount--;
        test.printProtectedValues();
    }

    return 0;
}