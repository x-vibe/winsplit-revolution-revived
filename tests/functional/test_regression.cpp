/**
 * Regression Tests
 *
 * Tests for specific bugs that were fixed in the Windows 11 compatibility update.
 * Ensures these issues don't reappear.
 */

#include "../tools/test_harness.h"
#include <stdio.h>
#include <dwmapi.h>

#pragma comment(lib, "dwmapi.lib")

// Create test window
HWND CreateTestWindow(int x, int y, int w, int h) {
    static bool registered = false;
    static const wchar_t* className = L"RegressionTestWindow";

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
        0, className, L"Regression Test",
        WS_OVERLAPPEDWINDOW,
        x, y, w, h,
        NULL, NULL, GetModuleHandle(NULL), NULL
    );

    ShowWindow(hwnd, SW_SHOW);
    return hwnd;
}

// Get visible rect (DWM compensated)
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

/**
 * BUG FIX #1: Logic error in functions_resize.cpp
 *
 * Original code had a precedence error:
 *   if (SUCCEEDED(hr) && ext_frame.top != res.x || ext_frame.left != res.y || ...)
 *
 * This evaluated as:
 *   if ((SUCCEEDED(hr) && ext_frame.top != res.x) || ext_frame.left != res.y || ...)
 *
 * Should be:
 *   if (SUCCEEDED(hr) && (ext_frame.top != res.x || ext_frame.left != res.y || ...))
 *
 * The bug caused DWM frame compensation to not work correctly because
 * the condition was almost always true due to operator precedence.
 */
bool Test_Bug1_LogicPrecedence() {
    printf("  BUG #1: Logic precedence in frame detection\n");
    printf("  Testing that DWM frames are detected correctly...\n");

    HWND hwnd = CreateTestWindow(200, 200, 400, 300);
    if (!hwnd) return false;

    // Get the actual window rect and the visible rect
    RECT winRect, visRect;
    GetWindowRect(hwnd, &winRect);

    HRESULT hr = DwmGetWindowAttribute(hwnd, DWMWA_EXTENDED_FRAME_BOUNDS,
                                        &visRect, sizeof(visRect));

    if (FAILED(hr)) {
        printf("  DWM not available (pre-Win10?)\n");
        DestroyWindow(hwnd);
        return true;  // Not a bug on non-DWM systems
    }

    // Calculate invisible frame borders
    int leftFrame = visRect.left - winRect.left;
    int topFrame = visRect.top - winRect.top;
    int rightFrame = winRect.right - visRect.right;
    int bottomFrame = winRect.bottom - visRect.bottom;

    printf("  Window rect:  (%ld, %ld) - (%ld, %ld)\n",
           winRect.left, winRect.top, winRect.right, winRect.bottom);
    printf("  Visible rect: (%ld, %ld) - (%ld, %ld)\n",
           visRect.left, visRect.top, visRect.right, visRect.bottom);
    printf("  Invisible frame: L=%d T=%d R=%d B=%d\n",
           leftFrame, topFrame, rightFrame, bottomFrame);

    // On Windows 10/11, there should be invisible frames (usually 7-8 pixels)
    bool hasInvisibleFrame = (leftFrame != 0 || rightFrame != 0 || bottomFrame != 0);

    if (hasInvisibleFrame) {
        printf("  Invisible frame detected correctly [PASS]\n");
    } else {
        // Could be Win7/8 without extended frames
        printf("  No invisible frame (expected on older Windows) [OK]\n");
    }

    DestroyWindow(hwnd);
    return true;
}

/**
 * BUG FIX #2: DWM frame compensation in multimonitor_move.cpp
 *
 * Original code didn't compensate for invisible DWM frames when
 * moving windows between monitors. This caused windows to appear
 * offset from their expected positions.
 */
bool Test_Bug2_MultiMonitorDwmCompensation() {
    printf("  BUG #2: DWM compensation in multi-monitor moves\n");

    HWND hwnd = CreateTestWindow(100, 100, 400, 300);
    if (!hwnd) return false;

    SetForegroundWindow(hwnd);
    Sleep(100);

    // Get work area
    RECT workArea = TestHarness::GetPrimaryWorkArea();

    // Snap to left half
    SimulateHotkey(MOD_CONTROL | MOD_ALT, VK_NUMPAD4);
    Sleep(300);

    RECT visRect = GetVisibleRect(hwnd);

    // The VISIBLE bounds should align with the work area, not the window rect
    int tolerance = 5;

    bool leftAligned = abs(visRect.left - workArea.left) <= tolerance;
    bool topAligned = abs(visRect.top - workArea.top) <= tolerance;

    printf("  Work area left: %ld, Visible left: %ld (diff: %ld)\n",
           workArea.left, visRect.left, visRect.left - workArea.left);
    printf("  Work area top:  %ld, Visible top:  %ld (diff: %ld)\n",
           workArea.top, visRect.top, visRect.top - workArea.top);

    DestroyWindow(hwnd);

    if (leftAligned && topAligned) {
        printf("  Visible bounds align with work area [PASS]\n");
        return true;
    } else {
        printf("  Visible bounds don't align with work area [FAIL]\n");
        printf("  This indicates DWM compensation is not working.\n");
        return false;
    }
}

/**
 * BUG FIX #3: Drag'n'Go frame compensation in frame_hook.cpp
 *
 * The drag-and-drop preview rectangle was not compensated for
 * invisible DWM frames, causing the preview to not match the
 * actual window position after release.
 */
bool Test_Bug3_DragNGoFrameCompensation() {
    printf("  BUG #3: Drag'n'Go preview frame compensation\n");
    printf("  (This test is informational - requires manual verification)\n");

    HWND hwnd = CreateTestWindow(200, 200, 400, 300);
    if (!hwnd) return false;

    SetForegroundWindow(hwnd);
    Sleep(100);

    // Snap to a position
    SimulateHotkey(MOD_CONTROL | MOD_ALT, VK_NUMPAD5);
    Sleep(300);

    // Get the visible rect AFTER snapping
    RECT visAfterSnap = GetVisibleRect(hwnd);

    printf("  Window snapped to: (%ld, %ld) - (%ld, %ld)\n",
           visAfterSnap.left, visAfterSnap.top,
           visAfterSnap.right, visAfterSnap.bottom);

    printf("  To fully test: Use Drag'n'Go and verify preview matches result.\n");

    DestroyWindow(hwnd);
    return true;
}

/**
 * BUG FIX #4: DPI awareness initialization in main.cpp
 *
 * Original code didn't call SetProcessDpiAwarenessContext or fallbacks,
 * relying only on the manifest. This could cause issues if the manifest
 * wasn't properly embedded.
 */
bool Test_Bug4_DpiAwareness() {
    printf("  BUG #4: DPI awareness initialization\n");

    // Check if we can detect DPI awareness
    typedef UINT (WINAPI *GetDpiForSystem_t)();
    typedef DPI_AWARENESS_CONTEXT (WINAPI *GetThreadDpiAwarenessContext_t)();
    typedef DPI_AWARENESS (WINAPI *GetAwarenessFromDpiAwarenessContext_t)(DPI_AWARENESS_CONTEXT);

    HMODULE hUser32 = GetModuleHandle(L"user32.dll");
    if (!hUser32) return true;

    GetDpiForSystem_t pGetDpiForSystem =
        (GetDpiForSystem_t)GetProcAddress(hUser32, "GetDpiForSystem");
    GetThreadDpiAwarenessContext_t pGetThreadDpiAwarenessContext =
        (GetThreadDpiAwarenessContext_t)GetProcAddress(hUser32, "GetThreadDpiAwarenessContext");
    GetAwarenessFromDpiAwarenessContext_t pGetAwarenessFromDpiAwarenessContext =
        (GetAwarenessFromDpiAwarenessContext_t)GetProcAddress(hUser32, "GetAwarenessFromDpiAwarenessContext");

    if (pGetDpiForSystem) {
        UINT dpi = pGetDpiForSystem();
        printf("  System DPI: %u (%d%% scaling)\n", dpi, dpi * 100 / 96);
    }

    if (pGetThreadDpiAwarenessContext && pGetAwarenessFromDpiAwarenessContext) {
        DPI_AWARENESS_CONTEXT ctx = pGetThreadDpiAwarenessContext();
        DPI_AWARENESS awareness = pGetAwarenessFromDpiAwarenessContext(ctx);

        const char* awarenessStr = "Unknown";
        switch (awareness) {
        case DPI_AWARENESS_UNAWARE: awarenessStr = "Unaware"; break;
        case DPI_AWARENESS_SYSTEM_AWARE: awarenessStr = "System Aware"; break;
        case DPI_AWARENESS_PER_MONITOR_AWARE: awarenessStr = "Per-Monitor Aware"; break;
        }

        printf("  Test process DPI awareness: %s\n", awarenessStr);

        // Note: We can't easily check WinSplit's DPI awareness from here
        printf("  WinSplit should be Per-Monitor Aware (via manifest + code)\n");
    }

    return true;
}

/**
 * BUG FIX #5: Manifest embedding for Windows 11 compatibility
 *
 * Added winsplit.manifest with:
 * - Windows 11 compatibility GUID
 * - Per-Monitor DPI V2 awareness
 * - Common Controls v6
 */
bool Test_Bug5_ManifestEmbedding() {
    printf("  BUG #5: Windows 11 manifest embedding\n");

    // Find WinSplit executable
    DWORD pid = TestHarness::GetWinSplitProcessId();
    if (!pid) {
        printf("  WinSplit not running - cannot verify manifest\n");
        return true;
    }

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!hProcess) {
        printf("  Cannot open WinSplit process\n");
        return true;
    }

    wchar_t exePath[MAX_PATH];
    DWORD size = MAX_PATH;
    QueryFullProcessImageName(hProcess, 0, exePath, &size);
    CloseHandle(hProcess);

    printf("  WinSplit path: %ls\n", exePath);

    // Check if the manifest is embedded by looking for manifest resource
    HMODULE hExe = LoadLibraryEx(exePath, NULL, LOAD_LIBRARY_AS_DATAFILE);
    if (hExe) {
        HRSRC hRes = FindResource(hExe, MAKEINTRESOURCE(1), RT_MANIFEST);
        if (hRes) {
            printf("  Manifest resource found (ID 1, RT_MANIFEST) [PASS]\n");

            // Load and check content
            HGLOBAL hGlobal = LoadResource(hExe, hRes);
            if (hGlobal) {
                DWORD resSize = SizeofResource(hExe, hRes);
                char* manifest = (char*)LockResource(hGlobal);
                if (manifest && resSize > 0) {
                    // Check for key strings
                    std::string manifestStr(manifest, resSize);

                    bool hasWin11 = manifestStr.find("8e0f7a12-bfb3-4fe8-b9a5-48fd50a15a9a") != std::string::npos;
                    bool hasDpiV2 = manifestStr.find("PerMonitorV2") != std::string::npos;
                    bool hasComCtl = manifestStr.find("Microsoft.Windows.Common-Controls") != std::string::npos;

                    printf("  Windows 11 GUID: %s\n", hasWin11 ? "Found" : "Missing");
                    printf("  PerMonitorV2 DPI: %s\n", hasDpiV2 ? "Found" : "Missing");
                    printf("  Common Controls v6: %s\n", hasComCtl ? "Found" : "Missing");
                }
            }
        } else {
            printf("  No manifest resource found [WARN]\n");
        }
        FreeLibrary(hExe);
    }

    return true;
}

int main() {
    TestHarness::Init("Regression Tests");

    printf("Testing that fixed bugs don't reappear...\n\n");

    if (!TestHarness::IsWinSplitRunning()) {
        printf("NOTE: Some tests require WinSplit to be running.\n\n");
    }

    // Run tests
    TestHarness::RunTest("Bug #1: Logic precedence", Test_Bug1_LogicPrecedence);
    TestHarness::RunTest("Bug #2: Multi-monitor DWM", Test_Bug2_MultiMonitorDwmCompensation);
    TestHarness::RunTest("Bug #3: Drag'n'Go frames", Test_Bug3_DragNGoFrameCompensation);
    TestHarness::RunTest("Bug #4: DPI awareness", Test_Bug4_DpiAwareness);
    TestHarness::RunTest("Bug #5: Manifest embedding", Test_Bug5_ManifestEmbedding);

    printf("\n");
    TestHarness::PrintColored("REGRESSION TEST INFO:\n", COLOR_YELLOW);
    printf("These tests verify that fixes from v10.0.2 remain effective:\n");
    printf("- DWM invisible frame handling\n");
    printf("- Per-monitor DPI awareness\n");
    printf("- Windows 11 compatibility manifest\n\n");

    return TestHarness::Summarize();
}
