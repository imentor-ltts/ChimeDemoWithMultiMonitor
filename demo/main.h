// --------------------------------------------------------------------------------------------------------------------
// <copyright file="main.h" company="Siemens Ultrasound">
// Copyright(c) Since 2022 by Siemens Healthineers
// All Rights Reserved.
// No part of this software may be reproduced or transmitted in any
// form or by any means including photocopying or recording without
// written permission of the copyright owner.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------
#pragma once
#include <iostream>
#include <fstream>
#include <ctime>
#include <sstream>
#include "Conversion.h"
#include "FileLogger.h"
#include "Win32InterOp.h"
#include "DisplayInfo.h"
#include "BitmapFileCreator.h"
#define _CRT_SECURE_NO_WARNINGS

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////GLOBAL HELPER FUNCTIONS CREATED FOR TESTABILITY AND MODULARITY///////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//// ________________________________________________
////
//// StopService
////
//// PURPOSE: 
//// Stops the specific service.
////
//// ARG:
//// STRING: The service name which has to be stopped.
//// 
//// RETURN VALUE:
//// BOOL: True if successfully stopped else false and detaild info can be viewed in the log file
//// ________________________________________________
////
//BOOL StopService(std::string serviceName) 
//{
//	SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
//	if (scm == NULL)
//	{
//		FileLogger::LogToFile("Failed to open Service Control Manager\n");
//		return FALSE;
//	}
//	auto _serviceName = Conversion::ConvertToLPCWSTR(serviceName);
//	SC_HANDLE service = OpenService(scm, _serviceName, SERVICE_STOP | SERVICE_QUERY_STATUS);
//	if (service == NULL)
//	{
//		FileLogger::LogToFile("Failed to open service\n");
//		CloseServiceHandle(scm);
//		return FALSE;
//	}
//
//	SERVICE_STATUS status;
//	if (!ControlService(service, SERVICE_CONTROL_STOP, &status))
//	{
//		auto errMessage = Conversion::GetLastErrorAsString(GetLastError());
//		std::cout << "Failed to stop the service" << errMessage << std::endl;
//		FileLogger::LogToFile("Failed to stop service\n");
//		FileLogger::LogToFile(errMessage);
//		CloseServiceHandle(service);
//		CloseServiceHandle(scm);
//		return FALSE;
//	}
//
//	std::cout << "Service stopped successfully\n";
//	FileLogger::LogToFile("Service stopped successfully\n");
//	CloseServiceHandle(service);
//	CloseServiceHandle(scm);
//	return TRUE;
//}

//Gets the Monitor where the Mouse is present...
MONITORINFOEX getMonitorInfo() {
	// Get the cursor position.
	POINT cursorPos;
	GetCursorPos(&cursorPos);

	// Get the monitor that the cursor is pointing to.

	HMONITOR monitor = MonitorFromPoint(cursorPos, MONITOR_DEFAULTTOPRIMARY);

	// Get the monitor's work area.
	MONITORINFOEX monitorInfo;
	monitorInfo.cbSize = sizeof(MONITORINFOEX);
	GetMonitorInfo(monitor, &monitorInfo);
	return monitorInfo;

	wcout << L"The Current Monitor Name: " << monitorInfo.szDevice << endl;
}

// ________________________________________________
//
// CaptureScreen
//
// PURPOSE: 
// Helper Function to capture the screen based on the Monitor Index.
//
// RETURN VALUE:
// HBITMAP: A Handle to Bitmap object that contains the captured screen shot
// ________________________________________________
HBITMAP CaptureScreen(int index) {
	bool success = Win32Interop::SwitchToInputDesktop();
	
	DisplayInformation info;
	info.EnumerateAndSelectMonitor(index);
	//auto selectedMonitorRect = getMonitorInfo().rcMonitor;
	
	// Calculate monitor dimensions
	int monitorWidth = selectedMonitorRect.right - selectedMonitorRect.left;
	int monitorHeight = selectedMonitorRect.bottom - selectedMonitorRect.top;

	char strText[100];
	sprintf(strText, "The Screen Width and Height: (%d, %d)", monitorWidth, monitorHeight);
	FileLogger::LogToFile(strText);
	std::cout << strText << std::endl;
	HDC hScreenDC = GetDC(NULL);
	if (!hScreenDC) {
		// Handle error, possibly log or throw an exception
		std::cout << "Error in getting screen DC" << std::endl;
		FileLogger::LogToFile("Error in getting Screen DC");
		return NULL;
	}

	HDC hMemoryDC = CreateCompatibleDC(hScreenDC);
	if (!hMemoryDC) {
		// Handle error
		std::cout << "Error in creating memory DC" << std::endl;
		FileLogger::LogToFile("Error in getting memory DC");
		ReleaseDC(NULL, hScreenDC);
		return NULL;
	}

	HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, monitorWidth, monitorHeight);
	if (!hBitmap) {
		// Handle error
		std::cout << "Error in creating bitmap" << std::endl;
		FileLogger::LogToFile("Error in creating the Bitmap");
		ReleaseDC(NULL, hScreenDC);
		DeleteDC(hMemoryDC);
		return NULL;
	}

	HGDIOBJ hOldBitmap = SelectObject(hMemoryDC, hBitmap);
	if (!hOldBitmap) {
		std::cout << "Error in selecting bitmap" << std::endl;
		FileLogger::LogToFile("Error in selecting bitmap");
		// Handle error
		DeleteObject(hBitmap);
		ReleaseDC(NULL, hScreenDC);
		DeleteDC(hMemoryDC);
		return NULL;
	}
	//bit-block-transfer
	//if (!BitBlt(hMemoryDC, 0, 0, currentScreenBounds.Left, currentScreenBounds.Top, hScreenDC, currentScreenBounds.Right, currentScreenBounds.Bottom, SRCCOPY)) {
	if (!BitBlt(hMemoryDC, 0, 0, monitorWidth, monitorHeight, hScreenDC, selectedMonitorRect.left, selectedMonitorRect.top, SRCCOPY)) {
		// Handle BitBlt failure
		std::cout << "Error in BitBlt" << std::endl;
		FileLogger::LogToFile("Error in BitBlt");
		auto code = GetLastError();
		char strText[100];
		sprintf(strText, "The Error code: %d", code);
		FileLogger::LogToFile(strText);
		std::cout << Conversion::GetLastErrorAsString(code) << code << std::endl;
		FileLogger::LogToFile(Conversion::GetLastErrorAsString(GetLastError()));
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
// ________________________________________________
//
// GetBitmapData
//
// PURPOSE: 
// Helper Function to get all the Bitmaps generated
//
// RETURN VALUE:
// vector<uint8_t>: The bitmap data 
// ________________________________________________
//
std::vector<uint8_t> GetBitmapData(HBITMAP hBitmap, BITMAPINFO& bmpInfo) {
	if (!hBitmap) {

		// Handle null bitmap error
		std::cout << "Error in getting bitmap data" << std::endl;
		FileLogger::LogToFile("Error in getting bitmap data");
		return {};
	}

	HDC hDC = GetDC(NULL);
	HDC hMemDC = CreateCompatibleDC(hDC);

	BITMAP bmp;
	if (!GetObject(hBitmap, sizeof(BITMAP), &bmp)) {

		// Handle GetObject failure
		std::cout << "Error in getting object" << std::endl;
		FileLogger::LogToFile("Error in getting object");
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
		FileLogger::LogToFile("Error in getting DIBits");
		ReleaseDC(NULL, hDC);
		DeleteDC(hMemDC);
		return {};
	}

	ReleaseDC(NULL, hDC);
	DeleteDC(hMemDC);

	return bitmapData;
}

void invokeUsingCmdArgs(char* argv[]);
