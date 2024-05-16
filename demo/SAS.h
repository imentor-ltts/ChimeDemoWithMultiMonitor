#pragma once
#include <windows.h>
#include <wtsapi32.h>
#include <stdio.h>

#pragma comment(lib, "Wtsapi32.lib")

class AdvancedKeys {
public:
	static void SendSAS() {
		if (!WTSDisconnectSession(WTS_CURRENT_SERVER_HANDLE, WTS_CURRENT_SESSION, FALSE)) {
			printf("WTSDisconnectSession failed with error: %lu\n", GetLastError());
		}
	}
};