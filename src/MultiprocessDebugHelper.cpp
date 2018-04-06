#include <iostream>
#include <chrono>
#include <thread>
#include <sstream>
#include <unistd.h>
#include <csignal>
#include "MultiprocessDebugHelper.h"

int MultiprocessDebugHelper::gdbServerPid = 0;

std::string execAndCapturePid(const char* command) {
    const int bufferSize = 21; // PID surely won't be bigger than MAX_U_INT64 which is 20 digits long (+ '\0')
    std::array<char, bufferSize> buffer{0};
    std::string result;
    std::shared_ptr<FILE> pipe(popen(command, "r"), pclose);
    if (not pipe) {
        throw std::runtime_error("popen() failed!");
    }
    if (fgets(buffer.data(), bufferSize, pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

int MultiprocessDebugHelper::setup(int port) {
    signal(SIGINT, cleanup);
    signal(SIGABRT, cleanup);
    signal(SIGTERM, cleanup);
    signal(SIGTSTP, cleanup);
    std::atexit([]() { cleanup(0); });
    std::stringstream gdbServerCommand;
    gdbServerCommand << "( gdbserver --attach localhost:" << port << " " << getpid() << " & echo $! )";
    MultiprocessDebugHelper::gdbServerPid = std::stoi(execAndCapturePid(gdbServerCommand.str().c_str()));

    std::this_thread::sleep_for(std::chrono::seconds(1));

    return gdbServerPid;
}

void MultiprocessDebugHelper::cleanup(int i) {
//    kill(gdbServerPid, SIGKILL);
    system("killall gdbserver");
}
