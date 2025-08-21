
#include <windows.h>
#include <iostream>
#include <gdiplus.h>

#include <thread>
#pragma comment (lib, "gdiplus.lib")

//#define STB_IMAGE_IMPLEMENTATION
//#include "stb-master/stb_image.h"
//#define STBI_GAMMA_CORRECT

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


#include "Resource.h"


// vcpkg install stb
//

#include <dwmapi.h> // For DwmExtendFrameIntoClientArea
#pragma comment(lib, "dwmapi.lib")

void ConvertRGBAToBGRA(unsigned char* data, int width, int height);


void ShakeWindow(HWND hWnd) {
    RECT rect;
    GetWindowRect(hWnd, &rect);
    int originalX = rect.left;
    int originalY = rect.top;

    for (int i = 0; i < 15; i++) {
        SetWindowPos(hWnd, NULL, originalX + ((i % 2 == 0) ? 10 : -10), originalY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        Sleep(10);
    }
     SetWindowPos(hWnd, NULL, originalX, originalY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
   // 
}


void EnableBlurBehind(HWND hWnd) {
    DWM_BLURBEHIND bb = { 0 };
    bb.dwFlags = DWM_BB_ENABLE;
    bb.fEnable = TRUE;
    bb.hRgnBlur = NULL;
    DwmEnableBlurBehindWindow(hWnd, &bb);
}

void FadeWindow(HWND hWnd, int startAlpha, int endAlpha, int stepDelay) {

 

    int step = (endAlpha > startAlpha) ? 1 : -1;
    for (int alpha = startAlpha; alpha != endAlpha; alpha += step) {
        SetLayeredWindowAttributes(hWnd, 0, alpha, LWA_ALPHA);
        Sleep(stepDelay);
    }
     SetLayeredWindowAttributes(hWnd, 0, endAlpha, LWA_ALPHA);
     ShowWindow(hWnd, SW_HIDE);
    
}



struct ImageData {
    unsigned char* data;
    int width;
    int height;
    int channels;
};

// Functions for loading and freeing image data
ImageData LoadPNG(const char* filename) {
    ImageData image = { nullptr, 0, 0, 0 };

    //image.data = stbi_load(filename, &image.width, &image.height, &image.channels, 3); // rgb

   // image.data = stbi_load(filename, &image.width, &image.height, &image.channels, 0); // No forced conversion

    image.data = stbi_load(filename, &image.width, &image.height, &image.channels, 4); // Force RGBA
    if (!image.data) {
        std::cerr << "Failed to load image: " << filename << std::endl;
    }

    // Ensure the image data is converted from RGBA to BGRA
    ConvertRGBAToBGRA(image.data, image.width, image.height);

    image.channels = 4;

    return image;
}

// Function to load PNG from resource
ImageData LoadPNGFromResource(HINSTANCE hInstance, int resourceID) {
    ImageData image = { nullptr, 0, 0, 0 };

    // Find the resource and load it
    //HRSRC hRes = FindResource(hInstance, MAKEINTRESOURCE(resourceID), RT_RCDATA);
    HRSRC hRes = FindResource(hInstance, MAKEINTRESOURCE(resourceID), L"PNG");
    if (hRes == NULL) {


        MessageBox(NULL, L"Failed to find resource.1", L"Error", MB_ICONERROR | MB_OK);
        return image;
    }

    // Load the resource data into memory
    HGLOBAL hResData = LoadResource(hInstance, hRes);
    if (hResData == NULL) {

        MessageBox(NULL, L"Failed to find resource.2", L"Error", MB_ICONERROR | MB_OK);
        return image;
    }

    // Lock the resource to get the pointer to the image data
    void* pResData = LockResource(hResData);
    DWORD resSize = SizeofResource(hInstance, hRes);

    // Decode the PNG image data from memory
    image.data = stbi_load_from_memory(reinterpret_cast<unsigned char*>(pResData), resSize, &image.width, &image.height, &image.channels, 4); // Force RGBA
    if (!image.data) {

        MessageBox(NULL, L"Failed to load image from resource", L"Error", MB_ICONERROR | MB_OK);
    }
    // Optionally, convert RGBA to BGRA if needed
    ConvertRGBAToBGRA(image.data, image.width, image.height);

    image.channels = 4;  // Set the number of channels to 4 (RGBA)

    return image;
}


void FreeImage(ImageData& image) {
    if (image.data) {
        stbi_image_free(image.data);
        image.data = nullptr;
    }
}

// Utility to create HBITMAP from pixel data
HBITMAP CreateBitmapFromData(HDC hdc, unsigned char* data, int width, int height, int channels) {
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height; // Negative to indicate top-down DIB
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = channels * 8; // Assume 4 channels (32 bits per pixel)
    bmi.bmiHeader.biCompression = BI_RGB;

    void* pBits = nullptr;
    HBITMAP hBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pBits, nullptr, 0);

    if (hBitmap && pBits) {
        memcpy(pBits, data, width * height * channels);
    }

    return hBitmap;
}

void ConvertRGBAToBGRA(unsigned char* data, int width, int height) {
    for (int i = 0; i < width * height; ++i) {
        unsigned char* pixel = data + i * 4;
        std::swap(pixel[0], pixel[2]); // Swap R (0) and B (2)
    }
}


LRESULT CALLBACK WndProcsplash(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;


        HDC hdc = BeginPaint(hwnd, &ps);

        // Load image and render (hardcoded file for demonstration)
       // static ImageData img = LoadPNG("skn.png");
        //static ImageData img = LoadPNG("skn2.png");


        // Load the PNG from resource
        HINSTANCE hInstance = GetModuleHandle(NULL);


        // int i = rand() % 2;
         // if(i==0) img = LoadPNGFromResource(hInstance, IDB_PNG1);
         // if (i == 1) ImageData img = LoadPNGFromResource(hInstance, IDB_PNG2);
         // else img = LoadPNGFromResource(hInstance, IDB_PNG4);

        ImageData img = LoadPNGFromResource(hInstance, IDB_PNG1);


        static HBITMAP hBitmap = CreateBitmapFromData(hdc, img.data, img.width, img.height, 4);

        HDC memDC = CreateCompatibleDC(hdc);
        HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, hBitmap);


        int x = (GetSystemMetrics(SM_CXSCREEN) - img.width) / 2;
        int y = (GetSystemMetrics(SM_CYSCREEN) - img.height) / 2;

        BitBlt(hdc, x, y, img.width, img.height, memDC, 0, 0, SRCCOPY);

        SelectObject(memDC, oldBitmap);
        DeleteDC(memDC);
        EndPaint(hwnd, &ps);

         EnableBlurBehind(hwnd);
        Sleep(555 * 3);

        ShakeWindow(hwnd);
        FadeWindow(hwnd, 253, 1, 1);
        ShowWindow(hwnd, SW_HIDE);

        ShowWindow(hwnd, SW_HIDE);
        Sleep(1);
       
        // BitBlt(hdc, x, y, img.width, img.height, memDC, 0, 0, SRCINVERT);
         //Sleep(66);
         
        //ExitProcess(0);
        // PostQuitMessage(0);
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

int begin() {
    const wchar_t CLASS_NAME[] = L"Splash";

    // Register Window Class
    WNDCLASSEX wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProcsplash;
    wcex.hInstance = GetModuleHandle(nullptr);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.lpszClassName = CLASS_NAME;

    if (!RegisterClassEx(&wcex)) {
        std::cerr << "Failed to register window class!" << std::endl;
        return -1;
    }

    // Create the window
    HWND hwnd = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST, // Transparent and topmost
        CLASS_NAME, L"Splash", WS_POPUP,       // Borderless
        0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
        nullptr, nullptr, GetModuleHandle(nullptr), nullptr);

    if (!hwnd) {
        std::cerr << "Failed to create window!" << std::endl;
        return -1;
    }

    // Make window transparent
    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);
    // SetLayeredWindowAttributes(hwnd, RGB(255, 127, 39), 0, LWA_COLORKEY);

     // Show the window and set it as fullscreen
    //SetWindowPos(hwnd, HWND_TOPMOST, -1, -1, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_SHOWWINDOW);

   ShowWindow(hwnd, SW_SHOW);
   
    UpdateWindow(hwnd);

    // Sleep(2000);

    return 0;
}