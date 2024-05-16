#include <iostream>
#include <fstream>
#include<sstream>
#include <ctime>

class FileLogger {
private:
    std::ofstream logFile;
public:
    static std::string GetCurrentDate() {
        // Get the current time
        time_t now = time(0);
        // Convert it to tm structure
        tm* timeinfo = localtime(&now);
        // Format the date as YYYYMMDD
        std::stringstream ss;
        ss << (timeinfo->tm_year + 1900)
            << '-' << (timeinfo->tm_mon + 1)
            << '-' << timeinfo->tm_mday;
        return ss.str();
    }

    static std::string GetCurrentDateTime() {
        // Get the current time
        time_t now = time(nullptr);
        // Convert it to a string
        std::string timestamp = ctime(&now);
        // Remove the newline character from the end
        timestamp.pop_back();
        return timestamp;
    }

public:
    FileLogger(const std::string& filename) {
        logFile.open(filename, std::ios::out | std::ios::app);
        if (!logFile.is_open()) {
            std::cerr << "Error: Unable to open log file." << std::endl;
        }
    }

    ~FileLogger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    void Log(const std::string& message) {
        if (logFile.is_open()) {
            std::string timestamp = GetCurrentDateTime();
            logFile << "[" << timestamp << "] " << message << std::endl;
        }
        else {
            std::cerr << "Error: Log file is not open." << std::endl;
        }
    }
};

