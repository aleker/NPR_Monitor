//
// Created by ola on 27.04.18.
//

#ifndef NPR_MONITOR_LOGGER_H
#define NPR_MONITOR_LOGGER_H

#include <string>
#include <mutex>
#include <iostream>
#include <fstream>

class Logger {
private:
    std::mutex logMtx;
    std::ofstream myfile;
    std::streambuf *backupBuf, *psbuf;

    void setupLogFile(std::string fileName) {
        myfile.open(fileName);
        backupBuf = std::cout.rdbuf();      //save old buf
        psbuf = myfile.rdbuf();             // get file's streambuf
        std::cout.rdbuf(psbuf);             // assign streambuf to cout
    }

public:
    Logger() {
        // setupLogFile("/home/ola/CLionProjects/NPR_Monitor/src/log.txt");
    }

    Logger(std::string fileName) {
        setupLogFile(fileName);
    }

    ~Logger() {
        std::cout.rdbuf(backupBuf);        // restore cout's original streambuf
        myfile.close();
    }

    void log(std::string log) {
        std::lock_guard<std::mutex> lock(logMtx);
        std::cout << log << "\n";
    }

};

#endif //NPR_MONITOR_LOGGER_H
