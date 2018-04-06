#include <iostream>
#include <zconf.h>
#include <thread>
#include "Test.h"
#include "../connection/MPI_Connection.h"
#include "../MultiprocessDebugHelper.h"


int main(int argc, char *argv[]) {
    auto connection = std::make_unique<MPI_Connection>(argc, argv);
//    MultiprocessDebugHelper::setup(15000 + connection->getId());
//    MPI_Connection* connection = new MPI_Connection(argc, argv);
    Test test(std::move(connection));

    int loopsCount = 3;
    while(loopsCount > 0) {
        test.increment();
        std::this_thread::sleep_for(std::chrono::seconds(2));
        std::cout << "Id: " << test.getConnectionId() << ", test= " << test.getProtectedValue() << "\n";
        loopsCount--;
    }

    return 0;
}