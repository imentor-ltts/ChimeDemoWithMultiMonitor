#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>

#define _CRT_SECURE_NO_WARNINGS

// Function to log messages to a file
void logToFile(const std::string& message) {
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


int _tmain(int argc, TCHAR* argv[])
{
    HANDLE hToken;
    HANDLE hTokenDuplicate;
    HANDLE hProcess;
    DWORD dwSessionId = 0; // Session ID of the user
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    BOOL bResult = FALSE;

    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si);
    si.wShowWindow = TRUE;
//    si.lpDesktop = L"winsta0\\default";

    // Get the primary token of the user
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken))
    {
        logToFile("OpenProcessToken failed : " + GetLastError());
        printf("OpenProcessToken failed: %d\n", GetLastError());
        return 1;
    }

    // Duplicate the primary token for impersonation
    if (!DuplicateTokenEx(hToken, TOKEN_ALL_ACCESS, NULL, SecurityImpersonation, TokenPrimary, &hTokenDuplicate))
    {
        printf("DuplicateTokenEx failed: %d\n", GetLastError());
        logToFile("DuplicateTokenEx failed: " + GetLastError());
        CloseHandle(hToken);
        return 1;
    }

    // Close the primary token handle
    CloseHandle(hToken);

    // Specify the session ID of the user
    // This should be obtained from the user session
    // For simplicity, we hardcode it to 0
    dwSessionId = 0;

    // Impersonate the logged-on user
    if (!SetTokenInformation(hTokenDuplicate, TokenSessionId, &dwSessionId, sizeof(DWORD)))
    {
        printf("SetTokenInformation failed: %d\n", GetLastError());
        logToFile("SetTokenInformation failed:  " + GetLastError());
       // CloseHandle(hTokenDuplicate);
        //return 1;
    }

    // Create the process as the user
    bResult = CreateProcessAsUser(hTokenDuplicate,
        _T("I:\\Chime\\demo.exe"), // Replace with the path to the desired executable
        NULL,
        NULL,
        NULL,
        FALSE,
        0,
        NULL,
        NULL,
        &si,
        &pi);

    if (!bResult)
    {
        logToFile("CreateProcessAsUser failed: " + GetLastError());
        printf("CreateProcessAsUser failed: %d\n", GetLastError());
        CloseHandle(hTokenDuplicate);
        return 1;
    }
    logToFile("New Process has started");
    WaitForSingleObject(pi.hProcess, INFINITE);
    // Close handles
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(hTokenDuplicate);
    logToFile("Application exiting with Exit code: 0");
    return 0;
}
