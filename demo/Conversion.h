// --------------------------------------------------------------------------------------------------------------------
// <copyright file="Conversion.h" company="Siemens Ultrasound">
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
#include "DisplayInfo.h"


/// <summary>
/// Class that contains static methods to perform standard conversions in C++. 
/// </summary>
class Conversion {
public:
	// ________________________________________________
	//
	// ConvertToLPCWSTR
	//
	// PURPOSE: 
	// Convert string to LPCWSTR for low level API calls.
	//
	// ARG:
	// STRING: The String object to be converted to LPCWSTR.
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

	static std::string WideStringToString(const wchar_t* wideString)
	{
		// Get the length of the wide string
		int length = WideCharToMultiByte(CP_UTF8, 0, wideString, -1, NULL, 0, NULL, NULL);

		// Convert wide string to a UTF-8 encoded narrow string
		char* buffer = new char[length];
		WideCharToMultiByte(CP_UTF8, 0, wideString, -1, buffer, length, NULL, NULL);

		// Construct a std::string from the narrow string
		std::string narrowString(buffer);

		// Clean up buffer
		delete[] buffer;

		return narrowString;
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
		X = selectedMonitorRect.left + X;
		Y = selectedMonitorRect.top + Y;
		int monitorWidth = selectedMonitorRect.right - selectedMonitorRect.left;
		int monitorHeight = selectedMonitorRect.bottom - selectedMonitorRect.top;
		// Transform the resulting pixel coordinates into absolute coordinates.
		X = GetAbsoluteCoordinate(X, monitorWidth);
		Y = GetAbsoluteCoordinate(Y, monitorHeight);
	}

	// ________________________________________________
	//
	// GetLastErrorAsString
	//
	// PURPOSE: 
	// Convert the Error Code generated to a string.
	//
	// ARG:
	// ErrorCode: The Errorcode that is obtained from the GetLastError.
	// RETURN VALUE:
	// String Error message
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