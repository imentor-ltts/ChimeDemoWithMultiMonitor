// --------------------------------------------------------------------------------------------------------------------
// <copyright file="DisplayInfo.h" company="Siemens Ultrasound">
// Copyright(c) Since 2022 by Siemens Healthineers
// All Rights Reserved.
// No part of this software may be reproduced or transmitted in any
// form or by any means including photocopying or recording without
// written permission of the copyright owner.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------
#pragma once
#include<Windows.h>
#include<string>
#include<vector>
#include "Conversion.h"
using namespace std;


// Struct to hold monitor info
struct MonitorInfo {
    int index;
    RECT rect;
};

//Contains the list of monitors associated with the System.
std::vector<MonitorInfo> monitors;

//Contains the information about the secondary Monitor
static RECT selectedMonitorRect = { 0, 0, 0, 0 };
//Contains the information about the primary Monitor
static RECT primaryMonitorRect = { 0, 0, 0, 0 };


struct DisplayInformation
{
    static BOOL CALLBACK MonitorEnum(HMONITOR hMon, HDC hdc, LPRECT lprcMonitor, LPARAM pData)
    {
        DisplayInformation* pThis = reinterpret_cast<DisplayInformation*>(pData);
        MONITORINFOEX mi;
        mi.cbSize = sizeof(mi);
        bool success = GetMonitorInfo(hMon, &mi);
        if (success) {
            MonitorInfo info;
            info.index = static_cast<int>(monitors.size());
            info.rect = mi.rcMonitor;
            monitors.push_back(info);
        }
        char strText[100];
        sprintf(strText, "The No of Monitors: %d", monitors.size());
        std::cout << strText << std::endl;
        return TRUE;
    }

    void EnumerateAndSelectMonitor(int monitorIndex) {
        monitors.clear();
        if (!EnumDisplayMonitors(0, 0, MonitorEnum, (LPARAM)this)) {
            std::cerr << "Error enumerating monitors." << std::endl;
            return;
        }

        // Print information for debugging purposes
        std::cout << "Total monitors detected: " << monitors.size() << std::endl;
        for (const auto& monitor : monitors) {
            std::cout << "Monitor " << monitor.index << ": Left=" << monitor.rect.left << ", Top=" << monitor.rect.top
                << ", Right=" << monitor.rect.right << ", Bottom=" << monitor.rect.bottom << std::endl;
        }

        // Check if the desired monitor index is within bounds
        if (monitorIndex >= 0 && monitorIndex < static_cast<int>(monitors.size())) {
            selectedMonitorRect = monitors[monitorIndex].rect;
            primaryMonitorRect = monitors[0].rect;
        }
        else {
            std::cerr << "Invalid monitor index: " << monitorIndex << ". Total monitors: " << monitors.size() << std::endl;
        }
    }
    DisplayInformation()
    {
       
    }

    int GetNoOfMonitors() {
        return monitors.size();
    }
};

