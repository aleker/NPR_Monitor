#include <iostream>
#include <zconf.h>
#include <thread>
#include <vector>
#include "Test.h"
#include "../connection/MPI_Connection.h"
#include "../MultiprocessDebugHelper.h"

int main(int argc, char *argv[]) {
    auto connection = std::make_unique<MPI_Connection>(argc, argv);
    Test test(std::move(connection));

    // TESTING:
    // MultiprocessDebugHelper::setup(15000 + connection->getId());

    // --TEST MESSAGE SERIALIZATION
    MPI_Msg msg(test.getConnectionId(), 1, MessageType::EMPTY);
    std::string serializedMessage = Message::serializeMessage<MPI_Msg>(msg);
    std::cout << "Id: " << test.getConnectionId() << "Serial: " << serializedMessage << "\n";
    MPI_Msg desMsg;
    desMsg = Message::deserializeMessage<MPI_Msg>(serializedMessage);
    std::string serializedMessage2 = Message::serializeMessage<MPI_Msg>(desMsg);
    std::cout << "Id: " << test.getConnectionId() << "Serial2: " << serializedMessage << "\n";
    // ---------------------------
    
    int loopsCount = 4;
    while(loopsCount > 0) {
        test.updateLamportClock();
        std::this_thread::sleep_for(std::chrono::seconds(2));
        // std::cout << "Id: " << test.getConnectionId() << ", clock= " << test.getLamportClock() << "\n";
        loopsCount--;
    }

    return 0;
}