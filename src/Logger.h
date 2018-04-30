#ifndef NPR_MONITOR_LOGGER_H
#define NPR_MONITOR_LOGGER_H

#include <string>
#include <mutex>
#include <fstream>

class Logger {
private:
    std::ofstream log_file;
    std::mutex logMtx;

    void setupLogFile(std::string fileName) {
        // log_file.open(fileName, std::ios_base::out | std::ios_base::app );
    }

public:
    static bool showSystemLogs;

    Logger() : Logger("systemLog.txt") { }

    Logger(std::string fileName) {
        setupLogFile(fileName);
    }

    ~Logger() {
        log_file.close();
    }

    void printLog(std::string log) {
        std::lock_guard<std::mutex> lock(logMtx);
        std::cout << log << "\n";
        log_file << log << "\n";
    }

    // TODO additional method for user logs
    void systemLog(std::string log) {
        if (!Logger::showSystemLogs) return;
        printLog(log);
    }

    void log(std::string log) {
        printLog(log);
    }

};
#endif //NPR_MONITOR_LOGGER_H
