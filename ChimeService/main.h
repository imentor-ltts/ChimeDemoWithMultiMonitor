#pragma once
#include <iostream>
#include <fstream>
#include <ctime>
#include <sstream>
#include "Win32Interop.h"
#define _CRT_SECURE_NO_WARNINGS

class Conversion {
public:

	// ________________________________________________
	//
	// ConvertToLPCWSTR
	//
	// PURPOSE: 
	// Convert string to LPCWSTR for low level API calls.
	//
	// RETURN VALUE:
	// A valid LPCWSTR converted string.
	// ________________________________________________
	//
	static LPCWSTR ConvertToLPCWSTR(const std::string& str)
	{
		// Calculate the required buffer size for the wide string
		int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);

		// Allocate a buffer for the wide string
		wchar_t* buffer = new wchar_t[size_needed];

		// Convert the narrow string to wide string
		MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, buffer, size_needed);

		// Return the wide string as LPCWSTR
		return buffer;
	}

	// ________________________________________________
	//
	// GetAbsoluteCoordinate
	//
	// PURPOSE: 
	// Convert pixel coordinate to absolute coordinate (0-65535).
	//
	// RETURN VALUE:
	// Absolute Coordinate
	// ________________________________________________
	//
	static INT GetAbsoluteCoordinate(INT PixelCoordinate, INT ScreenResolution)
	{
		INT AbsoluteCoordinate = MulDiv(PixelCoordinate, 65535, ScreenResolution);
		return AbsoluteCoordinate;
	}


	// ________________________________________________
	//
	// GetAbsoluteCoordinates
	//
	// PURPOSE: 
	// Convert the Coordinates of the Mouse to Absolute Coordinates.
	//
	// RETURN VALUE:
	// Absolute Coordinate
	// ________________________________________________
	//
	static void GetAbsoluteCoordinates(INT32& X, INT32& Y)
	{
		HMONITOR hMonitor = MonitorFromWindow(GetDesktopWindow(), MONITOR_DEFAULTTOPRIMARY);
		// Get multi-screen coordinates
		MONITORINFO MonitorInfo = { 0 };
		MonitorInfo.cbSize = sizeof(MonitorInfo);
		if (GetMonitorInfoW(hMonitor, &MonitorInfo))
		{
			// 1) Get pixel coordinates of topleft pixel of target screen, relative to the virtual desktop ( coordinates should be 0,0 on Main screen);
			// 2) Get pixel coordinates of mouse cursor, relative to the target screen;
			// 3) Sum topleft margin pixel coordinates with mouse cursor coordinates;
			X = MonitorInfo.rcMonitor.left + X;
			Y = MonitorInfo.rcMonitor.top + Y;

			// 4) Transform the resulting pixel coordinates into absolute coordinates.
			X = GetAbsoluteCoordinate(X, GetSystemMetrics(SM_CXSCREEN));
			Y = GetAbsoluteCoordinate(Y, GetSystemMetrics(SM_CYSCREEN));
		}
	}

	// ________________________________________________
	//
	// GetLastErrorAsString
	//
	// PURPOSE: 
	// Convert the Error Code generated to a string.
	//
	// RETURN VALUE:
	// Human Readable Error message
	// ________________________________________________
	//
	static std::string GetLastErrorAsString(DWORD errorCode) {
		// FormatMessage flags
		DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;

		// Get the system error message for the given error code
		LPWSTR buffer = nullptr;
		DWORD size = FormatMessage(flags, nullptr, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&buffer, 0, nullptr);

		// Copy the error message from the buffer to a wstring
		std::wstring message(buffer, size);

		// Free the buffer
		LocalFree(buffer);

		std::string errorMessage(message.begin(), message.end());
		return errorMessage;
	}

};
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////GLOBAL HELPER FUNCTIONS CREATED FOR TESTABILITY AND MODULARITY///////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string GetCurrentDate() {
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

int logToFile(const std::string& message) {
	// Construct the filename based on the current date
	std::string filename = "C:\\ProgramData\\log_" + GetCurrentDate() + ".txt";

	// Open the file in append mode
	std::ofstream logfile(filename, std::ios_base::app);

	if (!logfile.is_open()) {
		std::cerr << "Error: Unable to open log file." << std::endl;
		return 1;
	}

	// Get the current time
	time_t now = time(0);
	// Convert it to a string
	std::string timestamp = ctime(&now);
	// Remove the newline character from the end
	timestamp.pop_back();

	// Log entry
	std::string logEntry = "[" + timestamp + "]" + message;

	// Write the log entry to the file
	logfile << logEntry << std::endl;
	// Close the file
	logfile.close();

	return 0;
}
//// Function to log messages to a file
//void logToFile(const std::string& message) {
//	// Get current time
//	std::time_t now = std::time(nullptr);
//	std::tm* timeinfo = std::localtime(&now);
//
//	// Format time as string
//	char timeString[20];
//	std::strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", timeinfo);
//	char fileName[100];
//	sprintf(fileName, "C:\\ProgramData\\log-%s.txt", timeString);
//	// Open file in append mode
//	std::ofstream logfile(fileName, std::ios_base::app);
//	if (logfile.is_open()) {
//		// Write formatted log entry to file
//		logfile << "[" << timeString << "] " << message << std::endl;
//		logfile.close();
//	}
//	else {
//		std::cerr << "Error opening log file!" << std::endl;
//	}
//}
void StopService(std::string serviceName) 
{
	SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
	if (scm == NULL)
	{
		logToFile("Failed to open Service Control Manager\n");
		return;
	}
	auto _serviceName = Conversion::ConvertToLPCWSTR(serviceName);
	SC_HANDLE service = OpenService(scm, _serviceName, SERVICE_STOP | SERVICE_QUERY_STATUS);
	if (service == NULL)
	{
		logToFile("Failed to open service\n");
		CloseServiceHandle(scm);
		return;
	}

	SERVICE_STATUS status;
	if (!ControlService(service, SERVICE_CONTROL_STOP, &status))
	{
		std::cout << "Failed to stop the service" << GetLastError() << std::endl;
		logToFile("Failed to stop service\n");
		CloseServiceHandle(service);
		CloseServiceHandle(scm);
		return;
	}

	std::cout << "Service stopped successfully\n";
	logToFile("Service stopped successfully\n");
	CloseServiceHandle(service);
	CloseServiceHandle(scm);
}



/// <summary>
/// Helper Function to capture the screen
/// </summary>
/// <returns>A Handle to Bitmap object that contains the captured screen shot</returns>
HBITMAP CaptureScreen() {
	//inside capture screen in the begininning
	bool success = Win32Interop::SwitchToInputDesktop();
	if (success) {
		logToFile("Successfully switched the input desktop");
	}
	else {
		logToFile("Failed in the SwitchToInputDesktop Func");
	}
	int screen_width = GetSystemMetrics(SM_CXSCREEN);
	int screen_height = GetSystemMetrics(SM_CYSCREEN);
	/*  std::cout << "--------------------------------------------------" << std::endl;
	  std::cout << "Width of Screen: " << screen_width << std::endl;
	  std::cout << "Height of Screen: " << screen_height << std::endl;
	  std::cout << "--------------------------------------------------" << std::endl;*/
	HDC hScreenDC = GetDC(NULL);
	if (!hScreenDC) {
		// Handle error, possibly log or throw an exception
		std::cout << "Error in getting screen DC" << std::endl;
		logToFile("Error in getting Screen DC");
		return NULL;
	}

	HDC hMemoryDC = CreateCompatibleDC(hScreenDC);
	if (!hMemoryDC) {
		// Handle error
		std::cout << "Error in creating memory DC" << std::endl;
		logToFile("Error in getting memory DC");
		ReleaseDC(NULL, hScreenDC);
		return NULL;
	}

	HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, screen_width, screen_height);
	if (!hBitmap) {
		// Handle error
		std::cout << "Error in creating bitmap" << std::endl;
		logToFile("Error in creating the Bitmap");
		ReleaseDC(NULL, hScreenDC);
		DeleteDC(hMemoryDC);
		return NULL;
	}

	HGDIOBJ hOldBitmap = SelectObject(hMemoryDC, hBitmap);
	if (!hOldBitmap) {
		std::cout << "Error in selecting bitmap" << std::endl;
		logToFile("Error in selecting bitmap");
		// Handle error
		DeleteObject(hBitmap);
		ReleaseDC(NULL, hScreenDC);
		DeleteDC(hMemoryDC);
		return NULL;
	}
	//bit-block-transfer
	if (!BitBlt(hMemoryDC, 0, 0, screen_width, screen_height, hScreenDC, 0, 0, SRCCOPY)) {

		// Handle BitBlt failure
		std::cout << "Error in BitBlt" << std::endl;
		logToFile("Error in BitBlt");
		DeleteObject(hBitmap);
		ReleaseDC(NULL, hScreenDC);
		DeleteDC(hMemoryDC);
		return NULL;
	}

	SelectObject(hMemoryDC, hOldBitmap); // Restore the original bitmap

	ReleaseDC(NULL, hScreenDC);
	DeleteDC(hMemoryDC);

	return hBitmap;
}





std::vector<uint8_t> GetBitmapData(HBITMAP hBitmap, BITMAPINFO& bmpInfo) {
	if (!hBitmap) {

		// Handle null bitmap error
		std::cout << "Error in getting bitmap data" << std::endl;
		logToFile("Error in getting bitmap data");
		return {};
	}

	HDC hDC = GetDC(NULL);
	HDC hMemDC = CreateCompatibleDC(hDC);

	BITMAP bmp;
	if (!GetObject(hBitmap, sizeof(BITMAP), &bmp)) {

		// Handle GetObject failure
		std::cout << "Error in getting object" << std::endl;
		logToFile("Error in getting object");
		ReleaseDC(NULL, hDC);
		DeleteDC(hMemDC);
		return {};
	}

	bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmpInfo.bmiHeader.biWidth = bmp.bmWidth;
	bmpInfo.bmiHeader.biHeight = -bmp.bmHeight; // Negative to flip the image
	bmpInfo.bmiHeader.biPlanes = 1;
	bmpInfo.bmiHeader.biBitCount = 32;
	bmpInfo.bmiHeader.biCompression = BI_RGB;

	std::vector<uint8_t> bitmapData(bmp.bmWidth * bmp.bmHeight * 4);

	if (!GetDIBits(hMemDC, hBitmap, 0, bmp.bmHeight, bitmapData.data(), &bmpInfo, DIB_RGB_COLORS)) {

		// Handle GetDIBits failure
		std::cout << "Error in getting DIBits" << std::endl;
		logToFile("Error in getting DIBits");
		ReleaseDC(NULL, hDC);
		DeleteDC(hMemDC);
		return {};
	}

	ReleaseDC(NULL, hDC);
	DeleteDC(hMemDC);

	return bitmapData;
}

