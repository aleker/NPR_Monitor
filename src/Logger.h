//
// Created by ola on 27.04.18.
//

#ifndef NPR_MONITOR_LOGGER_H
#define NPR_MONITOR_LOGGER_H

#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/tee.hpp>

#include <string>
#include <mutex>
#include <iostream>
#include <fstream>

class Logger {
private:
    std::mutex logMtx;

    void setupLogFile(std::string fileName) {
        // fclose(fopen(fileName.c_str(), "w"));
        // freopen(fileName.c_str(), "a+", stdout);
    }

public:
    Logger() {
        setupLogFile("log.txt");   // /home/ola/CLionProjects/NPR_Monitor/src/
    }

    Logger(std::string fileName) {
        setupLogFile(fileName);
    }

    ~Logger() {}

    void log(std::string log) {
        std::lock_guard<std::mutex> lock(logMtx);
        std::cout << log << "\n";
    }

};

#endif //NPR_MONITOR_LOGGER_H
