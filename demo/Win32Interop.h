#pragma once
//Win32Interop.h
#include <windows.h>

class Win32Interop {
public:
    static HDESK _lastInputDesktop;

    static bool SwitchToInputDesktop() {
        try {
            CloseDesktop(_lastInputDesktop);
            HDESK inputDesktop = OpenInputDesktop(0, TRUE, GENERIC_ALL);

            if (inputDesktop == nullptr) {
                return false;
            }

            bool result = SetThreadDesktop(inputDesktop) && SwitchDesktop(inputDesktop);
            _lastInputDesktop = inputDesktop;
            return result;
        }
        catch (...) {
            return false;
        }
    }
};
// Initialization of the static member
HDESK Win32Interop::_lastInputDesktop = nullptr;
