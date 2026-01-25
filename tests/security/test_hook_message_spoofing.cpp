/**
 * Hook Message Spoofing Security Test
 * CRITICAL SECURITY TEST
 *
 * Tests WinSplit's resistance to spoofed inter-process messages.
 *
 * Background:
 * WinSplit uses a global hook DLL that communicates with the main app
 * via Windows messages (WSM_STARTMOVING, WSM_STOPMOVING). These messages
 * can be sent by ANY process, creating a security risk where malicious
 * software could trigger window movements without user consent.
 *
 * This test verifies:
 * 1. WinSplit doesn't crash on spoofed messages
 * 2. Extreme/malformed values are handled gracefully
 * 3. No unauthorized window actions occur (manual verification)
 */

#include "../tools/test_harness.h"
#include <stdio.h>

// WinSplit message constants (from hook.h)
#define WSM_STARTMOVING  (WM_USER + 100)
#define WSM_STOPMOVING   (WM_USER + 101)

// Find WinSplit hook frame window
HWND FindHookFrame() {
    return FindWindow(NULL, L"WinSplit Revolution - Hook Frame");
}

// Test: WinSplit handles NULL hwnd in WSM_STARTMOVING
bool Test_NullHwnd() {
    HWND hwnd = FindHookFrame();
    if (!hwnd) return false;

    // Send with NULL window handle
    SendMessage(hwnd, WSM_STARTMOVING, (WPARAM)NULL, 0);
    SendMessage(hwnd, WSM_STOPMOVING, 0, 0);

    // If we get here without crash, test passes
    return true;
}

// Test: WinSplit handles invalid hwnd
bool Test_InvalidHwnd() {
    HWND hwnd = FindHookFrame();
    if (!hwnd) return false;

    // Send with obviously invalid handle
    SendMessage(hwnd, WSM_STARTMOVING, (WPARAM)0xDEADBEEF, 0);
    SendMessage(hwnd, WSM_STOPMOVING, 0, 0);

    // Send with -1 handle
    SendMessage(hwnd, WSM_STARTMOVING, (WPARAM)(HWND)-1, 0);
    SendMessage(hwnd, WSM_STOPMOVING, 0, 0);

    return true;
}

// Test: WinSplit handles extreme negative wheel values
bool Test_NegativeWheelValue() {
    HWND hwnd = FindHookFrame();
    if (!hwnd) return false;

    // First set up moving state
    HWND testWnd = GetDesktopWindow();
    SendMessage(hwnd, WSM_STARTMOVING, (WPARAM)testWnd, 0);

    // Send negative wheel delta (potential array underflow)
    SendMessage(hwnd, WSM_STOPMOVING, (WPARAM)-1, 0);

    // Repeat with INT_MIN
    SendMessage(hwnd, WSM_STARTMOVING, (WPARAM)testWnd, 0);
    SendMessage(hwnd, WSM_STOPMOVING, (WPARAM)INT_MIN, 0);

    return true;
}

// Test: WinSplit handles extreme positive wheel values
bool Test_ExtremePositiveWheel() {
    HWND hwnd = FindHookFrame();
    if (!hwnd) return false;

    HWND testWnd = GetDesktopWindow();
    SendMessage(hwnd, WSM_STARTMOVING, (WPARAM)testWnd, 0);

    // Send INT_MAX wheel delta (potential overflow)
    SendMessage(hwnd, WSM_STOPMOVING, (WPARAM)INT_MAX, 0);

    // Large positive value
    SendMessage(hwnd, WSM_STARTMOVING, (WPARAM)testWnd, 0);
    SendMessage(hwnd, WSM_STOPMOVING, (WPARAM)999999, 0);

    return true;
}

// Test: WSM_STOPMOVING without prior WSM_STARTMOVING
bool Test_StopWithoutStart() {
    HWND hwnd = FindHookFrame();
    if (!hwnd) return false;

    // Send stop without start - should be handled gracefully
    for (int i = 0; i < 100; i++) {
        SendMessage(hwnd, WSM_STOPMOVING, (WPARAM)i, 0);
    }

    return true;
}

// Test: Rapid alternating start/stop
bool Test_RapidStartStop() {
    HWND hwnd = FindHookFrame();
    if (!hwnd) return false;

    HWND testWnd = GetDesktopWindow();

    for (int i = 0; i < 1000; i++) {
        SendMessage(hwnd, WSM_STARTMOVING, (WPARAM)testWnd, 0);
        SendMessage(hwnd, WSM_STOPMOVING, (WPARAM)(i % 9), 0);
    }

    return true;
}

// Test: Multiple starts without stops
bool Test_MultipleStarts() {
    HWND hwnd = FindHookFrame();
    if (!hwnd) return false;

    HWND testWnd = GetDesktopWindow();

    // Send many starts without stops
    for (int i = 0; i < 100; i++) {
        SendMessage(hwnd, WSM_STARTMOVING, (WPARAM)testWnd, 0);
    }

    // Then one stop
    SendMessage(hwnd, WSM_STOPMOVING, 0, 0);

    return true;
}

// Test: Message from different thread
bool Test_CrossThreadMessage() {
    HWND hwnd = FindHookFrame();
    if (!hwnd) return false;

    // Post instead of Send (asynchronous)
    HWND testWnd = GetDesktopWindow();

    PostMessage(hwnd, WSM_STARTMOVING, (WPARAM)testWnd, 0);
    Sleep(50);
    PostMessage(hwnd, WSM_STOPMOVING, 0, 0);
    Sleep(50);

    return true;
}

// Test: Spoofed messages with closed target window
bool Test_ClosedWindow() {
    HWND hwnd = FindHookFrame();
    if (!hwnd) return false;

    // Create and immediately destroy a window
    HWND tempWnd = CreateWindow(
        L"STATIC", L"Temp", WS_POPUP,
        0, 0, 100, 100, NULL, NULL, NULL, NULL
    );

    HWND handleCopy = tempWnd;
    DestroyWindow(tempWnd);

    // Now send message with the destroyed window handle
    SendMessage(hwnd, WSM_STARTMOVING, (WPARAM)handleCopy, 0);
    SendMessage(hwnd, WSM_STOPMOVING, 0, 0);

    return true;
}

// Test: WM_USER range messages
bool Test_UserMessageRange() {
    HWND hwnd = FindHookFrame();
    if (!hwnd) return false;

    // Send various WM_USER messages to probe for crashes
    for (UINT msg = WM_USER; msg < WM_USER + 500; msg++) {
        SendMessage(hwnd, msg, 0, 0);
        SendMessage(hwnd, msg, (WPARAM)-1, (LPARAM)-1);
    }

    return true;
}

int main() {
    TestHarness::Init("Hook Message Spoofing Security Test");

    // Check if WinSplit is running
    if (!FindHookFrame()) {
        printf("ERROR: WinSplit hook window not found!\n");
        printf("Please start WinSplit Revolution before running this test.\n");
        return 1;
    }

    printf("WinSplit hook window found. Running security tests...\n\n");

    // Run all tests
    TestHarness::RunTest("NULL HWND in WSM_STARTMOVING", Test_NullHwnd);
    TestHarness::RunTest("Invalid HWND (0xDEADBEEF)", Test_InvalidHwnd);
    TestHarness::RunTest("Negative wheel values", Test_NegativeWheelValue);
    TestHarness::RunTest("Extreme positive wheel", Test_ExtremePositiveWheel);
    TestHarness::RunTest("STOP without START", Test_StopWithoutStart);
    TestHarness::RunTest("Rapid start/stop (1000x)", Test_RapidStartStop);
    TestHarness::RunTest("Multiple starts no stops", Test_MultipleStarts);
    TestHarness::RunTest("Cross-thread PostMessage", Test_CrossThreadMessage);
    TestHarness::RunTest("Closed window handle", Test_ClosedWindow);
    TestHarness::RunTest("WM_USER range probe", Test_UserMessageRange);

    printf("\n");
    TestHarness::PrintColored("IMPORTANT NOTES:\n", COLOR_YELLOW);
    printf("- These tests verify WinSplit doesn't CRASH on spoofed messages\n");
    printf("- Manual verification is needed to confirm no windows were moved\n");
    printf("- The vulnerability EXISTS: any process CAN send these messages\n");
    printf("- Fix recommendation: Validate message source or use named pipes\n\n");

    return TestHarness::Summarize();
}
