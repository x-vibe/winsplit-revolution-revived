/**
 * Window Positioning Functional Tests
 * Using Catch2 framework
 */

#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>
#include <windows.h>
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

RECT GetPrimaryWorkArea() {
    RECT rc;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, 0);
    return rc;
}

bool IsWinSplitRunning() {
    return FindWindow(nullptr, L"WinSplit Revolution - Hook Frame") != nullptr;
}

}  // namespace

TEST_CASE("Window Positioning", "[functional][positioning]") {
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

TEST_CASE("DWM Frame Compensation", "[functional][dwm]") {
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
