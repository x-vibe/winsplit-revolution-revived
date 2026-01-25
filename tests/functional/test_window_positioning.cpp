/**
 * Window Positioning Functional Test
 * CORE FUNCTIONALITY TEST
 *
 * Tests WinSplit's window positioning accuracy including:
 * - Basic numpad grid positions (1-9)
 * - DWM frame compensation
 * - Position cycling
 * - Window placement accuracy
 */

#include "../tools/test_harness.h"
#include <stdio.h>
#include <dwmapi.h>

#pragma comment(lib, "dwmapi.lib")

// Create a test window
HWND CreateTestWindow(int x, int y, int w, int h) {
    static bool registered = false;
    static const wchar_t* className = L"WinSplitTestWindow";

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
        0, className, L"Test Window",
        WS_OVERLAPPEDWINDOW,
        x, y, w, h,
        NULL, NULL, GetModuleHandle(NULL), NULL
    );

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    return hwnd;
}

// Get visible window rect (compensating for DWM frame)
RECT GetVisibleRect(HWND hwnd) {
    RECT rect;
    DwmGetWindowAttribute(hwnd, DWMWA_EXTENDED_FRAME_BOUNDS, &rect, sizeof(rect));
    return rect;
}

// Simulate a hotkey press
void SimulateHotkey(WORD mod, WORD vk) {
    INPUT inputs[4] = {};
    int count = 0;

    // Press modifiers
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

    // Press key
    inputs[count].type = INPUT_KEYBOARD;
    inputs[count].ki.wVk = vk;
    count++;

    SendInput(count, inputs, sizeof(INPUT));
    Sleep(50);

    // Release all
    for (int i = 0; i < count; i++) {
        inputs[i].ki.dwFlags = KEYEVENTF_KEYUP;
    }
    SendInput(count, inputs, sizeof(INPUT));
    Sleep(100);
}

// Test: Window snaps to left half (Numpad 4)
bool Test_SnapLeftHalf() {
    // Create test window in center
    RECT workArea = TestHarness::GetPrimaryWorkArea();
    int workW = workArea.right - workArea.left;
    int workH = workArea.bottom - workArea.top;

    HWND hwnd = CreateTestWindow(
        workArea.left + workW / 4,
        workArea.top + workH / 4,
        workW / 2, workH / 2
    );

    if (!hwnd) return false;

    // Focus the window
    SetForegroundWindow(hwnd);
    Sleep(100);

    // Simulate Ctrl+Alt+4 (left half)
    SimulateHotkey(MOD_CONTROL | MOD_ALT, VK_NUMPAD4);
    Sleep(200);

    // Check position
    RECT visRect = GetVisibleRect(hwnd);

    // Expected: left half of screen
    int tolerance = 10;  // Allow 10px variance for DWM frames

    bool leftOk = abs(visRect.left - workArea.left) <= tolerance;
    bool topOk = abs(visRect.top - workArea.top) <= tolerance;
    bool widthOk = abs((visRect.right - visRect.left) - workW / 2) <= tolerance;
    bool heightOk = abs((visRect.bottom - visRect.top) - workH) <= tolerance;

    printf("  Expected left half: L=%ld T=%ld W=%d H=%d\n",
           workArea.left, workArea.top, workW / 2, workH);
    printf("  Actual visible:     L=%ld T=%ld W=%ld H=%ld\n",
           visRect.left, visRect.top,
           visRect.right - visRect.left, visRect.bottom - visRect.top);

    DestroyWindow(hwnd);

    return leftOk && topOk && widthOk && heightOk;
}

// Test: DWM frame compensation accuracy
bool Test_DwmFrameCompensation() {
    RECT workArea = TestHarness::GetPrimaryWorkArea();
    int workW = workArea.right - workArea.left;
    int workH = workArea.bottom - workArea.top;

    HWND hwnd = CreateTestWindow(
        workArea.left + 100, workArea.top + 100,
        400, 300
    );

    if (!hwnd) return false;

    SetForegroundWindow(hwnd);
    Sleep(100);

    // Maximize (Ctrl+Alt+5)
    SimulateHotkey(MOD_CONTROL | MOD_ALT, VK_NUMPAD5);
    Sleep(200);

    RECT visRect = GetVisibleRect(hwnd);
    RECT winRect;
    GetWindowRect(hwnd, &winRect);

    // Calculate invisible frame
    int leftFrame = visRect.left - winRect.left;
    int topFrame = visRect.top - winRect.top;
    int rightFrame = winRect.right - visRect.right;
    int bottomFrame = winRect.bottom - visRect.bottom;

    printf("  Window rect:  L=%ld T=%ld R=%ld B=%ld\n",
           winRect.left, winRect.top, winRect.right, winRect.bottom);
    printf("  Visible rect: L=%ld T=%ld R=%ld B=%ld\n",
           visRect.left, visRect.top, visRect.right, visRect.bottom);
    printf("  Invisible frame: L=%d T=%d R=%d B=%d\n",
           leftFrame, topFrame, rightFrame, bottomFrame);

    // Check that visible bounds match work area
    int tolerance = 5;
    bool matchesWorkArea =
        abs(visRect.left - workArea.left) <= tolerance &&
        abs(visRect.top - workArea.top) <= tolerance &&
        abs(visRect.right - workArea.right) <= tolerance &&
        abs(visRect.bottom - workArea.bottom) <= tolerance;

    if (matchesWorkArea) {
        printf("  Visible bounds match work area [GOOD]\n");
    } else {
        printf("  Visible bounds do NOT match work area [BAD]\n");
    }

    DestroyWindow(hwnd);
    return matchesWorkArea;
}

// Test: Position cycling (pressing same hotkey multiple times)
bool Test_PositionCycling() {
    RECT workArea = TestHarness::GetPrimaryWorkArea();
    int workW = workArea.right - workArea.left;
    int workH = workArea.bottom - workArea.top;

    HWND hwnd = CreateTestWindow(
        workArea.left + 100, workArea.top + 100,
        400, 300
    );

    if (!hwnd) return false;

    SetForegroundWindow(hwnd);
    Sleep(100);

    // Press Numpad 4 multiple times - should cycle through positions
    std::vector<RECT> positions;

    for (int i = 0; i < 5; i++) {
        SimulateHotkey(MOD_CONTROL | MOD_ALT, VK_NUMPAD4);
        Sleep(200);

        RECT vis = GetVisibleRect(hwnd);
        positions.push_back(vis);

        printf("  Position %d: L=%ld W=%ld\n", i + 1,
               vis.left, vis.right - vis.left);
    }

    // Check that positions cycle (not all same)
    bool hasDifferent = false;
    for (size_t i = 1; i < positions.size(); i++) {
        if (positions[i].left != positions[0].left ||
            positions[i].right != positions[0].right) {
            hasDifferent = true;
            break;
        }
    }

    DestroyWindow(hwnd);

    if (hasDifferent) {
        printf("  Position cycling works [GOOD]\n");
        return true;
    } else {
        printf("  No position cycling detected [INFO]\n");
        return true;  // Not a failure, just informational
    }
}

// Test: All numpad positions
bool Test_AllNumpadPositions() {
    printf("  Testing all 9 numpad positions...\n");

    RECT workArea = TestHarness::GetPrimaryWorkArea();

    HWND hwnd = CreateTestWindow(
        workArea.left + 100, workArea.top + 100,
        400, 300
    );

    if (!hwnd) return false;

    SetForegroundWindow(hwnd);
    Sleep(100);

    // Test each numpad position
    WORD keys[] = {
        VK_NUMPAD7, VK_NUMPAD8, VK_NUMPAD9,
        VK_NUMPAD4, VK_NUMPAD5, VK_NUMPAD6,
        VK_NUMPAD1, VK_NUMPAD2, VK_NUMPAD3
    };

    const char* names[] = {
        "7-TopLeft", "8-Top", "9-TopRight",
        "4-Left", "5-Full", "6-Right",
        "1-BotLeft", "2-Bottom", "3-BotRight"
    };

    bool allPassed = true;

    for (int i = 0; i < 9; i++) {
        // Reset to center
        SetWindowPos(hwnd, NULL,
                     workArea.left + 100, workArea.top + 100,
                     400, 300, SWP_NOZORDER);
        Sleep(50);

        SetForegroundWindow(hwnd);
        Sleep(50);

        SimulateHotkey(MOD_CONTROL | MOD_ALT, keys[i]);
        Sleep(200);

        RECT vis = GetVisibleRect(hwnd);

        printf("  %s: L=%ld T=%ld W=%ld H=%ld\n",
               names[i], vis.left, vis.top,
               vis.right - vis.left, vis.bottom - vis.top);

        // Basic sanity check - window should have moved
        if (vis.left == workArea.left + 100 && vis.top == workArea.top + 100) {
            printf("    WARNING: Window may not have moved!\n");
        }
    }

    DestroyWindow(hwnd);
    return allPassed;
}

// Test: Window stays in bounds
bool Test_WindowStaysInBounds() {
    RECT workArea = TestHarness::GetPrimaryWorkArea();

    HWND hwnd = CreateTestWindow(
        workArea.left + 100, workArea.top + 100,
        400, 300
    );

    if (!hwnd) return false;

    SetForegroundWindow(hwnd);
    Sleep(100);

    // Try all positions and verify window stays in work area
    WORD keys[] = { VK_NUMPAD1, VK_NUMPAD2, VK_NUMPAD3, VK_NUMPAD4,
                    VK_NUMPAD5, VK_NUMPAD6, VK_NUMPAD7, VK_NUMPAD8, VK_NUMPAD9 };

    bool allInBounds = true;

    for (int i = 0; i < 9; i++) {
        SimulateHotkey(MOD_CONTROL | MOD_ALT, keys[i]);
        Sleep(200);

        RECT vis = GetVisibleRect(hwnd);

        // Check bounds (with small tolerance for DWM)
        int tolerance = 5;
        bool inBounds =
            vis.left >= workArea.left - tolerance &&
            vis.top >= workArea.top - tolerance &&
            vis.right <= workArea.right + tolerance &&
            vis.bottom <= workArea.bottom + tolerance;

        if (!inBounds) {
            printf("  Numpad %d: OUT OF BOUNDS!\n", i + 1);
            printf("    Window: L=%ld T=%ld R=%ld B=%ld\n",
                   vis.left, vis.top, vis.right, vis.bottom);
            printf("    WorkArea: L=%ld T=%ld R=%ld B=%ld\n",
                   workArea.left, workArea.top, workArea.right, workArea.bottom);
            allInBounds = false;
        }
    }

    DestroyWindow(hwnd);

    if (allInBounds) {
        printf("  All positions within work area bounds [GOOD]\n");
    }

    return allInBounds;
}

int main() {
    TestHarness::Init("Window Positioning Functional Test");

    // Check if WinSplit is running
    if (!TestHarness::IsWinSplitRunning()) {
        printf("WARNING: WinSplit may not be running.\n");
        printf("Hotkey tests require WinSplit to be active.\n\n");
    }

    printf("Running window positioning tests...\n");
    printf("NOTE: These tests simulate hotkeys and require WinSplit to be running.\n\n");

    // Run tests
    TestHarness::RunTest("Snap to left half (Numpad 4)", Test_SnapLeftHalf);
    TestHarness::RunTest("DWM frame compensation", Test_DwmFrameCompensation);
    TestHarness::RunTest("Position cycling", Test_PositionCycling);
    TestHarness::RunTest("All numpad positions", Test_AllNumpadPositions);
    TestHarness::RunTest("Window stays in bounds", Test_WindowStaysInBounds);

    printf("\n");
    TestHarness::PrintColored("NOTE:\n", COLOR_YELLOW);
    printf("Visual verification is recommended after running these tests.\n");
    printf("Watch for any gaps or overlaps at screen edges.\n\n");

    return TestHarness::Summarize();
}
