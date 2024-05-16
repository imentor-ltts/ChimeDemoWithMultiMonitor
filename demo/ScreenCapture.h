#pragma once
#include<Windows.h>
#include<string>
#include<vector>
#include <tchar.h>
#include <stdio.h>
#include <iostream>
#include"main.h"

using namespace std;

#pragma comment(lib,"gdiplus.lib")
using namespace std;
/// <summary>
/// Gets information about the Display
/// </summary>
struct DisplayInfo
{
    bool IsPrimary;
    POINT ScreenSize;
    RECT MonitorArea;
    RECT WorkArea;
    string DeviceName;
    HANDLE hMonitor;
};


struct cMonitorsVec
{
    std::vector<int>       iMonitors;
    std::vector<HMONITOR>  hMonitors;
    std::vector<HDC>       hdcMonitors;
    std::vector<RECT>      rcMonitors;

    static BOOL CALLBACK MonitorEnum(HMONITOR hMon, HDC hdc, LPRECT lprcMonitor, LPARAM pData)
    {
        static int monitorCount = 0;
        cMonitorsVec* pThis = reinterpret_cast<cMonitorsVec*>(pData);

        pThis->hMonitors.push_back(hMon);
        pThis->hdcMonitors.push_back(hdc);
        pThis->rcMonitors.push_back(*lprcMonitor);
        pThis->iMonitors.push_back(pThis->hdcMonitors.size());
        
        return TRUE;
    }

    cMonitorsVec()
    {
        EnumDisplayMonitors(0, 0, MonitorEnum, (LPARAM)this);
    }
};

void DisplayInfo(cMonitorsVec Monitors) 
{
	for (int monitorIndex = 0; monitorIndex < Monitors.iMonitors.size(); monitorIndex++)
	{
		std::wcout << "Screen id: " << monitorIndex << std::endl;
		std::wcout << "-----------------------------------------------------" << std::endl;
		std::wcout << " - screen left-top corner coordinates : (" << Monitors.rcMonitors[monitorIndex].left
			<< "," << Monitors.rcMonitors[monitorIndex].top
			<< ")" << std::endl;
		std::wcout << " - screen dimensions (width x height) : (" << std::abs(Monitors.rcMonitors[monitorIndex].right - Monitors.rcMonitors[monitorIndex].left)
			<< "," << std::abs(Monitors.rcMonitors[monitorIndex].top - Monitors.rcMonitors[monitorIndex].bottom)
			<< ")" << std::endl;
		std::wcout << "-----------------------------------------------------" << std::endl;
	}
}

#pragma comment(lib,"gdiplus.lib")


/**
 * Create a Bitmap file header..
 *
 * @param hwindowDC : window handle.
 * @param widht	    : image width.
 * @param height    : image height.
 *
 * @return Bitmap header.
 */
BITMAPINFOHEADER createBitmapHeader(int width, int height)
{
    BITMAPINFOHEADER  bi;

    // create a bitmap
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = width;
    bi.biHeight = -height;  //this is the line that makes it draw upside down or not
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    return bi;
}

HBITMAP GdiPlusScreenCapture(HWND hWnd)
{
    // get handles to a device context (DC)
    HDC hwindowDC = GetDC(hWnd);
    HDC hwindowCompatibleDC = CreateCompatibleDC(hwindowDC);
    SetStretchBltMode(hwindowCompatibleDC, COLORONCOLOR);

    // define scale, height and width
    int scale = 1;
    int screenx = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int screeny = GetSystemMetrics(SM_YVIRTUALSCREEN);
    int width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int height = GetSystemMetrics(SM_CYVIRTUALSCREEN);

    // create a bitmap
    HBITMAP hbwindow = CreateCompatibleBitmap(hwindowDC, width, height);
    BITMAPINFOHEADER bi = createBitmapHeader(width, height);

    // use the previously created device context with the bitmap
    SelectObject(hwindowCompatibleDC, hbwindow);

    // Starting with 32-bit Windows, GlobalAlloc and LocalAlloc are implemented as wrapper functions that call HeapAlloc using a handle to the process's default heap. 
    // Therefore, GlobalAlloc and LocalAlloc have greater overhead than HeapAlloc.
    DWORD dwBmpSize = ((width * bi.biBitCount + 31) / 32) * 4 * height;
    HANDLE hDIB = GlobalAlloc(GHND, dwBmpSize);
    char* lpbitmap = (char*)GlobalLock(hDIB);

    // copy from the window device context to the bitmap device context
    StretchBlt(hwindowCompatibleDC, 0, 0, width, height, hwindowDC, screenx, screeny, width, height, SRCCOPY);   //change SRCCOPY to NOTSRCCOPY for wacky colors !
    GetDIBits(hwindowCompatibleDC, hbwindow, 0, height, lpbitmap, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    // avoid memory leak
    DeleteDC(hwindowCompatibleDC);
    ReleaseDC(hWnd, hwindowDC);

    return hbwindow;
}

HBITMAP CaptureExtendedDesktop() {
    // Get the virtual screen dimensions.
    int x = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int y = GetSystemMetrics(SM_YVIRTUALSCREEN);
    int width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int height = GetSystemMetrics(SM_CYVIRTUALSCREEN);

    // Create a device context for the entire screen.
    HDC hScreenDC = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL);
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, width, height);
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemoryDC, hBitmap);

    // Copy the screen to the bitmap.
    BitBlt(hMemoryDC, 0, 0, width, height, hScreenDC, x, y, SRCCOPY);
    hBitmap = (HBITMAP)SelectObject(hMemoryDC, hOldBitmap);

    // Clean up.
    DeleteDC(hMemoryDC);
    DeleteDC(hScreenDC);

    return hBitmap;
    // At this point, hBitmap contains the captured image of the extended desktop.
    // You can use it as needed, for example, saving it to a file.
}

//class DisplayEnumeratorHelpers {
//public:
//    static vector<DisplayInfo> GetDisplays() {
//        vector<DisplayInfo> displays;
//
//        EnumDisplayMonitors(NULL, )
//    }
//};