/**
 * DPI Awareness Functional Tests
 * Using Catch2 framework
 */

#include <catch2/catch_all.hpp>
#include <windows.h>
#include <dwmapi.h>
#include <shellscalingapi.h>

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "shcore.lib")

namespace {

typedef UINT (WINAPI *GetDpiForWindow_t)(HWND);
typedef UINT (WINAPI *GetDpiForSystem_t)();

GetDpiForWindow_t pGetDpiForWindow = nullptr;
GetDpiForSystem_t pGetDpiForSystem = nullptr;

void LoadDpiFunctions() {
    static bool loaded = false;
    if (!loaded) {
        HMODULE hUser32 = GetModuleHandle(L"user32.dll");
        if (hUser32) {
            pGetDpiForWindow = (GetDpiForWindow_t)GetProcAddress(hUser32, "GetDpiForWindow");
            pGetDpiForSystem = (GetDpiForSystem_t)GetProcAddress(hUser32, "GetDpiForSystem");
        }
        loaded = true;
    }
}

UINT GetSystemDpi() {
    LoadDpiFunctions();
    if (pGetDpiForSystem) {
        return pGetDpiForSystem();
    }
    HDC hdc = GetDC(nullptr);
    UINT dpi = GetDeviceCaps(hdc, LOGPIXELSX);
    ReleaseDC(nullptr, hdc);
    return dpi;
}

HWND CreateTestWindow(int x, int y, int w, int h) {
    static bool registered = false;
    static const wchar_t* className = L"Catch2DpiTest";

    if (!registered) {
        WNDCLASS wc = {};
        wc.lpfnWndProc = DefWindowProc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.lpszClassName = className;
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        RegisterClass(&wc);
        registered = true;
    }

    return CreateWindowEx(0, className, L"DPI Test",
                          WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                          x, y, w, h, nullptr, nullptr,
                          GetModuleHandle(nullptr), nullptr);
}

RECT GetVisibleRect(HWND hwnd) {
    RECT rect;
    DwmGetWindowAttribute(hwnd, DWMWA_EXTENDED_FRAME_BOUNDS, &rect, sizeof(rect));
    return rect;
}

RECT GetPrimaryWorkArea() {
    RECT rc;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, 0);
    return rc;
}

bool IsWinSplitRunning() {
    return FindWindow(nullptr, L"WinSplit Revolution - Hook Frame") != nullptr;
}

void SimulateHotkey(WORD mod, WORD vk) {
    INPUT inputs[4] = {};
    int count = 0;

    if (mod & MOD_CONTROL) {
        inputs[count].type = INPUT_KEYBOARD;
        inputs[count].ki.wVk = VK_CONTROL;
        count++;
    }
    if (mod & MOD_ALT) {
        inputs[count].type = INPUT_KEYBOARD;
        inputs[count].ki.wVk = VK_MENU;
        count++;
    }

    inputs[count].type = INPUT_KEYBOARD;
    inputs[count].ki.wVk = vk;
    count++;

    SendInput(count, inputs, sizeof(INPUT));
    Sleep(50);

    for (int i = 0; i < count; i++) {
        inputs[i].ki.dwFlags = KEYEVENTF_KEYUP;
    }
    SendInput(count, inputs, sizeof(INPUT));
    Sleep(150);
}

}  // namespace

TEST_CASE("DPI Detection", "[functional][dpi]") {
    LoadDpiFunctions();

    SECTION("System DPI is detectable") {
        UINT dpi = GetSystemDpi();
        CHECK(dpi >= 96);   // Minimum DPI
        CHECK(dpi <= 384);  // 400% scaling max

        INFO("System DPI: " << dpi << " (" << (dpi * 100 / 96) << "%)");
    }

    SECTION("DPI functions available on Windows 10+") {
        // These functions are only available on Windows 10+
        OSVERSIONINFOEXW osvi = {};
        osvi.dwOSVersionInfoSize = sizeof(osvi);

        typedef NTSTATUS (WINAPI *RtlGetVersion_t)(PRTL_OSVERSIONINFOW);
        HMODULE hNtdll = GetModuleHandle(L"ntdll.dll");
        RtlGetVersion_t pRtlGetVersion = (RtlGetVersion_t)GetProcAddress(hNtdll, "RtlGetVersion");

        if (pRtlGetVersion) {
            pRtlGetVersion((PRTL_OSVERSIONINFOW)&osvi);
            if (osvi.dwMajorVersion >= 10) {
                CHECK(pGetDpiForWindow != nullptr);
                CHECK(pGetDpiForSystem != nullptr);
            }
        }
    }
}

TEST_CASE("Window Positioning at Current DPI", "[functional][dpi]") {
    if (!IsWinSplitRunning()) {
        SKIP("WinSplit not running");
    }

    UINT dpi = GetSystemDpi();
    INFO("Testing at " << dpi << " DPI (" << (dpi * 100 / 96) << "% scaling)");

    RECT workArea = GetPrimaryWorkArea();
    int workW = workArea.right - workArea.left;
    int workH = workArea.bottom - workArea.top;
    const int tolerance = 10;

    SECTION("Maximize fills work area at current DPI") {
        HWND hwnd = CreateTestWindow(
            workArea.left + 100,
            workArea.top + 100,
            400, 300
        );
        REQUIRE(hwnd != nullptr);

        SetForegroundWindow(hwnd);
        Sleep(100);

        SimulateHotkey(MOD_CONTROL | MOD_ALT, VK_NUMPAD5);

        RECT vis = GetVisibleRect(hwnd);

        CHECK(std::abs(vis.left - workArea.left) <= tolerance);
        CHECK(std::abs(vis.top - workArea.top) <= tolerance);
        CHECK(std::abs(vis.right - workArea.right) <= tolerance);
        CHECK(std::abs(vis.bottom - workArea.bottom) <= tolerance);

        DestroyWindow(hwnd);
    }

    SECTION("Half-screen positions are pixel-accurate") {
        HWND hwnd = CreateTestWindow(
            workArea.left + 100,
            workArea.top + 100,
            400, 300
        );
        REQUIRE(hwnd != nullptr);

        SetForegroundWindow(hwnd);
        Sleep(100);

        SimulateHotkey(MOD_CONTROL | MOD_ALT, VK_NUMPAD4);  // Left half

        RECT vis = GetVisibleRect(hwnd);
        int visW = vis.right - vis.left;

        int expectedWidth = workW / 2;
        CHECK(std::abs(visW - expectedWidth) <= tolerance);

        DestroyWindow(hwnd);
    }
}
