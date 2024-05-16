#include <Windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>

#define _CRT_SECURE_NO_WARNINGS

// Function to log messages to a file
void logToFile(const std::string & message) {
    // Get current time
    std::time_t now = std::time(nullptr);
    std::tm* timeinfo = std::localtime(&now);

    // Format time as string
    char timeString[20];
    std::strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", timeinfo);

    // Open file in append mode
    std::ofstream logfile("C:\\ProgramData\\log.txt", std::ios_base::app);
    if (logfile.is_open()) {
        // Write formatted log entry to file
        logfile << "[" << timeString << "] " << message << std::endl;
        logfile.close();
    }
    else {
        std::cerr << "Error opening log file!" << std::endl;
    }
}
int CreateNewProcess(LPCWSTR lpAppName) 
{
    // Additional command line arguments
    LPWSTR lpCommandLine = nullptr;
    logToFile("Starting the App");
    // Process information
    PROCESS_INFORMATION pi;
    STARTUPINFO si;


    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);

    // Create the process
    if (!CreateProcess(lpAppName, lpCommandLine, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
        logToFile("Failed to create process.Error code : " + GetLastError());
        std::cerr << "Failed to create process. Error code: " << GetLastError() << std::endl;
        return 1;
    }

    // Wait for the process to finish (optional)
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Close process and thread handles
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    logToFile("Exiting the code");
    return 0;
}

int main()
{
    LPCWSTR lpApplicationName = L"I:\\Chime\\demo.exe";
    auto res = CreateNewProcess(lpApplicationName);
    logToFile("Exiting the code" + res);
    return res;
}
