/**
 * Multi-Monitor Functional Tests
 * Using Catch2 framework
 */

#include <catch2/catch_all.hpp>
#include <windows.h>
#include <dwmapi.h>
#include <vector>

#pragma comment(lib, "dwmapi.lib")

namespace {

struct MonitorInfo {
    HMONITOR handle;
    RECT bounds;
    RECT workArea;
    bool isPrimary;
};

std::vector<MonitorInfo> g_monitors;

BOOL CALLBACK MonitorEnumProc(HMONITOR hMon, HDC, LPRECT, LPARAM) {
    MonitorInfo mi;
    mi.handle = hMon;

    MONITORINFO info;
    info.cbSize = sizeof(info);
    GetMonitorInfo(hMon, &info);

    mi.bounds = info.rcMonitor;
    mi.workArea = info.rcWork;
    mi.isPrimary = (info.dwFlags & MONITORINFOF_PRIMARY) != 0;

    g_monitors.push_back(mi);
    return TRUE;
}

void RefreshMonitors() {
    g_monitors.clear();
    EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc, 0);
}

HWND CreateTestWindow(int x, int y, int w, int h) {
    static bool registered = false;
    static const wchar_t* className = L"Catch2MultiMonTest";

    if (!registered) {
        WNDCLASS wc = {};
        wc.lpfnWndProc = DefWindowProc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.lpszClassName = className;
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        RegisterClass(&wc);
        registered = true;
    }

    return CreateWindowEx(0, className, L"Multi-Mon Test",
                          WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                          x, y, w, h, nullptr, nullptr,
                          GetModuleHandle(nullptr), nullptr);
}

RECT GetVisibleRect(HWND hwnd) {
    RECT rect;
    DwmGetWindowAttribute(hwnd, DWMWA_EXTENDED_FRAME_BOUNDS, &rect, sizeof(rect));
    return rect;
}

int GetMonitorIndex(POINT pt) {
    for (size_t i = 0; i < g_monitors.size(); i++) {
        if (PtInRect(&g_monitors[i].bounds, pt)) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

bool IsWinSplitRunning() {
    return FindWindow(nullptr, L"WinSplit Revolution - Hook Frame") != nullptr;
}

}  // namespace

TEST_CASE("Monitor Enumeration", "[functional][multimonitor]") {
    RefreshMonitors();

    REQUIRE(g_monitors.size() >= 1);

    SECTION("Primary monitor exists") {
        bool hasPrimary = false;
        for (const auto& mon : g_monitors) {
            if (mon.isPrimary) {
                hasPrimary = true;
                break;
            }
        }
        CHECK(hasPrimary);
    }

    SECTION("Monitor dimensions are valid") {
        for (const auto& mon : g_monitors) {
            int w = mon.bounds.right - mon.bounds.left;
            int h = mon.bounds.bottom - mon.bounds.top;

            CHECK(w > 0);
            CHECK(h > 0);
            CHECK(w >= 640);  // Minimum reasonable width
            CHECK(h >= 480);  // Minimum reasonable height
        }
    }
}

TEST_CASE("Multi-Monitor Window Operations", "[functional][multimonitor]") {
    RefreshMonitors();

    if (g_monitors.size() < 2) {
        SKIP("Only one monitor - multi-monitor tests require 2+");
    }

    if (!IsWinSplitRunning()) {
        SKIP("WinSplit not running");
    }

    SECTION("Window on secondary monitor") {
        // Find non-primary monitor
        int secondaryIdx = -1;
        for (size_t i = 0; i < g_monitors.size(); i++) {
            if (!g_monitors[i].isPrimary) {
                secondaryIdx = static_cast<int>(i);
                break;
            }
        }

        REQUIRE(secondaryIdx >= 0);

        const auto& secondary = g_monitors[secondaryIdx];
        HWND hwnd = CreateTestWindow(
            secondary.workArea.left + 100,
            secondary.workArea.top + 100,
            400, 300
        );
        REQUIRE(hwnd != nullptr);

        // Verify window is on secondary
        RECT vis = GetVisibleRect(hwnd);
        POINT center = { (vis.left + vis.right) / 2, (vis.top + vis.bottom) / 2 };
        int monIdx = GetMonitorIndex(center);

        CHECK(monIdx == secondaryIdx);

        DestroyWindow(hwnd);
    }
}
