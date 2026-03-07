/**
 * Window Positioning Functional Tests
 * Using Catch2 framework
 */

#include <catch2/catch_all.hpp>
#include <windows.h>
#include <tlhelp32.h>
#include <dwmapi.h>

#pragma comment(lib, "dwmapi.lib")

namespace {

HWND CreateTestWindow(int x, int y, int w, int h) {
    static bool registered = false;
    static const wchar_t* className = L"Catch2TestWindow";

    if (!registered) {
        WNDCLASS wc = {};
        wc.lpfnWndProc = DefWindowProc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.lpszClassName = className;
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        RegisterClass(&wc);
        registered = true;
    }

    HWND hwnd = CreateWindowEx(0, className, L"Test Window",
                                WS_OVERLAPPEDWINDOW,
                                x, y, w, h, nullptr, nullptr,
                                GetModuleHandle(nullptr), nullptr);
    if (hwnd) {
        ShowWindow(hwnd, SW_SHOW);
        UpdateWindow(hwnd);
    }
    return hwnd;
}

RECT GetVisibleRect(HWND hwnd) {
    RECT rect;
    DwmGetWindowAttribute(hwnd, DWMWA_EXTENDED_FRAME_BOUNDS, &rect, sizeof(rect));
    return rect;
}

// Find WinSplit's HotkeysManager window (a wxFrame that handles WM_HOTKEY)
HWND FindHotkeysManagerWindow() {
    // Find WinSplit process
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

    // Enumerate windows to find the HotkeysManager (wxWindowNR, not tool window)
    struct EnumData { DWORD pid; HWND result; };
    EnumData data = { pid, nullptr };

    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
        auto* d = reinterpret_cast<EnumData*>(lParam);
        DWORD windowPid = 0;
        GetWindowThreadProcessId(hwnd, &windowPid);
        if (windowPid != d->pid) return TRUE;

        // HotkeysManager is a non-tool-window wxFrame (unlike FrameHook which is WS_EX_TOOLWINDOW)
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

// WinSplit hotkey IDs start at HK_0 = 100
// Mapping: VK_NUMPAD0 -> HK_0(100), VK_NUMPAD1 -> HK_1(101), ...
int VkToHotkeyId(WORD vk) {
    if (vk >= VK_NUMPAD0 && vk <= VK_NUMPAD9)
        return 100 + (vk - VK_NUMPAD0);
    return -1;
}

void SimulateHotkey(WORD mod, WORD vk) {
    // Post WM_HOTKEY directly to WinSplit's HotkeysManager window.
    // SendInput doesn't reliably trigger RegisterHotKey from non-interactive processes.
    static HWND hkWnd = FindHotkeysManagerWindow();
    if (!hkWnd) {
        // Fallback: try SendInput
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
        return;
    }

    int id = VkToHotkeyId(vk);
    if (id < 0) return;

    // WM_HOTKEY: wParam = hotkey ID, lParam = LOWORD(modifiers) | HIWORD(vk)
    LPARAM lp = MAKELPARAM(mod, vk);
    PostMessageW(hkWnd, WM_HOTKEY, (WPARAM)id, lp);
    Sleep(300);  // Give WinSplit time to process and move the window
}

RECT GetPrimaryWorkArea() {
    RECT rc;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, 0);
    return rc;
}

bool IsWinSplitRunning() {
    // Detect by process name since the hook frame has no window title
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

}  // namespace

TEST_CASE("Window Positioning", "[functional][positioning][.interactive]") {
    if (!IsWinSplitRunning()) {
        SKIP("WinSplit not running");
    }

    RECT workArea = GetPrimaryWorkArea();
    int workW = workArea.right - workArea.left;
    int workH = workArea.bottom - workArea.top;
    const int tolerance = 10;

    SECTION("Snap to left half with Numpad 4") {
        HWND hwnd = CreateTestWindow(workArea.left + 100, workArea.top + 100, 400, 300);
        REQUIRE(hwnd != nullptr);

        SetForegroundWindow(hwnd);
        Sleep(100);

        SimulateHotkey(MOD_CONTROL | MOD_ALT, VK_NUMPAD4);

        RECT vis = GetVisibleRect(hwnd);
        int visW = vis.right - vis.left;
        int visH = vis.bottom - vis.top;

        CHECK(std::abs(vis.left - workArea.left) <= tolerance);
        CHECK(std::abs(vis.top - workArea.top) <= tolerance);
        CHECK(std::abs(visW - workW / 2) <= tolerance);
        CHECK(std::abs(visH - workH) <= tolerance);

        DestroyWindow(hwnd);
    }

    SECTION("Snap to right half with Numpad 6") {
        HWND hwnd = CreateTestWindow(workArea.left + 100, workArea.top + 100, 400, 300);
        REQUIRE(hwnd != nullptr);

        SetForegroundWindow(hwnd);
        Sleep(100);

        SimulateHotkey(MOD_CONTROL | MOD_ALT, VK_NUMPAD6);

        RECT vis = GetVisibleRect(hwnd);
        int expectedLeft = workArea.left + workW / 2;

        CHECK(std::abs(vis.left - expectedLeft) <= tolerance);
        CHECK(std::abs(vis.top - workArea.top) <= tolerance);

        DestroyWindow(hwnd);
    }

    SECTION("Maximize with Numpad 5") {
        HWND hwnd = CreateTestWindow(workArea.left + 100, workArea.top + 100, 400, 300);
        REQUIRE(hwnd != nullptr);

        SetForegroundWindow(hwnd);
        Sleep(100);

        SimulateHotkey(MOD_CONTROL | MOD_ALT, VK_NUMPAD5);

        RECT vis = GetVisibleRect(hwnd);

        // Visible bounds should match work area
        CHECK(std::abs(vis.left - workArea.left) <= tolerance);
        CHECK(std::abs(vis.top - workArea.top) <= tolerance);
        CHECK(std::abs(vis.right - workArea.right) <= tolerance);
        CHECK(std::abs(vis.bottom - workArea.bottom) <= tolerance);

        DestroyWindow(hwnd);
    }

    SECTION("All positions stay within work area") {
        HWND hwnd = CreateTestWindow(workArea.left + 100, workArea.top + 100, 400, 300);
        REQUIRE(hwnd != nullptr);

        WORD keys[] = { VK_NUMPAD1, VK_NUMPAD2, VK_NUMPAD3, VK_NUMPAD4,
                        VK_NUMPAD5, VK_NUMPAD6, VK_NUMPAD7, VK_NUMPAD8, VK_NUMPAD9 };

        for (WORD key : keys) {
            SetForegroundWindow(hwnd);
            Sleep(50);

            SimulateHotkey(MOD_CONTROL | MOD_ALT, key);

            RECT vis = GetVisibleRect(hwnd);

            CHECK(vis.left >= workArea.left - tolerance);
            CHECK(vis.top >= workArea.top - tolerance);
            CHECK(vis.right <= workArea.right + tolerance);
            CHECK(vis.bottom <= workArea.bottom + tolerance);
        }

        DestroyWindow(hwnd);
    }
}

TEST_CASE("DWM Frame Compensation", "[functional][dwm][.interactive]") {
    if (!IsWinSplitRunning()) {
        SKIP("WinSplit not running");
    }

    HWND hwnd = CreateTestWindow(200, 200, 400, 300);
    REQUIRE(hwnd != nullptr);

    SECTION("Visible bounds align with work area when maximized") {
        SetForegroundWindow(hwnd);
        Sleep(100);

        SimulateHotkey(MOD_CONTROL | MOD_ALT, VK_NUMPAD5);

        RECT winRect, visRect;
        GetWindowRect(hwnd, &winRect);
        DwmGetWindowAttribute(hwnd, DWMWA_EXTENDED_FRAME_BOUNDS, &visRect, sizeof(visRect));

        RECT workArea = GetPrimaryWorkArea();
        const int tolerance = 5;

        // The visible (not window) rect should align with work area
        CHECK(std::abs(visRect.left - workArea.left) <= tolerance);
        CHECK(std::abs(visRect.top - workArea.top) <= tolerance);
        CHECK(std::abs(visRect.right - workArea.right) <= tolerance);
        CHECK(std::abs(visRect.bottom - workArea.bottom) <= tolerance);

        INFO("Work area: " << workArea.left << "," << workArea.top
             << " -> " << workArea.right << "," << workArea.bottom);
        INFO("Visible rect: " << visRect.left << "," << visRect.top
             << " -> " << visRect.right << "," << visRect.bottom);
    }

    DestroyWindow(hwnd);
}
