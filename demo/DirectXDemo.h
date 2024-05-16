#pragma once
#include <DXGI.h>
#include<iostream>
#include "FileLogger.h"
#pragma comment(lib, "dxgi.lib")

void GetDisplays() {

    IDXGIFactory* pFactory;

    CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&pFactory);

    IDXGIAdapter* pAdapter;

    for (UINT i = 0; pFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND; ++i) {

        IDXGIOutput* pOutput;

        for (UINT j = 0; pAdapter->EnumOutputs(j, &pOutput) != DXGI_ERROR_NOT_FOUND; ++j) {
            DXGI_OUTPUT_DESC desc;

            pOutput->GetDesc(&desc);
            char strText[150];
            sprintf(strText, "Display: %ls, Screen Dimensions: (%d X %d)", desc.DeviceName, desc.DesktopCoordinates.right, desc.DesktopCoordinates.bottom);
            FileLogger::LogToFile(strText);
            std::wcout << L"Display: " << desc.DeviceName << L", " << desc.DesktopCoordinates.left << L", " << desc.DesktopCoordinates.top << L", " << desc.DesktopCoordinates.right << L", " << desc.DesktopCoordinates.bottom << std::endl;
            pOutput->Release();
        }
        pAdapter->Release();
    }
    pFactory->Release();
}
