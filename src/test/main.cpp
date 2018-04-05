#include <iostream>
#include <zconf.h>
#include "Test.h"
#include "../connection/MPI_Connection.h"


int main(int argc, char *argv[]) {
    MPI_Connection connection(argc, argv);
    Test test(&connection);

    int loopsCount = 4;
    while(loopsCount > 0) {
        test.increment();
        sleep(2);
        std::cout << "Id: " << connection.getId() << ", test= " << test.getConnectionId() << std::endl;
        loopsCount--;
    }

    return 0;
}