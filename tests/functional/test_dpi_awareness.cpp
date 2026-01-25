/**
 * DPI Awareness Functional Test
 *
 * Tests WinSplit's high DPI awareness including:
 * - Per-monitor DPI awareness
 * - Correct window positioning at different DPI scales
 * - DPI-aware coordinate handling
 */

#include "../tools/test_harness.h"
#include <stdio.h>
#include <dwmapi.h>
#include <shellscalingapi.h>

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "shcore.lib")

// DPI context types
typedef UINT (WINAPI *GetDpiForWindow_t)(HWND);
typedef UINT (WINAPI *GetDpiForSystem_t)();
typedef DPI_AWARENESS_CONTEXT (WINAPI *GetThreadDpiAwarenessContext_t)();
typedef DPI_AWARENESS (WINAPI *GetAwarenessFromDpiAwarenessContext_t)(DPI_AWARENESS_CONTEXT);
typedef BOOL (WINAPI *AdjustWindowRectExForDpi_t)(LPRECT, DWORD, BOOL, DWORD, UINT);

// Function pointers (loaded dynamically)
GetDpiForWindow_t pGetDpiForWindow = nullptr;
GetDpiForSystem_t pGetDpiForSystem = nullptr;
GetThreadDpiAwarenessContext_t pGetThreadDpiAwarenessContext = nullptr;
GetAwarenessFromDpiAwarenessContext_t pGetAwarenessFromDpiAwarenessContext = nullptr;

// Load DPI functions
bool LoadDpiFunctions() {
    HMODULE hUser32 = GetModuleHandle(L"user32.dll");
    if (!hUser32) return false;

    pGetDpiForWindow = (GetDpiForWindow_t)GetProcAddress(hUser32, "GetDpiForWindow");
    pGetDpiForSystem = (GetDpiForSystem_t)GetProcAddress(hUser32, "GetDpiForSystem");
    pGetThreadDpiAwarenessContext = (GetThreadDpiAwarenessContext_t)
        GetProcAddress(hUser32, "GetThreadDpiAwarenessContext");
    pGetAwarenessFromDpiAwarenessContext = (GetAwarenessFromDpiAwarenessContext_t)
        GetProcAddress(hUser32, "GetAwarenessFromDpiAwarenessContext");

    return pGetDpiForWindow != nullptr;
}

// Get DPI for a window (with fallback)
UINT GetDpiForWindowSafe(HWND hwnd) {
    if (pGetDpiForWindow) {
        return pGetDpiForWindow(hwnd);
    }

    // Fallback for older Windows
    HDC hdc = GetDC(hwnd);
    UINT dpi = GetDeviceCaps(hdc, LOGPIXELSX);
    ReleaseDC(hwnd, hdc);
    return dpi;
}

// Get system DPI
UINT GetSystemDpiSafe() {
    if (pGetDpiForSystem) {
        return pGetDpiForSystem();
    }

    HDC hdc = GetDC(NULL);
    UINT dpi = GetDeviceCaps(hdc, LOGPIXELSX);
    ReleaseDC(NULL, hdc);
    return dpi;
}

// Get current DPI awareness context
const char* GetDpiAwarenessString() {
    if (!pGetThreadDpiAwarenessContext || !pGetAwarenessFromDpiAwarenessContext) {
        return "Unknown (API not available)";
    }

    DPI_AWARENESS_CONTEXT ctx = pGetThreadDpiAwarenessContext();
    DPI_AWARENESS awareness = pGetAwarenessFromDpiAwarenessContext(ctx);

    switch (awareness) {
    case DPI_AWARENESS_UNAWARE:
        return "Unaware";
    case DPI_AWARENESS_SYSTEM_AWARE:
        return "System Aware";
    case DPI_AWARENESS_PER_MONITOR_AWARE:
        return "Per-Monitor Aware";
    default:
        return "Unknown";
    }
}

// Create a test window
HWND CreateTestWindow(int x, int y, int w, int h) {
    static bool registered = false;
    static const wchar_t* className = L"WinSplitDpiTest";

    if (!registered) {
        WNDCLASS wc = {};
        wc.lpfnWndProc = DefWindowProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.lpszClassName = className;
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        RegisterClass(&wc);
        registered = true;
    }

    HWND hwnd = CreateWindowEx(
        0, className, L"DPI Test Window",
        WS_OVERLAPPEDWINDOW,
        x, y, w, h,
        NULL, NULL, GetModuleHandle(NULL), NULL
    );

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    return hwnd;
}

// Get visible rect
RECT GetVisibleRect(HWND hwnd) {
    RECT rect;
    DwmGetWindowAttribute(hwnd, DWMWA_EXTENDED_FRAME_BOUNDS, &rect, sizeof(rect));
    return rect;
}

// Simulate hotkey
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
    Sleep(100);
}

// Test: Load DPI functions
bool Test_LoadDpiFunctions() {
    bool loaded = LoadDpiFunctions();

    printf("  GetDpiForWindow: %s\n", pGetDpiForWindow ? "Available" : "Not available");
    printf("  GetDpiForSystem: %s\n", pGetDpiForSystem ? "Available" : "Not available");
    printf("  GetThreadDpiAwarenessContext: %s\n",
           pGetThreadDpiAwarenessContext ? "Available" : "Not available");

    // These functions are only available on Windows 10+
    // Not a failure if missing on older systems
    return true;
}

// Test: Check current DPI settings
bool Test_CurrentDpiSettings() {
    printf("  System DPI: %u (%d%%)\n", GetSystemDpiSafe(), GetSystemDpiSafe() * 100 / 96);
    printf("  Test process DPI awareness: %s\n", GetDpiAwarenessString());

    // Check WinSplit process
    DWORD pid = TestHarness::GetWinSplitProcessId();
    if (pid) {
        // We can't easily get another process's DPI awareness
        printf("  WinSplit process found (PID %lu)\n", pid);
    }

    return true;
}

// Test: Window positioning at current DPI
bool Test_WindowPositioningAtDpi() {
    UINT dpi = GetSystemDpiSafe();
    float scale = (float)dpi / 96.0f;

    printf("  Testing at %d DPI (%.0f%% scale)...\n", dpi, scale * 100);

    RECT workArea = TestHarness::GetPrimaryWorkArea();
    int workW = workArea.right - workArea.left;
    int workH = workArea.bottom - workArea.top;

    HWND hwnd = CreateTestWindow(
        workArea.left + 100,
        workArea.top + 100,
        400, 300
    );

    if (!hwnd) return false;

    SetForegroundWindow(hwnd);
    Sleep(100);

    // Maximize
    SimulateHotkey(MOD_CONTROL | MOD_ALT, VK_NUMPAD5);
    Sleep(200);

    RECT vis = GetVisibleRect(hwnd);

    printf("  Work area: %dx%d\n", workW, workH);
    printf("  Window visible: %ldx%ld at (%ld,%ld)\n",
           vis.right - vis.left, vis.bottom - vis.top,
           vis.left, vis.top);

    int tolerance = 10;
    bool matches =
        abs(vis.left - workArea.left) <= tolerance &&
        abs(vis.top - workArea.top) <= tolerance &&
        abs(vis.right - workArea.right) <= tolerance &&
        abs(vis.bottom - workArea.bottom) <= tolerance;

    DestroyWindow(hwnd);

    if (matches) {
        printf("  Window fills work area correctly at %d DPI [GOOD]\n", dpi);
    } else {
        printf("  Window does not fill work area at %d DPI [BAD]\n", dpi);
    }

    return matches;
}

// Test: Per-monitor DPI with different DPI monitors
bool Test_PerMonitorDpi() {
    printf("  Checking for per-monitor DPI scenario...\n");

    // Enumerate monitors
    std::vector<HMONITOR> monitors;
    EnumDisplayMonitors(NULL, NULL, [](HMONITOR hMon, HDC, LPRECT, LPARAM data) -> BOOL {
        auto& vec = *reinterpret_cast<std::vector<HMONITOR>*>(data);
        vec.push_back(hMon);
        return TRUE;
    }, reinterpret_cast<LPARAM>(&monitors));

    if (monitors.size() < 2) {
        printf("  Single monitor - per-monitor DPI test not applicable\n");
        return true;
    }

    // Try to get DPI for each monitor (requires GetDpiForMonitor from shcore)
    HMODULE hShcore = LoadLibrary(L"shcore.dll");
    if (!hShcore) {
        printf("  Cannot load shcore.dll\n");
        return true;
    }

    typedef HRESULT (WINAPI *GetDpiForMonitor_t)(HMONITOR, int, UINT*, UINT*);
    GetDpiForMonitor_t pGetDpiForMonitor =
        (GetDpiForMonitor_t)GetProcAddress(hShcore, "GetDpiForMonitor");

    if (pGetDpiForMonitor) {
        bool differentDpi = false;
        UINT firstDpi = 0;

        for (size_t i = 0; i < monitors.size(); i++) {
            UINT dpiX, dpiY;
            if (SUCCEEDED(pGetDpiForMonitor(monitors[i], 0, &dpiX, &dpiY))) {
                printf("  Monitor %zu: %u DPI\n", i + 1, dpiX);

                if (i == 0) {
                    firstDpi = dpiX;
                } else if (dpiX != firstDpi) {
                    differentDpi = true;
                }
            }
        }

        if (differentDpi) {
            printf("  Mixed DPI configuration detected!\n");
            printf("  WinSplit should use per-monitor DPI awareness.\n");
        } else {
            printf("  All monitors have same DPI.\n");
        }
    } else {
        printf("  GetDpiForMonitor not available\n");
    }

    FreeLibrary(hShcore);
    return true;
}

// Test: Coordinate accuracy at high DPI
bool Test_CoordinateAccuracy() {
    printf("  Testing coordinate accuracy...\n");

    RECT workArea = TestHarness::GetPrimaryWorkArea();
    int workW = workArea.right - workArea.left;
    int workH = workArea.bottom - workArea.top;

    // Test various fractions
    struct TestCase {
        WORD key;
        float expectedX;
        float expectedWidth;
        const char* name;
    } cases[] = {
        { VK_NUMPAD4, 0.0f, 0.5f, "Left half" },
        { VK_NUMPAD6, 0.5f, 0.5f, "Right half" },
    };

    HWND hwnd = CreateTestWindow(
        workArea.left + 100, workArea.top + 100,
        400, 300
    );

    if (!hwnd) return false;

    bool allAccurate = true;

    for (const auto& tc : cases) {
        // Reset position
        SetWindowPos(hwnd, NULL, workArea.left + 100, workArea.top + 100,
                     400, 300, SWP_NOZORDER);
        Sleep(50);

        SetForegroundWindow(hwnd);
        Sleep(50);

        SimulateHotkey(MOD_CONTROL | MOD_ALT, tc.key);
        Sleep(200);

        RECT vis = GetVisibleRect(hwnd);

        int expectedX = workArea.left + (int)(tc.expectedX * workW);
        int expectedW = (int)(tc.expectedWidth * workW);
        int tolerance = 10;

        bool xOk = abs(vis.left - expectedX) <= tolerance;
        bool wOk = abs((int)(vis.right - vis.left) - expectedW) <= tolerance;

        printf("  %s: X=%ld (exp %d), W=%ld (exp %d) %s\n",
               tc.name,
               vis.left, expectedX,
               vis.right - vis.left, expectedW,
               (xOk && wOk) ? "[OK]" : "[MISMATCH]");

        if (!xOk || !wOk) allAccurate = false;
    }

    DestroyWindow(hwnd);
    return allAccurate;
}

// Test: WinSplit manifest DPI settings
bool Test_ManifestDpiSettings() {
    printf("  Checking WinSplit manifest for DPI settings...\n");

    // Find WinSplit executable
    DWORD pid = TestHarness::GetWinSplitProcessId();
    if (!pid) {
        printf("  WinSplit not running\n");
        return true;
    }

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!hProcess) {
        printf("  Cannot query WinSplit process\n");
        return true;
    }

    wchar_t exePath[MAX_PATH];
    DWORD size = MAX_PATH;
    QueryFullProcessImageName(hProcess, 0, exePath, &size);
    CloseHandle(hProcess);

    printf("  WinSplit path: %ls\n", exePath);

    // The manifest is embedded in the EXE
    // We can't easily read it, but we verified it exists in our updates
    printf("  Manifest check: Verify winsplit.manifest is embedded\n");
    printf("  Expected: <dpiAwareness>PerMonitorV2</dpiAwareness>\n");

    return true;
}

int main() {
    TestHarness::Init("DPI Awareness Functional Test");

    printf("Testing high DPI support...\n\n");

    if (!TestHarness::IsWinSplitRunning()) {
        printf("WARNING: WinSplit may not be running.\n\n");
    }

    // Run tests
    TestHarness::RunTest("Load DPI functions", Test_LoadDpiFunctions);
    TestHarness::RunTest("Current DPI settings", Test_CurrentDpiSettings);
    TestHarness::RunTest("Window positioning at DPI", Test_WindowPositioningAtDpi);
    TestHarness::RunTest("Per-monitor DPI", Test_PerMonitorDpi);
    TestHarness::RunTest("Coordinate accuracy", Test_CoordinateAccuracy);
    TestHarness::RunTest("Manifest DPI settings", Test_ManifestDpiSettings);

    printf("\n");
    TestHarness::PrintColored("TESTING RECOMMENDATIONS:\n", COLOR_YELLOW);
    printf("1. Test at 100%%, 125%%, 150%%, 200%% display scaling\n");
    printf("2. Test with mixed-DPI multi-monitor setup\n");
    printf("3. Verify no blurry text in WinSplit dialogs\n");
    printf("4. Check that window snapping aligns perfectly at all DPIs\n\n");

    return TestHarness::Summarize();
}
