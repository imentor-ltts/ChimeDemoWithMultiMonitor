// --------------------------------------------------------------------------------------------------------------------
// <copyright file="KeyBoardMouseService.h" company="Siemens Ultrasound">
// Copyright(c) Since 2022 by Siemens Healthineers
// All Rights Reserved.
// No part of this software may be reproduced or transmitted in any
// form or by any means including photocopying or recording without
// written permission of the copyright owner.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

#pragma once
#include <Windows.h>
#include <nlohmann\json.hpp>
#include<string>
#include<iostream>
#include<functional>
#include<vector>
#include<queue>
#include<thread>
#include <mutex>
#include <condition_variable>
#include "Win32Interop.h"
#include "Conversion.h"
#include "FileLogger.h"
#include "ThreadSafeQueue.h"
#include "VirtualKeys.h"
#include "EventMessage.h"
#include "DisplayInfo.h"
using namespace std;
using namespace EventMessages;


//////////////////////////////////GLOBAL REFERENCES///////////////////////////////////
using FunctionType = std::function<void()>;
///////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// Class for Handling Mouse Inputs.
/// </summary>
class KeyBoardMouseService
{
private:
	static ThreadSafeQueue<FunctionType> _inputActions;
	volatile BOOL _inputBlocked = TRUE;
	void CheckQueue(BOOL isCancellationRequested) {
		cout << "The Current Thread in CheckQueue is " << GetCurrentThreadId() << endl;
		const char* boolStr = isCancellationRequested ? "true" : "false";
		// Format string with bool value
		char buffer[100];
		std::sprintf(buffer, "My bool value is %s", boolStr);
		FileLogger::LogToFile(buffer);
		while (!isCancellationRequested) {
			try
			{

				std::cout << "Into the while loop" << endl;
				cout << "Got the first action" << endl;
				if (_inputActions.empty())
				{
					//Sleep for 100MS
					Sleep(100);
					continue;

				}
				cout << "Now calling the action" << endl;
				FunctionType action = _inputActions.pop();
				action();
			}
			catch (const std::exception& ex)
			{
				FileLogger::LogToFile(ex.what());
			}
			char strText[100];
			sprintf(strText, "Stopping input processing on thread %d", GetCurrentThreadId());
			FileLogger::LogToFile(strText);
		}
	}

	void inputProcessingThreadFunc() {
		FileLogger::LogToFile("New input processing thread started on thread");
		if (_inputBlocked) {
			//ToggleBlockInput(true);
		}
		cout << "The Current Thread in inputProcessingThreadFunc is " << GetCurrentThreadId() << endl;
		CheckQueue(FALSE);
	}

private:
	// ________________________________________________
	//
	// CreateKeyboardInput
	//
	// PURPOSE: 
	// Helper function for generating the Keyboard inputs for performing Keyboard operations.
	//
	// ARG:
	// VIRTUALKEY: The Virtual key for which the INPUT Struct has to be generated.
	// FLAGS: The Pre-defined flags that should be set for generating the INPUT STRUCT.
	// 
	// RETURN VALUE:
	// INPUT : The INPUT Struct that contains the key that has to be sent for processing in the SendInput API. 
	// ________________________________________________
	//
	INPUT CreateKeyboardInput(VirtualKeys virtualKey, DWORD flags = 0)
	{
		INPUT input = {};
		input.type = INPUT_KEYBOARD;
		input.ki.wVk = virtualKey;
		input.ki.wScan = MapVirtualKeyEx((UINT)virtualKey, MAPVK_VSC_TO_VK_EX, GetKeyboardLayout(GetCurrentThreadId()));
		input.ki.dwFlags = flags;
		input.ki.dwExtraInfo = GetMessageExtraInfo();
		PrintInput(input);
		return input;
	}
	// ________________________________________________
	//
	// CreateKeyboardInput
	//
	// PURPOSE: 
	// Helper function for generating the Keyboard inputs for performing Keyboard operations with non virtual Keys.
	//
	// ARG:
	// UNCODEKEY: The key for which the INPUT Struct has to be generated.
	// FLAGS: The Pre-defined flags that should be set for generating the INPUT STRUCT. Default is KEYEVENTF_UNICODE
	// 
	// RETURN VALUE:
	// INPUT : The INPUT Struct that contains the key that has to be sent for processing in the SendInput API. 
	// ________________________________________________
	//
	INPUT CreateKeyboardInput(short unicodeKey, DWORD flags = KEYEVENTF_UNICODE)
	{
		INPUT input = {};
		input.type = INPUT_KEYBOARD;
		input.ki.wVk = 0;
		input.ki.wScan = unicodeKey;
		input.ki.dwFlags = flags;
		input.ki.dwExtraInfo = GetMessageExtraInfo();
		PrintInput(input);
		return input;
	}


	// ________________________________________________
	//
	// GetKeyPressState
	//
	// PURPOSE: 
	// Helper function for extracting the Keystate of the virtual Keys(Pressed or not). 
	//
	// ARG:
	// key: The key whose state has to be checked.
	// 
	// RETURN VALUE:
	// MODIFIERSTATE : The MODIFIERSTATE Struct that gets the status of the virtual key. 
	// ________________________________________________
	//
	ModifierState GetKeyPressState(VirtualKeys key) {
		ModifierState status;
		auto state = GetKeyState(key);
		status.Pressed = state < 0;
		status.Toggled = (state & 1) != 0;
		return status;
	}

	// ________________________________________________
	//
	// PrintInput
	//
	// PURPOSE: 
	// Helper function for logging the INPUT details before we sent to SendInput API. 
	//
	// ARG:
	// INPUT: The INPUT Struct's information that needs to be logged.
	// 
	// RETURN VALUE:
	// VOID 
	// ________________________________________________
	//
	void PrintInput(INPUT input) {
		std::string details = "Input type: " + input.type;
		details += "\nInput scan code: " + input.ki.wScan;
		FileLogger::LogToFile(details);
	}

	// ________________________________________________
	//
	// IsModKeyPressed
	//
	// PURPOSE: 
	// Helper function to check if any of the Modifier Keys are pressed.  
	// 
	// RETURN VALUE:
	// BOOL : TRUE if any of the Modifer Keys(Ctrl, Alt, WinState) is pressed. 
	// ________________________________________________
	//
	bool IsModKeyPressed()
	{
		auto ctrlState = GetKeyPressState(VirtualKeys::Control);
		auto altState = GetKeyPressState(VirtualKeys::Menu);
		auto winState = GetKeyPressState(VirtualKeys::LWin);
		return ctrlState.Pressed || altState.Pressed || winState.Pressed;
	}
public:
	void StartInputProcessingThread() {
		FileLogger::LogToFile("Into creating a new thread");
		std::thread myThread(&KeyBoardMouseService::inputProcessingThreadFunc, service);
		FileLogger::LogToFile("Stated input processing thread. now detaching");
		myThread.detach();
	}


	void TryOnInputDesktop(FunctionType inputAction) {
		_inputActions.push([=]() {
			try {
				if (!Win32Interop::SwitchToInputDesktop()) {
					FileLogger::LogToFile("Desktop switch failed during input Processing");
					//Start a new thread for processing input
					FileLogger::LogToFile("Start a new thread for processing input");
					StartInputProcessingThread();
					return;
				}
				cout << "Calling the input Action" << endl;
				inputAction();
			}
			catch (...) {
				FileLogger::LogToFile("Error occured while performing inputAction");
				FileLogger::LogToFile(Conversion::GetLastErrorAsString(GetLastError()));
			}
			});
	}
public:
	void ToggleBlockInput(BOOL toggleOn) {
		// _inputActions.push([&]() {
		// 	_inputBlocked = toggleOn;
		// 	auto result = BlockInput(toggleOn);
		// 	FileLogger::LogToFile("Result of the ToggleBlockInput set to " + result);
		// 	});
	}


	// ________________________________________________
	//
	// GetDisplayScale
	//
	// PURPOSE: 
	// Gets the Display Scale for the current Monitor of the Machine
	//
	// RETURN VALUE:
	// VOID
	// ________________________________________________
	//
	void GetDisplayScale(double& h_Scale, double& v_Scale) {
		HWND activeWindow = GetDesktopWindow();
		HMONITOR monitor = MonitorFromWindow(activeWindow, MONITOR_DEFAULTTONEAREST);

		// Get the logical width and height of the monitor
		MONITORINFOEX monitorInfoEx;
		monitorInfoEx.cbSize = sizeof(monitorInfoEx);
		GetMonitorInfo(monitor, &monitorInfoEx);
		long cxLogical = monitorInfoEx.rcMonitor.right - monitorInfoEx.rcMonitor.left;
		long cyLogical = monitorInfoEx.rcMonitor.bottom - monitorInfoEx.rcMonitor.top;

		// Get the physical width and height of the monitor
		DEVMODE devMode;
		devMode.dmSize = sizeof(devMode);
		devMode.dmDriverExtra = 0;
		EnumDisplaySettings(monitorInfoEx.szDevice, ENUM_CURRENT_SETTINGS, &devMode);
		DWORD cxPhysical = devMode.dmPelsWidth;
		DWORD cyPhysical = devMode.dmPelsHeight;

		// Calculate the scaling factor
		h_Scale = static_cast<double>(cxPhysical) / static_cast<double>(cxLogical);
		v_Scale = static_cast<double>(cyPhysical) / static_cast<double>(cyLogical);

		// Round off to 2 decimal places
		h_Scale = round(h_Scale * 100.0) / 100.0;
		v_Scale = round(v_Scale * 100.0) / 100.0;

		char strText[100];
		sprintf(strText, "The hScale and v_Scale values: %d, %d", h_Scale, v_Scale);
		FileLogger::LogToFile(strText);
	}

	
	UINT SendLButtonDown(int xPos, int yPos) {
		TryOnInputDesktop([=]() {
			double horizontalScale, verticalScale;
			GetDisplayScale(horizontalScale, verticalScale);
			MOUSEINPUT mi;
			mi.dx = xPos * horizontalScale;
			mi.dy = yPos * verticalScale;
			mi.dwExtraInfo = GetMessageExtraInfo();
			mi.mouseData = 0;

			INPUT input;
			input.type = INPUT_MOUSE;
			input.mi = mi;
			std::cout << "xPos: " << mi.dx << " --yPos: " << mi.dy << std::endl;
			input.mi.dwFlags = (MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTDOWN);
			//input.mi.dwFlags = (MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTDOWN);
			auto retVal = SendInput(1, &input, sizeof(INPUT));
			if (retVal == 0) {
				FileLogger::LogToFile(Conversion::GetLastErrorAsString(GetLastError()));
			}
			});
		return 1;
	}

	// ________________________________________________
	//
	// SendRButtonDown
	//
	// PURPOSE: 
	// Simulates the Mouse Right Button click on the specified Mouse Coordinates
	//
	// RETURN VALUE:
	// UINT : The Value indicates the operation was successfull and returns 0 if the Simulation has failed. 
	// ________________________________________________
	//
	UINT SendRButtonDown(int xPos, int yPos) {
		TryOnInputDesktop([xPos, yPos, this]() {
			double horizontalScale, verticalScale;
			GetDisplayScale(horizontalScale, verticalScale);

			INPUT input = { 0 };
			input.type = INPUT_MOUSE;
			input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE | MOUSEEVENTF_RIGHTDOWN;; // Specify right mouse button down event
			input.mi.dx = xPos * horizontalScale;
			input.mi.dy = yPos * verticalScale;
			// Send the input event
			auto retVal = SendInput(1, &input, sizeof(INPUT));
			if (retVal == 0) {
				FileLogger::LogToFile(Conversion::GetLastErrorAsString(GetLastError()));
			}
			});
		return 1;
	}

	// ________________________________________________
	//
	// SendRButtonUp
	//
	// PURPOSE: 
	// Simulates the Right Button Up on the specified Mouse Coordinates
	//
	// RETURN VALUE:
	// UINT : The Value indicates the operation was successfull and returns 0 if the Simulation has failed.
	// ________________________________________________
	//
	UINT SendRButtonUp(int xPos, int yPos) {
		TryOnInputDesktop([=] {
			double horizontalScale, verticalScale;
			GetDisplayScale(horizontalScale, verticalScale);

			INPUT input{ 0 };
			input.type = INPUT_MOUSE;
			input.mi.dx = xPos * horizontalScale;
			input.mi.dy = yPos * verticalScale;
			input.mi.dwFlags = (MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_RIGHTUP);
			input.mi.dwExtraInfo = GetMessageExtraInfo();
			// Send the input event
			auto retVal = SendInput(1, &input, sizeof(INPUT));
			if (retVal == 0) {
				FileLogger::LogToFile(Conversion::GetLastErrorAsString(GetLastError()));
			}
			char strText[100];
			sprintf(strText, "The ret value after Mouse R Button Up:: %d", retVal);
			FileLogger::LogToFile(strText);
			});
		return 1;
	}


	// ________________________________________________
	//
	// SendMouseMove
	//
	// PURPOSE: 
	// Simulates the Mouse movement to the specified Mouse Coordinates
	//
	// RETURN VALUE:
	// UINT : The Value indicates the operation was successfull and returns 0 if the Simulation has failed.
	// ________________________________________________
	//
	UINT SendMouseMove(int xPos, int yPos) {
		FileLogger::LogToFile("Into mouse move");
		TryOnInputDesktop([=]() {
				UINT retVal = 0;
				double horizontalScale, verticalScale;
				GetDisplayScale(horizontalScale, verticalScale);
				int mouseX = xPos * horizontalScale;;
				int mouseY = yPos * verticalScale;
				
				std::string _text;
				_text = "The xPos: " + to_string(xPos) + " The yPos: " + to_string(yPos);
				FileLogger::LogToFile(_text);
				cout << _text << endl;

				INPUT input{ 0 };
				input.mi.dx = mouseX;
				input.mi.dy = mouseY;
				input.mi.dwExtraInfo = GetMessageExtraInfo();
				input.mi.dwFlags = (MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE);
				retVal = SendInput(1, &input, sizeof(INPUT));
				if (retVal == 0)
				{
					FileLogger::LogToFile(Conversion::GetLastErrorAsString(GetLastError()));
					return retVal;
				}
				char strText[100];
				sprintf(strText, "The ret value after MouseMove:: %d", retVal);
				FileLogger::LogToFile(strText);
			});
			return 1;
			}


			// ________________________________________________
			//
			// SendMouseUp
			//
			// PURPOSE: 
			// Simulates the Left Button Up on the targetted Window
			//
			// RETURN VALUE:
			// UINT : The Value indicates the operation was successfull and returns 0 if the Simulation has failed.
			// ________________________________________________
			//
			UINT SendMouseUp(int xPos, int yPos)
			{
				TryOnInputDesktop([=] {
					INPUT inputMouse{ 0 };
					inputMouse.mi.dwFlags = (MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTUP | MOUSEEVENTF_VIRTUALDESK);
					auto retVal = SendInput(1, &inputMouse, sizeof(INPUT));
					if (retVal == 0)
					{
						FileLogger::LogToFile(Conversion::GetLastErrorAsString(GetLastError()));
					}
					else {
						char strText[100];
						sprintf(strText, "The ret value after MouseMove:: %d", retVal);
						FileLogger::LogToFile(strText);
					}
					});
				return 1;
			}
			
			UINT SendCtrlAltDel() {
				TryOnInputDesktop([=]() {
					INPUT inputs[3];

					// Press Ctrl
					inputs[0].type = INPUT_KEYBOARD;
					inputs[0].ki.wVk = VK_CONTROL;
					inputs[0].ki.dwFlags = 0;

					// Press Alt
					inputs[1].type = INPUT_KEYBOARD;
					inputs[1].ki.wVk = VK_MENU; // VK_MENU is the virtual-key code for Alt
					inputs[1].ki.dwFlags = 0;

					// Press Del
					inputs[2].type = INPUT_KEYBOARD;
					inputs[2].ki.wVk = VK_DELETE;
					inputs[2].ki.dwFlags = 0;

					// Send the inputs
					SendInput(3, inputs, sizeof(INPUT));

					// Release all keys
					inputs[0].ki.dwFlags = KEYEVENTF_KEYUP;
					inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
					inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;

					// Send the key releases
					auto retVal = SendInput(3, inputs, sizeof(INPUT));
				});
				return 3;
			}
			UINT SendKeyUp(std::string key)
			{
				TryOnInputDesktop([=]() {
					auto retVal = 0;
					VirtualKeys vKey = (VirtualKeys)0;
					if (key.size() == 1) {
						WCHAR charecter = key.front();
						if (!IsModKeyPressed()) {
							auto keyCode = static_cast<short>(charecter);
							INPUT inputs[1] = {};
							inputs[0] = CreateKeyboardInput(keyCode, KEYEVENTF_KEYUP | KEYEVENTF_UNICODE);
							retVal = SendInput(1, inputs, sizeof(INPUT));
							if (retVal != 1) {
								FileLogger::LogToFile("Unable to process Unicode key");
							}
							return retVal;
						}
						else {
							INPUT input[1] = {};
							auto scannedKey = (VirtualKeys)VkKeyScan(charecter);
							input[0] = CreateKeyboardInput(scannedKey, KEYEVENTF_KEYUP);
							retVal = SendInput(1, input, sizeof(INPUT));
							if (retVal != 1) {
								FileLogger::LogToFile("Unable to process Scanned key");
							}
							return retVal;
						}
					}
					else {
						bool result = ConvertJSKeyToVirtualKey(key, vKey);
						if (result) {
							INPUT input[1] = {};
							input[0] = CreateKeyboardInput(vKey, KEYEVENTF_KEYUP | KEYEVENTF_EXTENDEDKEY);
							retVal = SendInput(1, input, sizeof(INPUT));
							if (retVal != 1) {
								FileLogger::LogToFile("Unable to process Scanned key");
							}
							return retVal;
						}
						else {
							std::cout << "Unable to simulate key input " << key << std::endl;
							return -1;
						}
					}
					});
				return 1;
			}
			// ________________________________________________
			//
			// SendAltKeyDown
			//
			// PURPOSE: 
			// Simulates the Alt Key Down on the targeted Window
			//
			// RETURN VALUE:
			// UINT : The Value indicates the operation was successfull and returns 0 if the Simulation has failed.
			// ________________________________________________
			//
			UINT SendAltKeyDown() {
				TryOnInputDesktop([=]() {
					// Create an array of INPUT to hold the keystroke information
					INPUT inputs[1] = {};

					// Press the Alt key
					inputs[0].type = INPUT_KEYBOARD;
					inputs[0].ki.wVk = VK_MENU; // VK_MENU is the virtual key code for the Alt key
					inputs[0].ki.dwFlags = 0; // 0 for key press

					// Send the keystroke
					UINT uSent = SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
					if (uSent != ARRAYSIZE(inputs)) {
						// The SendInput call failed or didn't send all inputs
						char strText[100];
						sprintf(strText, "Failed to send input Alt Key Down");
						FileLogger::LogToFile(strText);
					}
					});
				return 1;
			}

			// ________________________________________________
			//
			// SendAltKeyUp
			//
			// PURPOSE: 
			// Simulates the Alt Key Up on the targeted Window
			//
			// RETURN VALUE:
			// UINT : The Value indicates the operation was successfull and returns 0 if the Simulation has failed.
			// ________________________________________________
			//
			UINT SendAltKeyUp() {
				TryOnInputDesktop([=]() {
					// Create an array of INPUT to hold the keystroke information
					INPUT inputs[1] = {};

					// Press the Alt key
					inputs[0].type = INPUT_KEYBOARD;
					inputs[0].ki.wVk = VK_MENU; // VK_MENU is the virtual key code for the Alt key
					inputs[0].ki.dwFlags = KEYEVENTF_KEYUP; // 0 for key press

					// Send the keystroke
					UINT uSent = SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
					if (uSent != ARRAYSIZE(inputs)) {
						// The SendInput call failed or didn't send all inputs
						//MessageBox(NULL, L"Failed to send input", L"Error", MB_OK);
						char strText[100];
						sprintf(strText, "Failed to send Alt Key Up");
						FileLogger::LogToFile(strText);
					}
					});
				return 1;
			}

			// ________________________________________________
			//
			// SendKeyDown
			//
			// PURPOSE: 
			// Simulates the specified Key Down on the targeted Window
			//
			// ARG:
			// KEY: The key that is to be down from the std Keyboard. 
			// 
			// RETURN VALUE:
			// UINT : The Value indicates the operation was successfull and returns 0 if the Simulation has failed.
			// ________________________________________________
			//
			UINT SendKeyDown(std::string key)
			{
				TryOnInputDesktop([=]() {
					VirtualKeys vKey = (VirtualKeys)0;
					if (key.size() == 1) {
						WCHAR charecter = key.front();
						INPUT inputs[1] = {};
						if (!IsModKeyPressed()) {
							auto keyCode = static_cast<short>(charecter);
							inputs[0] = CreateKeyboardInput(keyCode);
						}
						else {
							auto scannedKey = (VirtualKeys)VkKeyScan(charecter);
							inputs[0] = CreateKeyboardInput(scannedKey);
						}
						auto retVal = SendInput(1, inputs, sizeof(INPUT));
						if (retVal == 0)
						{
							auto errCode = GetLastError();
							auto strErrMessage = Conversion::GetLastErrorAsString(errCode);
							FileLogger::LogToFile(strErrMessage);
						}
						char strText[100];
						sprintf(strText, "The ret value after KeyDown:: %d", retVal);
						FileLogger::LogToFile(strText);
					}
					else {
						bool result = ConvertJSKeyToVirtualKey(key, vKey);
						if (result) {
							INPUT input[1] = {};
							input[0] = CreateKeyboardInput(vKey, KEYEVENTF_EXTENDEDKEY);
							auto retVal = SendInput(1, input, sizeof(INPUT));
							if (retVal == 0)
							{
								auto errCode = GetLastError();
								auto strErrMessage = Conversion::GetLastErrorAsString(errCode);
								FileLogger::LogToFile(strErrMessage);
							}
							char strText[100];
							sprintf(strText, "The ret value after KeyDown:: %d", retVal);
							FileLogger::LogToFile(strText);
						}
						else {
							std::cout << "Unable to simulate key input " << key << std::endl;
							FileLogger::LogToFile("Unable to simulate key input " + key);
						}
					}
					});
			}

			// ________________________________________________
			//
			// SendText
			//
			// PURPOSE: 
			// Simulates the text to be pasted from the ClipBoard on the targeted Window
			//
			// ARG:
			// TRANSFERTEXT: The content that is to be transfered from the Clipboard to the targetted location. 
			// 
			// RETURN VALUE:
			// UINT : The Value indicates the number of SendInputs and determines that the operation was successfull and returns 0 if the Simulation has failed.
			// ________________________________________________
			//
			UINT SendText(std::string transferText) {
				TryOnInputDesktop([=]() {
					int length = transferText.length();
					INPUT* inputDown = new INPUT[length];
					INPUT* inputUp = new INPUT[length];
					vector<INPUT> inputs;
					ZeroMemory(inputDown, sizeof(inputDown));
					ZeroMemory(inputUp, sizeof(inputUp));
					for (int i = 0; i < length; i++) {
						inputDown[i].type = INPUT_KEYBOARD;
						inputDown[i].ki.wVk = 0;
						inputDown[i].ki.wScan = transferText[i];
						inputDown[i].ki.dwFlags = KEYEVENTF_UNICODE;
						inputs.push_back(inputDown[i]);

						inputUp[i].type = INPUT_KEYBOARD;
						inputUp[i].ki.wVk = 0;
						inputUp[i].ki.wScan = transferText[i];
						inputUp[i].ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
						inputs.push_back(inputUp[i]);
					}
					INPUT* inputData = inputs.data();
					FileLogger::LogToFile("The Paste operation is done with data: " + transferText);
					auto retVal = SendInput(inputs.size(), inputData, sizeof(INPUT));
					char strText[100];
					sprintf(strText, "The ret value after transferText:: %d", retVal);
					FileLogger::LogToFile(strText);
					});
			}
	}service;

//Static variable initialization....
ThreadSafeQueue<std::function<void()>>KeyBoardMouseService::_inputActions;

