/**
 * DPI Awareness Functional Tests
 * Using Catch2 framework
 */

#include <catch2/catch_all.hpp>
#include <windows.h>
#include <tlhelp32.h>
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
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return false;

    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(pe);
    bool found = false;
    if (Process32FirstW(snapshot, &pe)) {
        do {
            if (_wcsicmp(pe.szExeFile, L"Winsplit.exe") == 0) {
                found = true;
                break;
            }
        } while (Process32NextW(snapshot, &pe));
    }
    CloseHandle(snapshot);
    return found;
}

HWND FindHotkeysManagerWindow() {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return nullptr;

    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(pe);
    DWORD pid = 0;
    if (Process32FirstW(snapshot, &pe)) {
        do {
            if (_wcsicmp(pe.szExeFile, L"Winsplit.exe") == 0) {
                pid = pe.th32ProcessID;
                break;
            }
        } while (Process32NextW(snapshot, &pe));
    }
    CloseHandle(snapshot);
    if (!pid) return nullptr;

    struct EnumData { DWORD pid; HWND result; };
    EnumData data = { pid, nullptr };

    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
        auto* d = reinterpret_cast<EnumData*>(lParam);
        DWORD windowPid = 0;
        GetWindowThreadProcessId(hwnd, &windowPid);
        if (windowPid != d->pid) return TRUE;
        LONG exStyle = GetWindowLongW(hwnd, GWL_EXSTYLE);
        if (exStyle & WS_EX_TOOLWINDOW) return TRUE;
        wchar_t className[64] = {};
        GetClassNameW(hwnd, className, 64);
        if (wcsstr(className, L"wxWindow")) {
            d->result = hwnd;
            return FALSE;
        }
        return TRUE;
    }, reinterpret_cast<LPARAM>(&data));

    return data.result;
}

int VkToHotkeyId(WORD vk) {
    if (vk >= VK_NUMPAD0 && vk <= VK_NUMPAD9)
        return 100 + (vk - VK_NUMPAD0);
    return -1;
}

void SimulateHotkey(WORD mod, WORD vk) {
    static HWND hkWnd = FindHotkeysManagerWindow();
    if (hkWnd) {
        int id = VkToHotkeyId(vk);
        if (id >= 0) {
            PostMessageW(hkWnd, WM_HOTKEY, (WPARAM)id, MAKELPARAM(mod, vk));
            Sleep(300);
            return;
        }
    }
    // Fallback: SendInput
    INPUT inputs[4] = {};
    int count = 0;
    if (mod & MOD_CONTROL) { inputs[count].type = INPUT_KEYBOARD; inputs[count].ki.wVk = VK_CONTROL; count++; }
    if (mod & MOD_ALT) { inputs[count].type = INPUT_KEYBOARD; inputs[count].ki.wVk = VK_MENU; count++; }
    inputs[count].type = INPUT_KEYBOARD; inputs[count].ki.wVk = vk; count++;
    SendInput(count, inputs, sizeof(INPUT));
    Sleep(50);
    for (int i = 0; i < count; i++) inputs[i].ki.dwFlags = KEYEVENTF_KEYUP;
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

TEST_CASE("Window Positioning at Current DPI", "[functional][dpi][.interactive]") {
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
