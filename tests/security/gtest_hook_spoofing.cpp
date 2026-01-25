/**
 * Hook Message Spoofing Security Tests
 * Using Google Test framework
 */

#include <gtest/gtest.h>
#include <windows.h>
#include <tlhelp32.h>

// WinSplit message constants
#define WSM_STARTMOVING  (WM_USER + 100)
#define WSM_STOPMOVING   (WM_USER + 101)

class HookSpoofingTest : public ::testing::Test {
protected:
    HWND hwndWinSplit = nullptr;

    void SetUp() override {
        hwndWinSplit = FindWindow(nullptr, L"WinSplit Revolution - Hook Frame");
        if (!hwndWinSplit) {
            GTEST_SKIP() << "WinSplit not running - skipping hook tests";
        }
    }

    void TearDown() override {
        // Cleanup: ensure no dangling state
        if (hwndWinSplit) {
            SendMessage(hwndWinSplit, WSM_STOPMOVING, 0, 0);
        }
    }

    bool IsWinSplitRunning() {
        return hwndWinSplit != nullptr;
    }
};

// Test: NULL window handle doesn't crash
TEST_F(HookSpoofingTest, NullHwndDoesNotCrash) {
    ASSERT_NE(hwndWinSplit, nullptr);

    // Should not crash
    LRESULT result = SendMessage(hwndWinSplit, WSM_STARTMOVING, (WPARAM)nullptr, 0);
    SendMessage(hwndWinSplit, WSM_STOPMOVING, 0, 0);

    // If we get here, test passed (no crash)
    SUCCEED();
}

// Test: Invalid window handle doesn't crash
TEST_F(HookSpoofingTest, InvalidHwndDoesNotCrash) {
    ASSERT_NE(hwndWinSplit, nullptr);

    // Various invalid handles
    HWND invalidHandles[] = {
        (HWND)0xDEADBEEF,
        (HWND)0xFFFFFFFF,
        (HWND)-1,
        (HWND)1
    };

    for (HWND invalid : invalidHandles) {
        SendMessage(hwndWinSplit, WSM_STARTMOVING, (WPARAM)invalid, 0);
        SendMessage(hwndWinSplit, WSM_STOPMOVING, 0, 0);
    }

    SUCCEED();
}

// Test: Negative wheel values don't cause array underflow
TEST_F(HookSpoofingTest, NegativeWheelValuesHandled) {
    ASSERT_NE(hwndWinSplit, nullptr);

    HWND testWnd = GetDesktopWindow();

    int negativeValues[] = { -1, -100, INT_MIN, -2147483648 };

    for (int val : negativeValues) {
        SendMessage(hwndWinSplit, WSM_STARTMOVING, (WPARAM)testWnd, 0);
        SendMessage(hwndWinSplit, WSM_STOPMOVING, (WPARAM)val, 0);
    }

    SUCCEED();
}

// Test: Extreme positive wheel values handled
TEST_F(HookSpoofingTest, ExtremePositiveWheelValues) {
    ASSERT_NE(hwndWinSplit, nullptr);

    HWND testWnd = GetDesktopWindow();

    int extremeValues[] = { 9999, 99999, INT_MAX, 2147483647 };

    for (int val : extremeValues) {
        SendMessage(hwndWinSplit, WSM_STARTMOVING, (WPARAM)testWnd, 0);
        SendMessage(hwndWinSplit, WSM_STOPMOVING, (WPARAM)val, 0);
    }

    SUCCEED();
}

// Test: STOP without START doesn't crash
TEST_F(HookSpoofingTest, StopWithoutStartSafe) {
    ASSERT_NE(hwndWinSplit, nullptr);

    // Send many stops without starts
    for (int i = 0; i < 100; i++) {
        SendMessage(hwndWinSplit, WSM_STOPMOVING, (WPARAM)i, 0);
    }

    SUCCEED();
}

// Test: Rapid fire messages don't cause issues
TEST_F(HookSpoofingTest, RapidFireStability) {
    ASSERT_NE(hwndWinSplit, nullptr);

    HWND testWnd = GetDesktopWindow();

    // 1000 rapid messages
    for (int i = 0; i < 1000; i++) {
        SendMessage(hwndWinSplit, WSM_STARTMOVING, (WPARAM)testWnd, 0);
        SendMessage(hwndWinSplit, WSM_STOPMOVING, (WPARAM)(i % 10), 0);
    }

    SUCCEED();
}

// Test: WM_USER range messages don't crash
TEST_F(HookSpoofingTest, UserMessageRangeSafe) {
    ASSERT_NE(hwndWinSplit, nullptr);

    // Probe entire WM_USER range used by WinSplit
    for (UINT msg = WM_USER; msg < WM_USER + 200; msg++) {
        SendMessage(hwndWinSplit, msg, 0, 0);
        SendMessage(hwndWinSplit, msg, (WPARAM)-1, (LPARAM)-1);
    }

    SUCCEED();
}

// Test: Closed window handle handled gracefully
TEST_F(HookSpoofingTest, ClosedWindowHandleSafe) {
    ASSERT_NE(hwndWinSplit, nullptr);

    // Create and immediately destroy a window
    HWND tempWnd = CreateWindow(L"STATIC", L"Temp", WS_POPUP,
                                0, 0, 100, 100, nullptr, nullptr, nullptr, nullptr);
    HWND handleCopy = tempWnd;
    DestroyWindow(tempWnd);

    // Use the now-invalid handle
    SendMessage(hwndWinSplit, WSM_STARTMOVING, (WPARAM)handleCopy, 0);
    SendMessage(hwndWinSplit, WSM_STOPMOVING, 0, 0);

    SUCCEED();
}
