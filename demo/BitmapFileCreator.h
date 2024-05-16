#pragma once
#include <Windows.h>
#include <gdiplus.h>
#include <gdiplusheaders.h>
#include <iostream>

using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")


std::wstring GenerateTimestampFilename() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::tm bt{};
    localtime_s(&bt, &time_t_now);

    std::wostringstream woss;
    woss << L"C:\\Chime-Cpp\\chime_\\screenshot_"
        << std::put_time(&bt, L"%Y%m%d%H%M%S")
        << L".png";
    return woss.str();
}

// Function to save HBITMAP to a file
bool SaveHBITMAPToFile(HBITMAP hBitmap, wstring fileName) {
    // Create a compatible DC
    HDC hdc = CreateCompatibleDC(NULL);
    if (hdc == NULL)
        return false;

    // Get the bitmap dimensions
    BITMAP bmp;
    GetObject(hBitmap, sizeof(BITMAP), &bmp);

    // Create a bitmap compatible with the DC
    HBITMAP hOldBmp = (HBITMAP)SelectObject(hdc, hBitmap);

    // Create a BITMAPINFO structure
    BITMAPINFOHEADER bi;
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = bmp.bmWidth;
    bi.biHeight = -bmp.bmHeight;
    bi.biPlanes = 1;
    bi.biBitCount = 24; // Change this to 32 for 32-bit images
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    // Get the size of the image data
    DWORD dwBmpSize = ((bmp.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmp.bmHeight;

    // Allocate memory for the image data
    BYTE* lpBits = new BYTE[dwBmpSize];

    // Get the image data
    GetDIBits(hdc, hBitmap, 0, bmp.bmHeight, lpBits, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    // Create file and write image data
    std::ofstream outFile(fileName, std::ios::binary);
    if (!outFile) {
        delete[] lpBits;
        SelectObject(hdc, hOldBmp);
        DeleteDC(hdc);
        return false;
    }
    outFile.write((char*)lpBits, dwBmpSize);

    // Clean up
    outFile.close();
    delete[] lpBits;
    SelectObject(hdc, hOldBmp);
    DeleteDC(hdc);

    return true;
}

bool SaveHBitmapToFile(HBITMAP hBitmap, const wchar_t* filePath) {
    // Create a compatible device context
    HDC hdc = CreateCompatibleDC(NULL);
    if (!hdc) {
        return false;
    }

    // Get bitmap dimensions
    BITMAP bmp;
    if (!GetObject(hBitmap, sizeof(BITMAP), &bmp)) {
        DeleteDC(hdc);
        return false;
    }

    // Create a BITMAPINFO structure
    BITMAPINFO bmi;
    ZeroMemory(&bmi, sizeof(BITMAPINFO));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = bmp.bmWidth;
    bmi.bmiHeader.biHeight = - bmp.bmHeight;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = bmp.bmBitsPixel;
    bmi.bmiHeader.biCompression = BI_RGB;

    // Allocate memory for the bitmap bits
    BYTE* bits = new BYTE[bmp.bmWidthBytes * bmp.bmHeight];
    if (!bits) {
        DeleteDC(hdc);
        return false;
    }

    // Get the bitmap bits
    if (!GetBitmapBits(hBitmap, bmp.bmWidthBytes * bmp.bmHeight, bits)) {
        delete[] bits;
        DeleteDC(hdc);
        return false;
    }

    // Create the file and write the bitmap data
    HANDLE file = CreateFile(filePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE) {
        delete[] bits;
        DeleteDC(hdc);
        return false;
    }

    DWORD bytesWritten;
    BITMAPFILEHEADER bmfh;
    bmfh.bfType = 0x4D42; // "BM"
    bmfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + bmp.bmWidthBytes * bmp.bmHeight;
    bmfh.bfReserved1 = 0;
    bmfh.bfReserved2 = 0;
    bmfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    WriteFile(file, &bmfh, sizeof(BITMAPFILEHEADER), &bytesWritten, NULL);
    WriteFile(file, &bmi.bmiHeader, sizeof(BITMAPINFOHEADER), &bytesWritten, NULL);
    WriteFile(file, bits, bmp.bmWidthBytes * bmp.bmHeight, &bytesWritten, NULL);

    CloseHandle(file);
    delete[] bits;
    DeleteDC(hdc);

    return true;
}
