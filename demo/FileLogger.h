// --------------------------------------------------------------------------------------------------------------------
// <copyright file="FileLogger[.h" company="Siemens Ultrasound">
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
#include<sstream>
#include <ctime>

/// <summary>
/// File Logger Class for logging the Application.
/// </summary>
class FileLogger {
private:
	// ________________________________________________
	//
	// getCurrentDate
	//
	// PURPOSE: 
	// Helper function to get the Current System date as string.
	//
	// ARG:
	// STRING: The String object that contains the current date of the System.
	// 
	// RETURN VALUE:
	// A String that contains the date.
	// ________________________________________________
	//
static std::string getCurrentDate() {
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
public:
// ________________________________________________
//
// LogToFile
//
// PURPOSE: 
// Logs the messages to the default logger file.
//
// ARG:
// STRING: The message to be logged.
// 
// RETURN VALUE:
// INT: 0 if successfull, 1 if failed.
// ________________________________________________
//
	static int LogToFile(const std::string& message) {
		// Construct the filename based on the current date
		std::string filename = "C:\\ProgramData\\ChimeLogs\\Chimelog_" + getCurrentDate() + ".txt";

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
};




