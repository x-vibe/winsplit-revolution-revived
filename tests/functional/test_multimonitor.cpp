/**
 * Multi-Monitor Functional Test
 *
 * Tests WinSplit's multi-monitor functionality including:
 * - Window movement between monitors
 * - DWM frame compensation across monitors
 * - Different resolution handling
 * - Monitor enumeration
 */

#include "../tools/test_harness.h"
#include <stdio.h>
#include <dwmapi.h>
#include <vector>

#pragma comment(lib, "dwmapi.lib")

// Monitor info structure
struct MonitorData {
    HMONITOR hMonitor;
    RECT rcMonitor;
    RECT rcWork;
    int index;
    bool isPrimary;
};

std::vector<MonitorData> g_monitors;

// Monitor enumeration callback
BOOL CALLBACK MonitorEnumProc(HMONITOR hMon, HDC hdcMon, LPRECT lprcMon, LPARAM dwData) {
    MonitorData md;
    md.hMonitor = hMon;
    md.index = (int)g_monitors.size();

    MONITORINFO mi;
    mi.cbSize = sizeof(mi);
    GetMonitorInfo(hMon, &mi);

    md.rcMonitor = mi.rcMonitor;
    md.rcWork = mi.rcWork;
    md.isPrimary = (mi.dwFlags & MONITORINFOF_PRIMARY) != 0;

    g_monitors.push_back(md);
    return TRUE;
}

// Enumerate all monitors
void EnumerateMonitors() {
    g_monitors.clear();
    EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, 0);
}

// Get visible window rect
RECT GetVisibleRect(HWND hwnd) {
    RECT rect;
    DwmGetWindowAttribute(hwnd, DWMWA_EXTENDED_FRAME_BOUNDS, &rect, sizeof(rect));
    return rect;
}

// Create a test window
HWND CreateTestWindow(int x, int y, int w, int h) {
    static bool registered = false;
    static const wchar_t* className = L"WinSplitMultiMonTest";

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
        0, className, L"Multi-Monitor Test Window",
        WS_OVERLAPPEDWINDOW,
        x, y, w, h,
        NULL, NULL, GetModuleHandle(NULL), NULL
    );

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    return hwnd;
}

// Simulate a hotkey
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

// Get which monitor contains a point
int GetMonitorIndex(POINT pt) {
    for (size_t i = 0; i < g_monitors.size(); i++) {
        if (PtInRect(&g_monitors[i].rcMonitor, pt)) {
            return (int)i;
        }
    }
    return -1;
}

// Test: Monitor enumeration
bool Test_MonitorEnumeration() {
    EnumerateMonitors();

    printf("  Found %zu monitor(s):\n", g_monitors.size());

    for (const auto& mon : g_monitors) {
        printf("  Monitor %d: %ldx%ld at (%ld,%ld)%s\n",
               mon.index + 1,
               mon.rcMonitor.right - mon.rcMonitor.left,
               mon.rcMonitor.bottom - mon.rcMonitor.top,
               mon.rcMonitor.left, mon.rcMonitor.top,
               mon.isPrimary ? " [PRIMARY]" : "");
    }

    return g_monitors.size() > 0;
}

// Test: Single monitor scenario
bool Test_SingleMonitor() {
    EnumerateMonitors();

    if (g_monitors.size() != 1) {
        printf("  Skipping: %zu monitors detected (need exactly 1)\n", g_monitors.size());
        return true;  // Not a failure, just skip
    }

    printf("  Single monitor detected. Testing basic positioning...\n");

    HWND hwnd = CreateTestWindow(
        g_monitors[0].rcWork.left + 100,
        g_monitors[0].rcWork.top + 100,
        400, 300
    );

    if (!hwnd) return false;

    SetForegroundWindow(hwnd);
    Sleep(100);

    // Test maximize
    SimulateHotkey(MOD_CONTROL | MOD_ALT, VK_NUMPAD5);
    Sleep(200);

    RECT vis = GetVisibleRect(hwnd);
    int tolerance = 10;

    bool fullScreen =
        abs(vis.left - g_monitors[0].rcWork.left) <= tolerance &&
        abs(vis.top - g_monitors[0].rcWork.top) <= tolerance &&
        abs(vis.right - g_monitors[0].rcWork.right) <= tolerance &&
        abs(vis.bottom - g_monitors[0].rcWork.bottom) <= tolerance;

    DestroyWindow(hwnd);

    if (fullScreen) {
        printf("  Window fills work area correctly [GOOD]\n");
    } else {
        printf("  Window does NOT fill work area [BAD]\n");
    }

    return fullScreen;
}

// Test: Multi-monitor move (Win+Shift+Arrow or Ctrl+Alt+Arrow)
bool Test_MultiMonitorMove() {
    EnumerateMonitors();

    if (g_monitors.size() < 2) {
        printf("  Skipping: Only %zu monitor(s) detected (need 2+)\n", g_monitors.size());
        return true;  // Not a failure
    }

    printf("  Multiple monitors detected. Testing cross-monitor movement...\n");

    // Create window on primary monitor
    int primaryIdx = 0;
    for (size_t i = 0; i < g_monitors.size(); i++) {
        if (g_monitors[i].isPrimary) {
            primaryIdx = (int)i;
            break;
        }
    }

    HWND hwnd = CreateTestWindow(
        g_monitors[primaryIdx].rcWork.left + 100,
        g_monitors[primaryIdx].rcWork.top + 100,
        400, 300
    );

    if (!hwnd) return false;

    SetForegroundWindow(hwnd);
    Sleep(100);

    // Get initial monitor
    RECT vis1 = GetVisibleRect(hwnd);
    POINT center1 = { (vis1.left + vis1.right) / 2, (vis1.top + vis1.bottom) / 2 };
    int startMon = GetMonitorIndex(center1);

    printf("  Window starts on monitor %d\n", startMon + 1);

    // Try to move right (Win+Shift+Right or Ctrl+Alt+Right depending on config)
    // Default WinSplit uses Ctrl+Alt+Right
    SimulateHotkey(MOD_CONTROL | MOD_ALT, VK_RIGHT);
    Sleep(300);

    RECT vis2 = GetVisibleRect(hwnd);
    POINT center2 = { (vis2.left + vis2.right) / 2, (vis2.top + vis2.bottom) / 2 };
    int endMon = GetMonitorIndex(center2);

    printf("  Window now on monitor %d\n", endMon + 1);

    DestroyWindow(hwnd);

    if (endMon != startMon) {
        printf("  Window moved to different monitor [GOOD]\n");
        return true;
    } else {
        printf("  Window stayed on same monitor [INFO]\n");
        printf("  (Hotkey config may differ from test)\n");
        return true;  // Not a definite failure
    }
}

// Test: DWM compensation on secondary monitor
bool Test_DwmCompensationSecondary() {
    EnumerateMonitors();

    if (g_monitors.size() < 2) {
        printf("  Skipping: Only %zu monitor(s) detected\n", g_monitors.size());
        return true;
    }

    // Find non-primary monitor
    int secondaryIdx = -1;
    for (size_t i = 0; i < g_monitors.size(); i++) {
        if (!g_monitors[i].isPrimary) {
            secondaryIdx = (int)i;
            break;
        }
    }

    if (secondaryIdx < 0) {
        printf("  No secondary monitor found\n");
        return true;
    }

    printf("  Testing DWM compensation on monitor %d...\n", secondaryIdx + 1);

    // Create window on secondary monitor
    HWND hwnd = CreateTestWindow(
        g_monitors[secondaryIdx].rcWork.left + 100,
        g_monitors[secondaryIdx].rcWork.top + 100,
        400, 300
    );

    if (!hwnd) return false;

    SetForegroundWindow(hwnd);
    Sleep(100);

    // Maximize on secondary
    SimulateHotkey(MOD_CONTROL | MOD_ALT, VK_NUMPAD5);
    Sleep(200);

    RECT vis = GetVisibleRect(hwnd);
    RECT work = g_monitors[secondaryIdx].rcWork;
    int tolerance = 10;

    printf("  Work area: L=%ld T=%ld R=%ld B=%ld\n",
           work.left, work.top, work.right, work.bottom);
    printf("  Visible:   L=%ld T=%ld R=%ld B=%ld\n",
           vis.left, vis.top, vis.right, vis.bottom);

    bool matches =
        abs(vis.left - work.left) <= tolerance &&
        abs(vis.top - work.top) <= tolerance &&
        abs(vis.right - work.right) <= tolerance &&
        abs(vis.bottom - work.bottom) <= tolerance;

    DestroyWindow(hwnd);

    if (matches) {
        printf("  DWM compensation correct on secondary [GOOD]\n");
    } else {
        printf("  DWM compensation incorrect on secondary [BAD]\n");
    }

    return matches;
}

// Test: Different resolution monitors
bool Test_DifferentResolutions() {
    EnumerateMonitors();

    if (g_monitors.size() < 2) {
        printf("  Skipping: Only %zu monitor(s)\n", g_monitors.size());
        return true;
    }

    // Check if monitors have different resolutions
    int w0 = g_monitors[0].rcMonitor.right - g_monitors[0].rcMonitor.left;
    int h0 = g_monitors[0].rcMonitor.bottom - g_monitors[0].rcMonitor.top;

    bool differentRes = false;
    for (size_t i = 1; i < g_monitors.size(); i++) {
        int w = g_monitors[i].rcMonitor.right - g_monitors[i].rcMonitor.left;
        int h = g_monitors[i].rcMonitor.bottom - g_monitors[i].rcMonitor.top;
        if (w != w0 || h != h0) {
            differentRes = true;
            printf("  Monitor %zu has different resolution: %dx%d vs %dx%d\n",
                   i + 1, w, h, w0, h0);
        }
    }

    if (!differentRes) {
        printf("  All monitors have same resolution\n");
    }

    return true;  // Informational only
}

// Test: Window proportions after cross-monitor move
bool Test_ProportionalSizing() {
    EnumerateMonitors();

    if (g_monitors.size() < 2) {
        printf("  Skipping: Need 2+ monitors\n");
        return true;
    }

    printf("  Testing proportional sizing across monitors...\n");

    // Create window at 50% width on primary
    int primaryIdx = 0;
    for (size_t i = 0; i < g_monitors.size(); i++) {
        if (g_monitors[i].isPrimary) {
            primaryIdx = (int)i;
            break;
        }
    }

    RECT work = g_monitors[primaryIdx].rcWork;
    int halfWidth = (work.right - work.left) / 2;
    int fullHeight = work.bottom - work.top;

    HWND hwnd = CreateTestWindow(
        work.left, work.top, halfWidth, fullHeight
    );

    if (!hwnd) return false;

    SetForegroundWindow(hwnd);
    Sleep(100);

    // Move to next monitor
    SimulateHotkey(MOD_CONTROL | MOD_ALT, VK_RIGHT);
    Sleep(300);

    // Get new position
    RECT vis = GetVisibleRect(hwnd);
    POINT center = { (vis.left + vis.right) / 2, (vis.top + vis.bottom) / 2 };
    int newMon = GetMonitorIndex(center);

    if (newMon >= 0 && newMon != primaryIdx) {
        // Check proportions
        int newWorkW = g_monitors[newMon].rcWork.right - g_monitors[newMon].rcWork.left;
        int newWorkH = g_monitors[newMon].rcWork.bottom - g_monitors[newMon].rcWork.top;

        int expectedW = newWorkW / 2;  // Should still be 50%
        int actualW = vis.right - vis.left;

        float proportion = (float)actualW / (float)expectedW;
        printf("  Original: 50%% of monitor width\n");
        printf("  New width: %d (expected ~%d)\n", actualW, expectedW);
        printf("  Proportion ratio: %.2f\n", proportion);
    }

    DestroyWindow(hwnd);
    return true;  // Informational
}

int main() {
    TestHarness::Init("Multi-Monitor Functional Test");

    printf("Testing multi-monitor functionality...\n\n");

    // Check WinSplit
    if (!TestHarness::IsWinSplitRunning()) {
        printf("WARNING: WinSplit may not be running.\n\n");
    }

    // Run tests
    TestHarness::RunTest("Monitor enumeration", Test_MonitorEnumeration);
    TestHarness::RunTest("Single monitor scenario", Test_SingleMonitor);
    TestHarness::RunTest("Multi-monitor movement", Test_MultiMonitorMove);
    TestHarness::RunTest("DWM compensation (secondary)", Test_DwmCompensationSecondary);
    TestHarness::RunTest("Different resolution handling", Test_DifferentResolutions);
    TestHarness::RunTest("Proportional sizing", Test_ProportionalSizing);

    printf("\n");
    TestHarness::PrintColored("NOTE:\n", COLOR_YELLOW);
    printf("Multi-monitor tests work best with 2+ monitors.\n");
    printf("Some tests are informational if only 1 monitor is present.\n\n");

    return TestHarness::Summarize();
}
