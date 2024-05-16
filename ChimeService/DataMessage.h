#pragma once
#include <Windows.h>
#include <nlohmann\json.hpp>
#include<string>
#include<iostream>
#include<vector>
#include "main.h"
using namespace std;

namespace MouseEventMessages {
    /// <summary>
    /// Represents the input mouse message sent by the joinee
    /// </summary>
    struct DataMessage {
        int eventId =0;
        float xPos = 0;
        float yPos = 0;
        WORD keyCode = -1;
        std::string message = "";
        int monitor = 1;
    };

    
    /// <summary>
    /// Helper function to parse the Json Input message
    /// </summary>
    /// <param name="j">Json data to be parsed</param>
    /// <param name="r"></param>
    void from_json(const nlohmann::json& j, DataMessage& m) {
        j.at("eventId").get_to(m.eventId);
        j.at("xPos").get_to(m.xPos);
        j.at("yPos").get_to(m.yPos);
        j.at("keyCode").get_to(m.keyCode);
        j.at("message").get_to(m.message);
        j.at("monitor").get_to(m.monitor);
    }
}

using namespace MouseEventMessages;

/// <summary>
/// Helper class for Handling Mouse Inputs.
/// </summary>
class MouseService
{
public:

	/// <summary>
	/// Helper function to get the scaling Units. 
	/// </summary>
	/// <param name="h_Scale">Horizontal scale</param>
	/// <param name="v_Scale">Vertical Scale</param>
	static void getDisplayScale(double& h_Scale, double& v_Scale) {
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

	}
    // ________________________________________________
    //
    // SendLButtonDown
    //
    // PURPOSE: 
    // Simulates the Mouse Left Button click on the specified Mouse Coordinates
    //
    // RETURN VALUE:
    // VOID
    // ________________________________________________
    //
    static UINT SendLButtonDown(int xPos, int yPos) {
        double horizontalScale, verticalScale;
        getDisplayScale(horizontalScale, verticalScale);

        MOUSEINPUT mi;
        mi.dx = xPos * horizontalScale;
        mi.dy = yPos * verticalScale;
        mi.dwExtraInfo = GetMessageExtraInfo();
        mi.mouseData = 0;

        INPUT input;
        input.type = INPUT_MOUSE;
        input.mi = mi;
        std::cout << "xPos: " << mi.dx << " --yPos: " << mi.dy << std::endl;  
        input.mi.dwFlags = (MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTDOWN );
        auto retVal = SendInput(1, &input, sizeof(INPUT));
        if (retVal == 0) 
        {
            auto errCode = GetLastError();
            auto strErrMessage = Conversion::GetLastErrorAsString(errCode);
            logToFile(strErrMessage);
            return retVal;
        }
        char strText[100];
        sprintf(strText, "The Left Mouse button retVal is : %d", retVal);
        logToFile(strText);
        return retVal;
    }

    // ________________________________________________
    //
    // SendRButtonDown
    //
    // PURPOSE: 
    // Simulates the Mouse Right Button click on the specified Mouse Coordinates
    //
    // RETURN VALUE:
    // VOID
    // ________________________________________________
    //
    static UINT SendRButtonDown(int xPos, int yPos) {
        double horizontalScale, verticalScale;
        getDisplayScale(horizontalScale, verticalScale);

        INPUT input = { 0 };
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE | MOUSEEVENTF_RIGHTDOWN;; // Specify right mouse button down event
        input.mi.dx = xPos * horizontalScale;
        input.mi.dy = yPos * verticalScale;
        // Send the input event
        auto retVal =  SendInput(1, &input, sizeof(INPUT));
        char strText[100];
        sprintf(strText, "The ret value after RButton Down:: %d", retVal);
        logToFile(strText);
        return retVal;
    }

    // ________________________________________________
    //
    // SendRButtonUp
    //
    // PURPOSE: 
    // Simulates the Right Button Up on the specified Mouse Coordinates
    //
    // RETURN VALUE:
    // VOID
    // ________________________________________________
    //
    static UINT SendRButtonUp(int xPos, int yPos) {
        double horizontalScale, verticalScale;
        getDisplayScale(horizontalScale, verticalScale);

        INPUT input{ 0 };
        input.type = INPUT_MOUSE;
        input.mi.dx = xPos * horizontalScale;
        input.mi.dy = yPos * verticalScale;
        input.mi.dwFlags = (MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_RIGHTUP);
        input.mi.dwExtraInfo = GetMessageExtraInfo();
        auto retVal = SendInput(1, &input, sizeof(INPUT));
        char strText[100];
        sprintf(strText, "The ret value after RButton Up:: %d", retVal);
        logToFile(strText);
        return retVal;
    }


    // ________________________________________________
   //
   // SendMouseMove
   //
   // PURPOSE: 
   // Simulates the Mouse movement to the specified Mouse Coordinates
   //
   // RETURN VALUE:
   // VOID
   // ________________________________________________
   //
	static UINT SendMouseMove(int xPos, int yPos) {
        double horizontalScale, verticalScale;
        getDisplayScale(horizontalScale, verticalScale);
        INPUT input{ 0 };
        input.mi.dx = xPos * horizontalScale;
        input.mi.dy = yPos * verticalScale;
		input.mi.dwExtraInfo = GetMessageExtraInfo();
		input.mi.dwFlags = (MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE);
        auto retVal = SendInput(1, &input, sizeof(INPUT));
        if (retVal == 0)
        {
            auto errCode = GetLastError();
            auto strErrMessage = Conversion::GetLastErrorAsString(errCode);
            logToFile(strErrMessage);
            return retVal;
        }
        char strText[100];
        sprintf(strText, "The ret value after MouseMove:: %d", retVal);
        logToFile(strText);
        return retVal;
	}

    // ________________________________________________
    //
    // MouseClick
    //
    // PURPOSE: 
    // Simulates the Mouse Click on the specified Mouse Coordinates
    //
    // RETURN VALUE:
    // VOID
    // ________________________________________________
    //
    static UINT MouseClick()
    {
        INPUT inputMouse{ 0 };
        inputMouse.mi.dwFlags = (MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTDOWN);
        auto retVal = SendInput(1, &inputMouse, sizeof(INPUT));
        char strText[100];
        sprintf(strText, "The ret value after Mouse Click:: %d", retVal);
        logToFile(strText);
        Sleep(10);
        return retVal;
        //Commenting to test on Mouse drag feature 
       /* inputMouse.mi.dwFlags = (MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTUP);
        return SendInput(1, &inputMouse, sizeof(INPUT));*/
    }

    // ________________________________________________
    //
    // SendMouseUp
    //
    // PURPOSE: 
    // Simulates the Left Button Up on the targetted Window
    //
    // RETURN VALUE:
    // VOID
    // ________________________________________________
    //
    static UINT SendMouseUp() 
    {
        INPUT inputMouse{ 0 };
        inputMouse.mi.dwFlags = (MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTUP);
        auto retVal = SendInput(1, &inputMouse, sizeof(INPUT));
        if (retVal == 0)
        {
            auto errCode = GetLastError();
            auto strErrMessage = Conversion::GetLastErrorAsString(errCode);
            logToFile(strErrMessage);
        }
        char strText[100];
        sprintf(strText, "The ret value after MouseMove:: %d", retVal);
        logToFile(strText);
        return retVal;
    }
};

/// <summary>
/// Represents the virtual keys of the OS for JS conversion.
/// </summary>
enum VirtualKeys {
    Down = VK_DOWN, 
    Up = VK_UP,
    Left = VK_LEFT,
    Right = VK_RIGHT,
    Enter = VK_RETURN,
    Esc = VK_ESCAPE,
    Menu = 12,
    Control = VK_CONTROL,
    Shift = VK_SHIFT,
    PAUSE = VK_PAUSE,
    BREAK = VK_PAUSE,
    Backspace = VK_BACK,
    Tab = VK_TAB,
    CapsLock = VK_CAPITAL,
    Delete = VK_DELETE,
    Home = VK_HOME,
    End = VK_END,
    PageUp = VK_PRIOR,
    PageDown = VK_NEXT,
    NumLock = VK_NUMLOCK,
    Insert = VK_INSERT,
    ScrollLock = VK_SCROLL,
    F1 = VK_F1,
    F2 = VK_F2,
    F3 = VK_F3,
    F4 = VK_F4,
    F5 = VK_F5,
    F6 = VK_F6,
    F7 = VK_F7,
    F8 = VK_F8,
    F9 = VK_F9,
    F10 = VK_F10,
    F11 = VK_F11,
    F12 = VK_F12,
    LWin = VK_LWIN,
    RWin = VK_RWIN
};


bool ConvertJSKeyToVirtualKey(std::string key, VirtualKeys& vKey) {
    if (key == "ArrowDown") { vKey = VirtualKeys::Down; return true; }
    if (key == "ArrowUp") { vKey = VirtualKeys::Up; return true; }
    if (key == "ArrowLeft") { vKey = VirtualKeys::Left; return true; }
    if (key == "ArrowRight") { vKey = VirtualKeys::Right; return true; }
    if (key == "Enter") { vKey = VirtualKeys::Enter; return true; }
    if (key == "Escape") { vKey = VirtualKeys::Esc; return true; }
    if (key == "Alt") { vKey = VirtualKeys::Menu; return true; }
    if (key == "Control") { vKey = VirtualKeys::Control; return true; }
    if (key == "Shift") { vKey = VirtualKeys::Shift; return true; }
    if (key == "PAUSE") { vKey = VirtualKeys::PAUSE; return true; }
    if (key == "BREAK") { vKey = VirtualKeys::PAUSE; return true; }
    if (key == "Backspace") { vKey = VirtualKeys::Backspace; return true; }
    if (key == "Tab") { vKey = VirtualKeys::Tab; return true; }
    if (key == "CapsLock") { vKey = VirtualKeys::CapsLock; return true; }
    if (key == "Delete") { vKey = VirtualKeys::Delete; return true; }
    if (key == "Home") { vKey = VirtualKeys::Home; return true; }
    if (key == "End") { vKey = VirtualKeys::End; return true; }
    if (key == "PageUp") { vKey = VirtualKeys::PageUp; return true; }
    if (key == "PageDown") { vKey = VirtualKeys::PageDown; return true; }
    if (key == "NumLock") { vKey = VirtualKeys::NumLock; return true; }
    if (key == "Insert") { vKey = VirtualKeys::Insert; return true; }
    if (key == "ScrollLock") { vKey = VirtualKeys::ScrollLock; return true; }
    if (key == "F1") { vKey = VirtualKeys::F1; return true; }
    if (key == "F2") { vKey = VirtualKeys::F2; return true; }
    if (key == "F3") { vKey = VirtualKeys::F3; return true; }
    if (key == "F4") { vKey = VirtualKeys::F4; return true; }
    if (key == "F5") { vKey = VirtualKeys::F5; return true; }
    if (key == "F6") { vKey = VirtualKeys::F6; return true; }
    if (key == "F7") { vKey = VirtualKeys::F7; return true; }
    if (key == "F8") { vKey = VirtualKeys::F8; return true; }
    if (key == "F9") { vKey = VirtualKeys::F9; return true; }
    if (key == "F10") { vKey = VirtualKeys::F10; return true; }
    if (key == "F11") { vKey = VirtualKeys::F11; return true; }
    if (key == "F12") { vKey = VirtualKeys::F12; return true; }
    if (key == "Meta" || key == "Win") { vKey = VirtualKeys::LWin; return true; }
    if (vKey == 0) return false;
    return false;
}

struct ModifierState {
    bool Pressed;
    bool Toggled;
};

/// <summary>
/// Helper Class to handle all keyboard inputs sent as Data Messages
/// </summary>
class KeyBoardService {
public:
    static UINT SendKeyUp(std::string key) 
    {
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
                    logToFile("Unable to process Unicode key");
                }
                    return retVal;
            }
            else {
                INPUT input[1] = {};
                auto scannedKey = (VirtualKeys)VkKeyScan(charecter);
                input[0] = CreateKeyboardInput(scannedKey, KEYEVENTF_KEYUP  );
                retVal = SendInput(1, input, sizeof(INPUT));
                if (retVal != 1) {
                    logToFile("Unable to process Scanned key");
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
                    logToFile("Unable to process Scanned key");
                }
                return retVal;
            }
            else {
                std::cout << "Unable to simulate key input " << key << std::endl;
                return -1;
            }
        }
    }

    static UINT SendAltKeyDown() {
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
            logToFile(strText);
		}
        return uSent;
    }

    static UINT SendAltKeyUp() {
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
            logToFile(strText);
        }
        return uSent;
    }

    static UINT SendKeyDown(std::string key) 
    {
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
                logToFile(strErrMessage);
            }
            char strText[100];
            sprintf(strText, "The ret value after KeyDown:: %d", retVal);
            logToFile(strText);
            return retVal;
		}
        else {
            bool result = ConvertJSKeyToVirtualKey(key, vKey);
            if (result){
                INPUT input[1] = {};
                input[0] = CreateKeyboardInput(vKey, KEYEVENTF_EXTENDEDKEY);
                auto retVal = SendInput(1, input, sizeof(INPUT));
                if (retVal == 0)
                {
                    auto errCode = GetLastError();
                    auto strErrMessage = Conversion::GetLastErrorAsString(errCode);
                    logToFile(strErrMessage);
                }
                char strText[100];
                sprintf(strText, "The ret value after KeyDown:: %d", retVal);
                logToFile(strText);
                return retVal;
            }
		    else {
			    std::cout << "Unable to simulate key input " << key << std::endl;
                logToFile("Unable to simulate key input " + key);
                return -1;
		    }
        }
    }
    static UINT SendText(std::string transferText) {
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
        logToFile("The Paste operation is done with data: " + transferText);
        return SendInput(inputs.size(), inputData, sizeof(INPUT));
    };

    static INPUT CreateKeyboardInput(VirtualKeys virtualKey, DWORD flags = 0)
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

    static INPUT CreateKeyboardInput(short unicodeKey, DWORD flags = KEYEVENTF_UNICODE)
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

    static ModifierState GetKeyPressState(VirtualKeys key) {
        ModifierState status;
        auto state = GetKeyState(key);
        status.Pressed = state < 0;
        status.Toggled = (state & 1) != 0;
        return status;
    }

    static void PrintInput(INPUT input) {
        std::string details = "Input type: " + input.type;
        details += "\nInput scan code: " + input.ki.wScan;
        logToFile(details);
    }
    static bool IsModKeyPressed()
    {
        auto ctrlState = GetKeyPressState(VirtualKeys::Control);
        auto altState = GetKeyPressState(VirtualKeys::Menu);
        auto winState = GetKeyPressState(VirtualKeys::LWin);
        return ctrlState.Pressed || altState.Pressed || winState.Pressed;
    }
};

