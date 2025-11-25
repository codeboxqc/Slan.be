


//NuGet\Install-Package Microsoft.Web.WebView2 -Version 1.0.3179.45

//NuGet\Install-Package Microsoft.Web.WebView2 -Version 1.0.3296-prerelease

#include <windows.h>
#include <wrl.h>
#include <wil/com.h>
#include <webview2.h>
 
#include <string>
#include <sstream>

#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

 
#include <WebView2EnvironmentOptions.h>
 
#include <vector>


#include <commctrl.h> // For Tab Contr


#pragma comment(lib,"comctl32.lib")

#include <gdiplus.h>
#pragma comment (lib, "gdiplus.lib")

#include <thread>  // For creating worker threads
#include <atomic>  // For atomic flags to signal completion



#include "Resource.h"

using namespace Microsoft::WRL;

extern int begin();

HCURSOR hCustomCursor = nullptr;


extern "C" {
    WINUSERAPI BOOL WINAPI SetLayeredWindowAttributes(HWND hwnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags);
}


 
#define WS_EX_LAYERED           0x00080000
#define LWA_COLORKEY            0x00000001
#define LWA_ALPHA               0x00000002

typedef struct _ACCENT_POLICY {
    int nAccentState;
    int nFlags;
    int nColor;
    int nAnimationId;
} ACCENT_POLICY;

// Define the WINDOWCOMPOSITIONATTRIBDATA structure
typedef struct _WINDOWCOMPOSITIONATTRIBDATA {
    int Attribute;
    PVOID Data;
    ULONG SizeOfData;
} WINDOWCOMPOSITIONATTRIBDATA;

// Define the function pointer type
typedef BOOL(WINAPI* pSetWindowCompositionAttribute)(
    HWND, WINDOWCOMPOSITIONATTRIBDATA*);

 


BOOL  Transparent(HWND hWnd, BYTE alpha) {
    // Validate input
    if (!IsWindow(hWnd)) {
        return FALSE;
    }

    // Check if WS_EX_LAYERED is already set
    LONG_PTR exStyle = GetWindowLongPtr(hWnd, GWL_EXSTYLE);
    if (!(exStyle & WS_EX_LAYERED)) {
        // Set layered style
        if (!SetWindowLongPtr(hWnd, GWL_EXSTYLE, exStyle | WS_EX_LAYERED)) {
            return FALSE;
        }
    }

    // Set transparency
    BOOL result = SetLayeredWindowAttributes(hWnd, 0, alpha, LWA_ALPHA);
    if (result) {
        // Trigger minimal repaint
        InvalidateRect(hWnd, NULL, FALSE);
        UpdateWindow(hWnd);
    }

    return result;
}


BOOL SetWindowAcrylic(HWND hWnd) {
    if (!IsWindow(hWnd)) return FALSE;

    ACCENT_POLICY policy = {
        4, // ACCENT_ENABLE_ACRYLICBLURBEHIND
        0,
        0xCC000000, // Semi-transparent black (ARGB, alpha=0xCC)
        0
    };

    WINDOWCOMPOSITIONATTRIBDATA data = {
        19, // WCA_ACCENT_POLICY
        &policy,
        sizeof(policy)
    };

    HMODULE hUser = GetModuleHandleA("user32.dll");
    if (hUser) {
        auto SetWindowCompositionAttribute = (pSetWindowCompositionAttribute)GetProcAddress(hUser, "SetWindowCompositionAttribute");
        if (SetWindowCompositionAttribute) {
            return SetWindowCompositionAttribute(hWnd, &data);
        }
    }
    return FALSE;
}


void SetWindowTransparency(HWND hWnd, BYTE alpha) {
    RECT rect;
    GetClientRect(hWnd, &rect);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    HDC hdcScreen = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
     
   BITMAPINFO bmi = { sizeof(BITMAPINFOHEADER), width, -height, 1, 32, (int)BI_RGB };
 


    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    void* pvBits;
    HBITMAP hBitmap = CreateDIBSection(hdcScreen, &bmi, DIB_RGB_COLORS, &pvBits, NULL, 0);
    SelectObject(hdcMem, hBitmap);

    // Fill bitmap with semi-transparent color
    memset(pvBits, alpha, width * height * 4); // Set alpha channel

    POINT ptSrc = { 0, 0 };
    SIZE sizeWnd = { width, height };
    BLENDFUNCTION blend = { AC_SRC_OVER, 0, alpha, AC_SRC_ALPHA };

    SetWindowLongPtr(hWnd, GWL_EXSTYLE, GetWindowLongPtr(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
    UpdateLayeredWindow(hWnd, hdcScreen, NULL, &sizeWnd, hdcMem, &ptSrc, 0, &blend, ULW_ALPHA);

    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);
}


///////////////////////////////////////////////////////////
// URLs and tab names
static struct {
    LPCWSTR url;
    LPCWSTR name;
} tabs[] = {
    { L"https://chat.openai.com", L"ChatGPT" },
    { L"https://grok.com/?referrer=website", L"Grok" },
    { L"https://gemini.google.com/", L"Gemini" },
    { L"https://copilot.microsoft.com/?form=REDIRERR", L"Copilot" },
    { L"https://chat.deepseek.com/", L"Deepseek" },
    { L"https://www.bing.com/images/create", L"Bing" },
    { L"https://www.phind.com/", L"Phind" },
    { L"https://claude.ai/", L"Claude" },
    { L"https://labs.perplexity.ai/", L"Perplexity" },
    { L"https://chat.qwen.ai/", L"Qwen" },
    { L"https://replit.com/~", L"Replit" },
    { L"https://www.kimi.com/", L"Kimi" },
    { L"https://www.meta.ai/", L"Meta AI" },
    { L"https://chat.mistral.ai/chat", L"Mistral" },
    { L"https://translate.google.com/", L"Translator" },
    { L"https://www.deepl.com/en/write", L"Deepl" },
    { L"https://jules.google.com/", L"Jules" },
    { L"https://sora.chatgpt.com/", L"Sora" },
    { L"https://aistudio.google.com/", L"AI Studio" },
    { L"https://www.blackbox.ai/", L"Blackbox" },
    // NEW AI SITES ADDED:
    { L"https://ernie.baidu.com/", L"ernie baidu" },
    { L"https://www.zerogpt.com/", L"zerogpt" },
    { L"https://www.trae.ai/", L"trae ban canada" }, 
    { L"https://chat.z.ai/", L"chatzai " },
    { L"https://app.base44.com/", L"base44" }, 
    { L"https://dashboard.cohere.com/playground/chat", L"Cohere" },
    { L"https://production.creao.ai/home", L"creao" }, 
    { L"https://bard.google.com/", L"Bard" }
};


const int TAB_COUNT = sizeof(tabs) / sizeof(tabs[0]);


// Define colors for each tab (RGB format)
COLORREF tabColors[TAB_COUNT] = {
    RGB(40, 40, 80),  // ChatGPT
    RGB(80, 80, 80),  // Grok
    RGB(80, 40, 80),  // Gemini
    RGB(80, 80, 80),  // Copilot
    RGB(40, 80, 80),  // Deepseek
    RGB(80, 80, 80),  // Bing
    RGB(80, 80, 40),  // Phind
    RGB(80, 80, 80),  // Claude
    RGB(40, 40, 80),  // Perplexity
    RGB(80, 80, 80),  // Qwen
    RGB(40, 40, 80),  // Replit
    RGB(80, 80, 80),  // Kimi
    RGB(90, 20, 80),  // Meta AI
    RGB(80, 80, 80),  // Mistral
    RGB(40, 40, 80),  // Translator
    RGB(80, 80, 80),  // Deepl
    RGB(80, 40, 80),  // Jules
    RGB(40, 80, 80),  // Sora
    RGB(80, 80, 80),  // AI Studio
    RGB(40, 40, 80),  // Blackbox
    // New colors for new sites:
    RGB(50, 90, 70),  // You.com
    RGB(70, 50, 90),  // Poe
    RGB(90, 70, 50),  // HuggingChat
    RGB(50, 70, 90),  // Pi
    RGB(90, 50, 70),  // Together
    RGB(70, 90, 50),  // Cohere
    RGB(60, 60, 90),  // Anthropic
    RGB(90, 60, 60)   // Bard
};



/////////////////////////////////////////////////////////

// Required struct definitions



//ACCENT_DISABLED = 0,
//ACCENT_ENABLE_GRADIENT = 1,
//ACCENT_ENABLE_TRANSPARENTGRADIENT = 2,
//ACCENT_ENABLE_BLURBEHIND = 3,
//ACCENT_ENABLE_ACRYLIC = 4,
//ACCENT_INVALID_STATE = 5

void EnableBlur(HWND hwnd) {
    // Set up blur policy (use 3 for blur, 4 for acrylic)
    ACCENT_POLICY policy = {
        3, // ACCENT_ENABLE_BLURBEHIND (use 4 for acrylic)
        0,
        0, // Optional ARGB color (0 = transparent)
        0
    };
 

    WINDOWCOMPOSITIONATTRIBDATA data = {
        19, // WCA_ACCENT_POLICY
        &policy,
        sizeof(policy)
    };

    // Load the undocumented function
    HMODULE hUser = GetModuleHandleA("user32.dll");
    if (hUser) {
        pSetWindowCompositionAttribute SetWindowCompositionAttribute =
            (pSetWindowCompositionAttribute)GetProcAddress(
                hUser, "SetWindowCompositionAttribute");

        if (SetWindowCompositionAttribute) {
            SetWindowCompositionAttribute(hwnd, &data);



            // Prepare tint color with alpha
            COLORREF finalColor = (66 << 24) | (RGB(6, 0, 99) & 0x00FFFFFF);

            ACCENT_POLICY policy = {
                3,           // Blur type
                2,                  // Flags (2 = enable tint color)
                (int) finalColor,         // Tint color with alpha
                0                   // Animation ID
            };

            WINDOWCOMPOSITIONATTRIBDATA data = {
                19,                 // WCA_ACCENT_POLICY
                &policy,
                sizeof(ACCENT_POLICY)
            };

            BOOL result = SetWindowCompositionAttribute(hwnd, &data);
        }
    }
}


 
 
    
 
///////////////////////////////////////////////////








extern "C" {
    WINUSERAPI BOOL WINAPI SetLayeredWindowAttributes(HWND hwnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags);
}
 
#define WS_EX_LAYERED           0x00080000
#define LWA_COLORKEY            0x00000001
#define LWA_ALPHA               0x00000002




int begin();





 



// Global WebView2 controller and webview
wil::com_ptr<ICoreWebView2Controller> WEBC[TAB_COUNT] = {}; // Initializes all elements to nullptr
wil::com_ptr<ICoreWebView2> WEB = nullptr; // Explicitly set to nullptr
 

///////////////////////////////////////////////



void InitWebView2(HWND hwnd, RECT bounds, LPCWSTR url, wil::com_ptr<ICoreWebView2Controller>& controller, wil::com_ptr<ICoreWebView2>& webView) {
    // Path to the WebView2 runtime or custom browser executable folder (optional)
    PCWSTR browserExecutableFolder = nullptr;  // Optional: Path to a custom browser executable
    PCWSTR userDataFolder = nullptr;           // Optional: Path to user data folder

    // Use an atomic flag to track the initialization status
    std::atomic<bool> isInitialized(false);

    // Additional WebView2 options for optimization and GPU acceleration (as command-line flags)
    PCWSTR options[] = {
        L"--enable-gpu-rasterization",          // Enable GPU rasterization
        L"--enable-threaded-compositing",       // Enable threaded compositing
        L"--enable-accelerated-2d-canvas",      // Enable accelerated 2D canvas
        L"--enable-quic",                       // Enable QUIC protocol
        L"--enable-fast-unload",               // Enable faster unload of resources
        L"--enable-webgl",                      // Enable WebGL for better 3D rendering
        L"--enable-webgl-draft-extensions",
        L"--enable-accelerated-ssl",
        L"--force-device-scale-factor=1",
        L"--enable-features=DarkMode",
        L"--enable-background-video-track",     // Enable background video track
        L"--disable-background-timer-throttling", // Disable background timer throttling
        L"--disable-features=IsolateOrigins,SitePerProcess",  // Disable certain features


        // L"--disable-extensions ",  // Disable unnecessary extensions for faster startup
        // L"--enable-zero-copy ",     // Enable zero-copy GPU textures for faster rendering
        // L"--no-sandbox " ,         // Disable sandbox for speed (use cautiously, reduces security)
      //  L"--disable-dev-shm-usage ",  // Avoid shared memory issues on some systems
         L"--enable-accelerated-video-decode "   // Hardware-accelerated video for media-heavy sites
    };

    // Create WebView2 environment asynchronously with options
    CreateCoreWebView2EnvironmentWithOptions(
        browserExecutableFolder,              // Specify the browser executable folder (if needed)
        userDataFolder,                       // Specify the user data folder (if needed)
        nullptr,                              // Default options for environment
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [hwnd, bounds, url, &controller, &webView, options, &isInitialized](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
                if (FAILED(result)) {
                    MessageBox(hwnd, L"Failed to create WebView2 environment.", L"Error", MB_OK | MB_ICONERROR);
                    controller = nullptr;
                    return E_FAIL;
                }

                // Create WebView2 controller asynchronously
                env->CreateCoreWebView2Controller(hwnd, Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                    [hwnd, bounds, url, &controller, &webView, &isInitialized](HRESULT result, ICoreWebView2Controller* newController) -> HRESULT {
                        if (FAILED(result)) {
                            MessageBox(hwnd, L"Failed to create WebView2 controller.", L"Error", MB_OK | MB_ICONERROR);
                            controller = nullptr;
                            return E_FAIL;
                        }

                        if (newController != nullptr) {
                            controller = newController;
                            controller->get_CoreWebView2(&webView);

                            // Set bounds and security settings
                            controller->put_Bounds(bounds);

                            wil::com_ptr<ICoreWebView2Settings> settings;
                            webView->get_Settings(&settings);

                            // Set WebView2 settings (disable dialogs, etc.)
                            settings->put_IsWebMessageEnabled(TRUE);
                            /*
                            // Inject JavaScript to handle right-click context menu for images and videos
                            const wchar_t* script = LR"(
                                document.addEventListener('contextmenu', function(e) {
                                    if (e.target.tagName === 'VIDEO') {
                                        e.preventDefault(); // Prevent the default context menu for videos
                                        const videoSrc = e.target.src;
                                        const a = document.createElement('a');
                                        a.href = videoSrc;
                                        a.download = 'video.mp4'; // Specify the default video file name
                                        a.click(); // Trigger the download
                                    } else if (e.target.tagName === 'IMG') {
                                        e.preventDefault(); // Prevent the default context menu for images
                                        const imgSrc = e.target.src;
                                        const a = document.createElement('a');
                                        a.href = imgSrc;
                                        a.download = 'image.jpg'; // Specify the default image file name
                                        a.click(); // Trigger the download
                                    }
                                });
                            )";
                            */
                            // Enhanced JavaScript injection
                            const wchar_t* script = LR"(
                            document.addEventListener('contextmenu', function(e) {
                                if (e.target.tagName === 'VIDEO') {
                                    e.preventDefault();
                                    const videoSrc = e.target.src;
                                    const a = document.createElement('a');
                                    a.href = videoSrc;
                                    a.download = 'video.mp4';
                                    a.click();
                                } else if (e.target.tagName === 'IMG') {
                                    e.preventDefault();
                                    const imgSrc = e.target.src;
                                    const a = document.createElement('a');
                                    a.href = imgSrc;
                                    a.download = 'image.jpg';
                                    a.click();
                                }
                            });
                        )";

                            // Execute the script to add the right-click event listener for videos and images
                            webView->ExecuteScript(script, nullptr);

                            // Ensure only HTTPS URLs are allowed
                            if (wcsstr(url, L"https://") == nullptr) {
                                MessageBox(hwnd, L"Only HTTPS URLs are allowed.", L"Error", MB_OK | MB_ICONERROR);
                                return E_FAIL;
                            }

                            // Navigate to the given URL
                            webView->Navigate(url);

                            // Add navigation completed handler
                            webView->add_NavigationCompleted(
                                Callback<ICoreWebView2NavigationCompletedEventHandler>(
                                    [&isInitialized](ICoreWebView2* sender, ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT {
                                        isInitialized.store(true); // Mark as initialized only after navigation completes
                                        return S_OK;
                                    }).Get(),
                                        nullptr);



                            // Set the initialization flag to true after the WebView2 is ready
                            isInitialized.store(true);
                        }
                        return S_OK;
                    }).Get());

                return S_OK;
            }).Get());

    // Event loop to process messages and check if initialization is done
    MSG msg = {};
    while (!isInitialized.load()) {
        // Non-blocking GetMessage with a longer sleep duration (e.g., 100ms)
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));  // Sleep for a longer interval
    }
}

 






// Function to handle search
void slan() {

    //MessageBox(NULL, L"slan() called!", L"Debug", MB_OK);


    HINSTANCE result = ShellExecute(NULL, L"open", L"http://www.slan.be", NULL, NULL, SW_SHOWNORMAL);

    //if (webView1) webView1->Navigate(L"https://www.Slan.be/right");
    //if (webView2) webView2->Navigate(L"https://www.Slan.be/left");
}





void ClearCacheAndCookiesForWebView(wil::com_ptr<ICoreWebView2> webView) {
    if (webView) {
        // Clear cache
        webView->CallDevToolsProtocolMethod(
            L"Network.clearBrowserCache", L"{}",
            Callback<ICoreWebView2CallDevToolsProtocolMethodCompletedHandler>(
                [](HRESULT error, LPCWSTR jsonResult) -> HRESULT {
                    if (FAILED(error)) {
                        // Handle error (e.g., log or ignore)
                    }
                    return S_OK;
                }).Get());
    }
}







//Clear caches for all WebViews
// Call this when the application is closing
void OnAppClose() {
    for (int i = 0; i < TAB_COUNT; i++) {
        if (WEBC[i]) {
            wil::com_ptr<ICoreWebView2> webView;
            WEBC[i]->get_CoreWebView2(&webView);
            ClearCacheAndCookiesForWebView(webView);
            WEBC[i]->Close();
            WEBC[i].reset();
        }
    }
}

// Function to create main buttons
void CreateMainButtons(HWND hParent, HINSTANCE hInstance, HWND& Slan, HWND& Home, HWND& Back) {
    int buttonWidth = 60;
    int buttonHeight = 25;
    int buttonSpacing = 5;

    // barHeight and tabHeight are assumed to be 40 each for positioning calculations.
    // These were the values used for tabControl and general UI layout aound it.
    int barHeight = 40;
    int tabHeight = 40; // As per instruction for consistency, WM_SIZE uses 40.

    RECT rect;
    GetClientRect(hParent, &rect);

    int totalButtonsWidth = (3 * buttonWidth) + (2 * buttonSpacing);
    int startX = (rect.right - rect.left - totalButtonsWidth) / 2;
    int startY = barHeight + tabHeight + buttonSpacing; // e.g., 40 + 40 + 5 = 85

    // Create About button
    Slan = CreateWindow(L"BUTTON", L"About",
        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, // Added BS_OWNERDRAW
        startX, startY, buttonWidth, buttonHeight,
        hParent, (HMENU)3, hInstance, NULL);

    // Create Home button
    Home = CreateWindow(L"BUTTON", L"Home",
        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, // Added BS_OWNERDRAW
        startX + buttonWidth + buttonSpacing, startY, buttonWidth, buttonHeight,
        hParent, (HMENU)4, hInstance, NULL);

    // Create Back button
    Back = CreateWindow(L"BUTTON", L"Back",
        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, // Added BS_OWNERDRAW
        startX + (2 * buttonWidth) + (2 * buttonSpacing), startY, buttonWidth, buttonHeight,
        hParent, (HMENU)5, hInstance, NULL);

    EnableBlur(Slan);
    EnableBlur(Home);
    EnableBlur(Back);

}



LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND   Slan = nullptr, tabControl = nullptr;
    static HWND Home = nullptr, Back = nullptr;
    static HBRUSH hTabBrush = nullptr;
    static HBRUSH hButtonBrush = nullptr; // Declare new brush for buttons

    static int currentTab = 0;
   


    switch (uMsg) {


    case WM_CREATE: {
        // Initialize common controls for tabs
        INITCOMMONCONTROLSEX icex = { sizeof(INITCOMMONCONTROLSEX), ICC_TAB_CLASSES };
        InitCommonControlsEx(&icex);

        

        // Define initial dimensions
        int inputWidth = 800, buttonWidth = 60, elementHeight = 60;
        // int inputWidth = 800, buttonWidth = 60, elementHeight = 60; // Commented out as per plan, will be handled or passed if necessary
        // int totalWidth = inputWidth + buttonWidth + 10; 
        // int centerX = 50; 
        //*
        // Create input box with initial position and size


      // Create brush for tab control background
        hTabBrush = CreateSolidBrush(RGB(40, 40, 40)); // Dark gray for tab background
        hButtonBrush = CreateSolidBrush(RGB(80, 80, 80)); // Create brush for button background

        // Create main buttons by calling the new function
        CreateMainButtons(hwnd, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), Slan, Home, Back);


        tabControl = CreateWindow(WC_TABCONTROL, L"",
            WS_CHILD | WS_VISIBLE | TCS_TABS | TCS_MULTILINE | TCS_OWNERDRAWFIXED,
            0, 40, 0, 0, hwnd, NULL, GetModuleHandle(NULL), NULL);





        TCITEM tie = { TCIF_TEXT };
        for (int i = 0; i < TAB_COUNT; i++) {

            tie.pszText = (LPWSTR)tabs[i].name;
            TabCtrl_InsertItem(tabControl, i, &tie);
        }

        SendMessage(tabControl, TCM_SETMINTABWIDTH, 0, 80);
        TabCtrl_SetItemSize(tabControl, 80, 20);

       EnableBlur(tabControl);

     

        hCustomCursor = LoadCursor(GetModuleHandle(NULL), MAKEINTRESOURCE(IDC_CURSOR1));
        SetFocus(tabControl);

        // Initialize WebView2 instances (hidden initially except for the first tab)
        RECT clientRect; // Renamed from bounds to avoid conflict with loop's bounds
        GetClientRect(hwnd, &clientRect);

        // Define heights and spacing relevant for layout
        const int barHeight = 40;     // Height of the area above tabs
        const int tabCtrlHeight = 40; // Height of the tab control itself (consistent with WM_SIZE)
        const int btnHeight = 25;     // Height of the buttons
        const int btnSpacing = 5;     // Spacing around buttons

        // Calculate Y position for the top of WebViews
        // Buttons are at startY = barHeight + tabCtrlHeight + btnSpacing
        // WebViews are below buttons: (barHeight + tabCtrlHeight + btnSpacing) + btnHeight + btnSpacing
        int webViewTopY = barHeight + tabCtrlHeight + btnSpacing + btnHeight + btnSpacing; // e.g., 40 + 40 + 5 + 25 + 5 = 115

        // RECT webBounds = { bounds.left, bounds.top + barHeight + tabHeight, bounds.right, bounds.bottom }; // Old calculation


      

        for (  int i = 0; i < TAB_COUNT; ++i) {
            RECT webViewBounds; // Use a distinct RECT for each WebView's initial bounds
            GetClientRect(hwnd, &webViewBounds); // Get the full client area
            webViewBounds.top = webViewTopY; // Set the calculated top position
            InitWebView2(hwnd, webViewBounds, tabs[i].url, WEBC[i], WEB);  // Pass corresponding pointers
            if (i != 0 && WEBC[i]) {
                WEBC[i]->put_IsVisible(FALSE); // Hide all but the first tab
            }
            else  WEBC[0]->put_IsVisible(TRUE);
        }


       

        //SetWindowTransparency(hwnd, 250);
        //Transparency(hwnd, 250);
        //SetWindowAcrylic(hwnd);


        return 0;
    }


    case WM_NCHITTEST: {
        // Handle non-client hit testing to enable resizing and show resize cursors
        LRESULT hit = DefWindowProc(hwnd, uMsg, wParam, lParam);
        if (hit == HTCLIENT) {
            POINT pt = { LOWORD(lParam), HIWORD(lParam) };
            ScreenToClient(hwnd, &pt);
            RECT rc;
            GetClientRect(hwnd, &rc);
            // Detect if mouse is within 5 pixels of the borders for resizing
            if (pt.x < 5) {
                if (pt.y < 5) return HTTOPLEFT;
                if (pt.y > rc.bottom - 5) return HTBOTTOMLEFT;
                return HTLEFT;
            }
            if (pt.x > rc.right - 5) {
                if (pt.y < 5) return HTTOPRIGHT;
                if (pt.y > rc.bottom - 5) return HTBOTTOMRIGHT;
                return HTRIGHT;
            }
            if (pt.y < 5) return HTTOP;
            if (pt.y > rc.bottom - 5) return HTBOTTOM;
        }
        return hit;
    }

    case WM_SETCURSOR: {
        // Only set custom cursor in client area; allow default cursors (e.g., resize) elsewhere
        if (hCustomCursor && LOWORD(lParam) == HTCLIENT) {
            SetCursor(hCustomCursor);
            return TRUE;
        }
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

 
    case WM_ERASEBKGND: {
        HDC hdc = (HDC)wParam;
        RECT rect;
        GetClientRect(hwnd, &rect);
        FillRect(hdc, &rect, hTabBrush); // hTabBrush is RGB(40, 40, 40)
        return 1;
    }


    case WM_CTLCOLORSTATIC: {
        HDC hdcStatic = (HDC)wParam;
        HWND hwndStatic = (HWND)lParam;

        if (hwndStatic == tabControl) {
            SetBkColor(hdcStatic, RGB(40, 40, 40));
            SetTextColor(hdcStatic, RGB(255, 255, 255));
            return (LRESULT)hTabBrush;
        }
        SetBkColor(hdcStatic, RGB(23, 23, 23));
        return (LRESULT)hTabBrush;
    }

    case WM_CTLCOLORBTN: {
        HDC hdc = (HDC)wParam;
        SetTextColor(hdc, RGB(0, 0, 0));         // Black text
        SetBkColor(hdc, RGB(80, 80, 80));       // Professional gray background
        return (LRESULT)hButtonBrush;           // Return the new button brush
    }
    case WM_CTLCOLORDLG:
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX:
    case WM_CTLCOLORSCROLLBAR: {
        HDC hdc = (HDC)wParam;
        SetBkColor(hdc, RGB(23, 23, 23)); // Keep original for other controls if needed
        return (LRESULT)hTabBrush;       // Still use hTabBrush for these
    }

                             /////////////////////////////////////////////
    case WM_DRAWITEM: {
        LPDRAWITEMSTRUCT pDrawItem = (LPDRAWITEMSTRUCT)lParam;

        if (pDrawItem->CtlType == ODT_BUTTON) {
            wchar_t buttonText[256];
            GetWindowText(pDrawItem->hwndItem, buttonText, sizeof(buttonText) / sizeof(buttonText[0]));

            // Fill background
            // hButtonBrush should be initialized in WM_CREATE to CreateSolidBrush(RGB(80, 80, 80))
            FillRect(pDrawItem->hDC, &pDrawItem->rcItem, hButtonBrush);

            // Draw text
            SetTextColor(pDrawItem->hDC, RGB(0, 0, 0)); // Black text
            SetBkMode(pDrawItem->hDC, TRANSPARENT);
            DrawText(pDrawItem->hDC, buttonText, -1, &pDrawItem->rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            // Handle pressed state
            if (pDrawItem->itemState & ODS_SELECTED) {
                DrawEdge(pDrawItem->hDC, &pDrawItem->rcItem, EDGE_SUNKEN, BF_RECT);
            }
            // Handle focus state (only if not pressed)
            else if (pDrawItem->itemState & ODS_FOCUS) {
                DrawFocusRect(pDrawItem->hDC, &pDrawItem->rcItem);
            }
            return TRUE; // We handled the drawing for the button
        }
        else if (pDrawItem->CtlType == ODT_TAB) {
            // Get the tab index
            int tabIndex = pDrawItem->itemID;

            // Choose a color based on the tab index
            COLORREF tabColor = tabColors[tabIndex % TAB_COUNT]; // Use modulo to avoid out-of-bounds

            // Fill the tab background
            HBRUSH hTabItemBrush = CreateSolidBrush(tabColor); // Renamed to avoid conflict
            FillRect(pDrawItem->hDC, &pDrawItem->rcItem, hTabItemBrush);

            // Draw the tab text
            SetBkMode(pDrawItem->hDC, TRANSPARENT);
            SetTextColor(pDrawItem->hDC, RGB(0, 0, 0)); // Black text (adjust if needed)
            DrawText(pDrawItem->hDC, tabs[tabIndex].name, -1,
                &pDrawItem->rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            // Cleanup
            DeleteObject(hTabItemBrush); // Use the renamed brush
            return TRUE; // We handled the drawing for the tab
        }
        break;
    }
                    ///////////////////////////////////////////////////

    case WM_SYSCOLORCHANGE: {
        if (hTabBrush) DeleteObject(hTabBrush);

        hTabBrush = CreateSolidBrush(RGB(40, 40, 40));

        InvalidateRect(hwnd, NULL, TRUE);
        return 0;
    }


    case WM_GETMINMAXINFO: {
        LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
        lpMMI->ptMinTrackSize.x = 1200; // Minimum width
        lpMMI->ptMinTrackSize.y = 600;  // Minimum height
        return 0;
    }


    case WM_SIZE: {
        RECT bounds;
        GetClientRect(hwnd, &bounds);

        int barHeight = 1; // Height of the top bar area before tabs
        int tabHeight = 40; // Height of the tab control itself

        // Define button properties
        int btnWidth = 60;    // Uniform width for buttons
        int btnHeight = 25;   // Uniform height for buttons
        int btnSpacing = 5;   // Spacing between buttons

        // Calculate total width required for all buttons and spacing
        int totalButtonsWidth = (3 * btnWidth) + (2 * btnSpacing);

        // Calculate starting X position to center the buttons
        int startX = (bounds.right - bounds.left - totalButtonsWidth) / 2;

        // Calculate starting Y position for buttons (below tab control)
        int startY = barHeight + tabHeight + btnSpacing; // e.g., 40 + 40 + 5 = 85

        // Reposition About, Home, and Back buttons
        MoveWindow(Slan, startX, startY, btnWidth, btnHeight, TRUE);
        MoveWindow(Home, startX + btnWidth + btnSpacing, startY, btnWidth, btnHeight, TRUE);
        MoveWindow(Back, startX + (2 * btnWidth) + (2 * btnSpacing), startY, btnWidth, btnHeight, TRUE);

        // Transparent(Slan, 250); // Remove transparency for Slan button

        // Position tab control
        MoveWindow(tabControl, 0, barHeight, bounds.right, tabHeight, TRUE);

        // Adjust WebView2 bounds to be below the buttons
        // RECT webBounds = { bounds.left, bounds.top + barHeight + tabHeight, bounds.right, bounds.bottom }; // Old calculation

        // New top for WebView, considering buttons are now below tabs
        bounds.top = startY + btnHeight + btnSpacing; // e.g., 85 + 25 + 5 = 115

        for (int i = 0; i < TAB_COUNT; ++i) {
            if (WEBC[i]) {
                WEBC[i]->put_Bounds(bounds); // Apply new bounds with adjusted top
            }
        }




        return 0;
    }

     
    case WM_NOTIFY: {
        if (((LPNMHDR)lParam)->hwndFrom == tabControl && ((LPNMHDR)lParam)->code == TCN_SELCHANGE) {
            int selectedTab = TabCtrl_GetCurSel(tabControl);
            if (selectedTab != currentTab) {
                if (WEBC[currentTab]) {
                    WEBC[currentTab]->put_IsVisible(FALSE);
                }
                if (WEBC[selectedTab]) {
                    WEBC[selectedTab]->put_IsVisible(TRUE);
                }
                currentTab = selectedTab;
            }
        }
        break;
    }


    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case 3: // About
            slan();
            break;
       
        case 4: // Home
            if (WEBC[currentTab]) {
                wil::com_ptr<ICoreWebView2> webView;
                WEBC[currentTab]->get_CoreWebView2(&webView);
                if (webView && currentTab < TAB_COUNT) {
                    webView->Navigate(tabs[currentTab].url);
                }
            }
            break;
        case 5: // Back
            if (WEBC[currentTab]) {
                wil::com_ptr<ICoreWebView2> webView;
                WEBC[currentTab]->get_CoreWebView2(&webView);
                if (webView) {
                    webView->GoBack();
                }
            }
            break;


        }
        return 0;
    }

 

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        HBRUSH hBrush = CreateSolidBrush(RGB(13, 13, 13));
        FillRect(hdc, &ps.rcPaint, hBrush);
        DeleteObject(hBrush);
        EndPaint(hwnd, &ps);
        return 0;
    }
    

 

    case WM_DESTROY:


        OnAppClose();

  

        // Cleanup brushes
        if (hTabBrush) {
            DeleteObject(hTabBrush);
            hTabBrush = nullptr;
        }
        if (hButtonBrush) { // Cleanup new button brush
            DeleteObject(hButtonBrush);
            hButtonBrush = nullptr;
        }

        // Cleanup cursor
        if (hCustomCursor) {
            DestroyCursor(hCustomCursor);
            hCustomCursor = nullptr;
        }

        CoUninitialize();

        PostQuitMessage(0);
        return 0;

        // Add other cases (WM_CTLCOLORBTN, WM_CTLCOLOREDIT, etc.) as needed from your original code
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}





int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"IA";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;


  
    // Create named mutexes to allow maximum 2 instances
    HANDLE hMutex1 = CreateMutex(NULL, TRUE, L"slan1");
    HANDLE hMutex2 = NULL;

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        // First instance exists, try second mutex
        CloseHandle(hMutex1);
        hMutex2 = CreateMutex(NULL, TRUE, L"slan2");

        if (GetLastError() == ERROR_ALREADY_EXISTS) {
            // Both instances are already running, so exit
            CloseHandle(hMutex2);
            MessageBox(NULL, L"Maximum 2 instances already running!", L"Info", MB_OK | MB_ICONINFORMATION);
            return 0;
        }
    }

    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);


    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    /////////////////////////////////////////////
    std::thread splashThread([]() {
        begin();
        });
    splashThread.detach();  // Detach so it runs independently
    //////////////////////////////////////////

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, L"IA",
        WS_OVERLAPPEDWINDOW | WS_THICKFRAME,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, hInstance, NULL
    );

    if (hwnd == NULL) return 0;


    // Transparent(hwnd, 250);
   // SetClassLong(hwnd, GCL_STYLE, GetClassLong(hwnd, GCL_STYLE) | CS_DROPSHADOW);



    // Enable dark mode for the title bar
    // DWMWA_USE_IMMERSIVE_DARK_MODE is 20 in newer SDKs (typically after Windows 10 1809)
    // For older SDKs it might be 19. We'll use 20.
   // BOOL useDarkMode = TRUE;
   // HRESULT hr = DwmSetWindowAttribute(
    //    hwnd,
    //    20, // DWMWA_USE_IMMERSIVE_DARK_MODE (actual constant might not be available in older SDKs, so using the value directly)
   //     &useDarkMode,
  //      sizeof(useDarkMode)
   // );

   // Enable Dark Mode
    BOOL useDarkMode = TRUE;
    DwmSetWindowAttribute(hwnd, 20, &useDarkMode, sizeof(useDarkMode));

    // Enable Blur
    EnableBlur(hwnd); // See below for updated version
   

    // Only extend small margin to allow border glow
    MARGINS margins = { 1, 1, 1, 1 }; // Thin margins just for glow
    DwmExtendFrameIntoClientArea(hwnd, &margins);




    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);



    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }


    Gdiplus::GdiplusShutdown(gdiplusToken);


    return 0;
}

 