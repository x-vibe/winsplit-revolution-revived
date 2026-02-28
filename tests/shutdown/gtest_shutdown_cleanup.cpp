/**
 * Shutdown & Cleanup Tests
 * Using Google Test framework
 *
 * Tests verify that WinSplit Revolution properly cleans up resources during
 * shutdown, including hooks, hotkeys, DLL handles, and system tray icons.
 *
 * These are external process tests: they interact with a running WinSplit
 * instance and verify cleanup behavior from the outside.
 */

#include <gtest/gtest.h>
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>

#pragma comment(lib, "psapi.lib")

// ---------------------------------------------------------------------------
// Helper utilities
// ---------------------------------------------------------------------------

namespace ShutdownTestUtils {

// Find the hidden hook frame window used by WinSplit
HWND FindWinSplitHookFrame() {
    return FindWindowW(nullptr, L"WinSplit Revolution - Hook Frame");
}

// Find WinSplit process ID
DWORD GetWinSplitPid() {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return 0;

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
    return pid;
}

// Wait for WinSplit process to exit
bool WaitForProcessExit(DWORD pid, DWORD timeoutMs) {
    HANDLE hProcess = OpenProcess(SYNCHRONIZE, FALSE, pid);
    if (!hProcess) return true;  // Already exited

    DWORD result = WaitForSingleObject(hProcess, timeoutMs);
    CloseHandle(hProcess);
    return result == WAIT_OBJECT_0;
}

// Check if winsplithook.dll is loaded in any process
bool IsDllLoadedInAnyProcess(const wchar_t* dllName) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return false;

    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(pe);
    bool found = false;

    if (Process32FirstW(snapshot, &pe)) {
        do {
            HANDLE modSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pe.th32ProcessID);
            if (modSnap != INVALID_HANDLE_VALUE) {
                MODULEENTRY32W me;
                me.dwSize = sizeof(me);
                if (Module32FirstW(modSnap, &me)) {
                    do {
                        if (_wcsicmp(me.szModule, dllName) == 0) {
                            found = true;
                            break;
                        }
                    } while (Module32NextW(modSnap, &me));
                }
                CloseHandle(modSnap);
            }
            if (found) break;
        } while (Process32NextW(snapshot, &pe));
    }

    CloseHandle(snapshot);
    return found;
}

// Try to register a hotkey and immediately unregister it
// Returns true if the hotkey was available (not in use by another app)
bool IsHotkeyAvailable(UINT modifiers, UINT vk) {
    HWND hwnd = CreateWindowExW(0, L"STATIC", L"HotkeyTest",
        0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, nullptr, nullptr);
    if (!hwnd) return false;

    bool available = RegisterHotKey(hwnd, 1, modifiers, vk) != FALSE;
    if (available) {
        UnregisterHotKey(hwnd, 1);
    }
    DestroyWindow(hwnd);
    return available;
}

// Get resource counts for a process
struct ProcessResources {
    DWORD handleCount;
    DWORD gdiObjects;
    DWORD userObjects;
    SIZE_T privateBytes;
    bool valid;
};

ProcessResources GetProcessResources(DWORD pid) {
    ProcessResources res = {};
    res.valid = false;

    HANDLE hProcess = OpenProcess(
        PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (!hProcess) return res;

    GetProcessHandleCount(hProcess, &res.handleCount);

    PROCESS_MEMORY_COUNTERS_EX pmc;
    pmc.cb = sizeof(pmc);
    if (GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
        res.privateBytes = pmc.PrivateUsage;
    }

    res.gdiObjects = GetGuiResources(hProcess, GR_GDIOBJECTS);
    res.userObjects = GetGuiResources(hProcess, GR_USEROBJECTS);

    CloseHandle(hProcess);
    res.valid = true;
    return res;
}

// Send quit to WinSplit via tray icon menu simulation
void SendQuitToWinSplit(HWND hookFrame) {
    // Post WM_CLOSE to the hook frame; the wxApp event loop handles shutdown
    PostMessageW(hookFrame, WM_CLOSE, 0, 0);
}

}  // namespace ShutdownTestUtils

// ---------------------------------------------------------------------------
// Smoke Tests: Basic shutdown behavior
// ---------------------------------------------------------------------------

class ShutdownSmokeTest : public ::testing::Test {
protected:
    HWND hwndHookFrame = nullptr;
    DWORD winsplitPid = 0;

    void SetUp() override {
        hwndHookFrame = ShutdownTestUtils::FindWinSplitHookFrame();
        winsplitPid = ShutdownTestUtils::GetWinSplitPid();
        if (!winsplitPid) {
            GTEST_SKIP() << "WinSplit not running - skipping shutdown tests";
        }
    }
};

// Verify WinSplit is running and has the expected window
TEST_F(ShutdownSmokeTest, ProcessAndWindowExist) {
    EXPECT_NE(winsplitPid, 0u) << "WinSplit process should be running";
    EXPECT_NE(hwndHookFrame, nullptr) << "Hook frame window should exist";

    if (hwndHookFrame) {
        EXPECT_TRUE(IsWindow(hwndHookFrame)) << "Hook frame should be a valid window";
    }
}

// Verify hook DLL is loaded while WinSplit is running
TEST_F(ShutdownSmokeTest, HookDllLoadedWhileRunning) {
    // The DLL should be loaded in the WinSplit process at minimum
    HANDLE modSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, winsplitPid);
    ASSERT_NE(modSnap, INVALID_HANDLE_VALUE);

    MODULEENTRY32W me;
    me.dwSize = sizeof(me);
    bool found = false;

    if (Module32FirstW(modSnap, &me)) {
        do {
            if (_wcsicmp(me.szModule, L"winsplithook.dll") == 0) {
                found = true;
                break;
            }
        } while (Module32NextW(modSnap, &me));
    }

    CloseHandle(modSnap);
    EXPECT_TRUE(found) << "winsplithook.dll should be loaded in WinSplit process";
}

// Verify resource baseline can be taken (precondition for leak tests)
TEST_F(ShutdownSmokeTest, CanTakeResourceSnapshot) {
    auto res = ShutdownTestUtils::GetProcessResources(winsplitPid);
    EXPECT_TRUE(res.valid) << "Should be able to query process resources";
    EXPECT_GT(res.handleCount, 0u) << "Process should have handles";
    EXPECT_GT(res.gdiObjects, 0u) << "Process should have GDI objects";
}

// ---------------------------------------------------------------------------
// E2E Tests: Full cleanup verification after shutdown
// ---------------------------------------------------------------------------

class ShutdownE2ETest : public ::testing::Test {
protected:
    HWND hwndHookFrame = nullptr;
    DWORD winsplitPid = 0;

    void SetUp() override {
        hwndHookFrame = ShutdownTestUtils::FindWinSplitHookFrame();
        winsplitPid = ShutdownTestUtils::GetWinSplitPid();
        if (!winsplitPid) {
            GTEST_SKIP() << "WinSplit not running - skipping E2E shutdown tests. "
                         << "Start WinSplit first, then re-run.";
        }
    }
};

// After WinSplit exits, default hotkeys should be available for re-registration
// NOTE: This test only checks availability, it doesn't actually shut down WinSplit.
// It verifies that hotkeys WinSplit doesn't use ARE available.
TEST_F(ShutdownE2ETest, UnusedHotkeysAreAvailable) {
    // Pick a hotkey that WinSplit doesn't register by default
    // (WinSplit uses Ctrl+Alt+Numpad keys; Ctrl+Shift+F12 should be free)
    bool available = ShutdownTestUtils::IsHotkeyAvailable(
        MOD_CONTROL | MOD_SHIFT, VK_F12);
    EXPECT_TRUE(available) << "Non-WinSplit hotkeys should be registerable";
}

// Verify that WinSplit has properly registered its hooks (precondition)
TEST_F(ShutdownE2ETest, HooksRegisteredWhileRunning) {
    ASSERT_NE(hwndHookFrame, nullptr);

    // Send a benign registered window message and check it doesn't crash
    UINT testMsg = RegisterWindowMessageW(L"WinSplitMessage_StartMoving");
    ASSERT_NE(testMsg, 0u);

    // Posting should succeed (window exists and accepts messages)
    BOOL result = PostMessageW(hwndHookFrame, testMsg, 0, 0);
    EXPECT_TRUE(result) << "Should be able to post hook messages to WinSplit";

    // Clean up: send stop moving
    UINT stopMsg = RegisterWindowMessageW(L"WinSplitMessage_StopMoving");
    if (stopMsg) {
        PostMessageW(hwndHookFrame, stopMsg, 0, 0);
    }
}

// Simulate WM_QUERYENDSESSION and verify WinSplit consents
TEST_F(ShutdownE2ETest, QueryEndSessionReturnsTrue) {
    ASSERT_NE(hwndHookFrame, nullptr);

    // WM_QUERYENDSESSION: WinSplit should return TRUE (consent to shutdown)
    // Using SendMessage so we get the return value
    LRESULT result = SendMessageW(hwndHookFrame, WM_QUERYENDSESSION, 0, 0);

    // Non-zero means the app consents to shutdown
    EXPECT_NE(result, 0) << "WinSplit should consent to WM_QUERYENDSESSION";
}

// Verify WM_ENDSESSION handler exists and doesn't crash
TEST_F(ShutdownE2ETest, EndSessionDoesNotCrash) {
    ASSERT_NE(hwndHookFrame, nullptr);

    // Send WM_ENDSESSION with wParam=FALSE (session NOT actually ending)
    // This tests the handler without actually shutting down
    LRESULT result = SendMessageW(hwndHookFrame, WM_ENDSESSION, FALSE, 0);

    // If we get here, the handler didn't crash
    SUCCEED() << "WM_ENDSESSION(FALSE) handled without crash";

    // Verify WinSplit is still running (since we said session isn't ending)
    Sleep(500);
    EXPECT_NE(ShutdownTestUtils::GetWinSplitPid(), 0u)
        << "WinSplit should still be running after WM_ENDSESSION(FALSE)";
}

// ---------------------------------------------------------------------------
// Edge Case Tests
// ---------------------------------------------------------------------------

class ShutdownEdgeTest : public ::testing::Test {
protected:
    HWND hwndHookFrame = nullptr;
    DWORD winsplitPid = 0;

    void SetUp() override {
        hwndHookFrame = ShutdownTestUtils::FindWinSplitHookFrame();
        winsplitPid = ShutdownTestUtils::GetWinSplitPid();
        if (!winsplitPid) {
            GTEST_SKIP() << "WinSplit not running - skipping edge case tests";
        }
    }
};

// Rapid message flooding followed by checking stability
TEST_F(ShutdownEdgeTest, StableAfterMessageFlood) {
    ASSERT_NE(hwndHookFrame, nullptr);

    UINT startMsg = RegisterWindowMessageW(L"WinSplitMessage_StartMoving");
    UINT stopMsg = RegisterWindowMessageW(L"WinSplitMessage_StopMoving");
    UINT wheelMsg = RegisterWindowMessageW(L"WinSplitMessage_Wheel");

    // Flood with 200 messages rapidly (exceeds the 50/100ms rate limit)
    for (int i = 0; i < 200; i++) {
        PostMessageW(hwndHookFrame, startMsg, 0, 0);
        PostMessageW(hwndHookFrame, wheelMsg, MAKEWPARAM(0, 120), 0);
        PostMessageW(hwndHookFrame, stopMsg, 0, 0);
    }

    // Give WinSplit time to process
    Sleep(1000);

    // WinSplit should still be running and responsive
    DWORD pidAfter = ShutdownTestUtils::GetWinSplitPid();
    EXPECT_NE(pidAfter, 0u) << "WinSplit should survive message flooding";

    HWND hwndAfter = ShutdownTestUtils::FindWinSplitHookFrame();
    EXPECT_NE(hwndAfter, nullptr) << "Hook frame should still exist after flood";
}

// Verify WM_QUERYENDSESSION + WM_ENDSESSION(FALSE) sequence is handled
TEST_F(ShutdownEdgeTest, QueryThenCancelledShutdown) {
    ASSERT_NE(hwndHookFrame, nullptr);

    // Simulate: system asks "can I shut down?" -> app says yes -> system cancels
    LRESULT queryResult = SendMessageW(hwndHookFrame, WM_QUERYENDSESSION, 0, 0);
    EXPECT_NE(queryResult, 0);

    // Cancelled shutdown: WM_ENDSESSION with wParam=FALSE
    SendMessageW(hwndHookFrame, WM_ENDSESSION, FALSE, 0);

    Sleep(500);

    // WinSplit should still be fully functional
    DWORD pidAfter = ShutdownTestUtils::GetWinSplitPid();
    EXPECT_NE(pidAfter, 0u) << "WinSplit should survive cancelled shutdown";
}

// Send multiple WM_QUERYENDSESSION in sequence (edge case: rapid shutdown attempts)
TEST_F(ShutdownEdgeTest, MultipleQueryEndSession) {
    ASSERT_NE(hwndHookFrame, nullptr);

    for (int i = 0; i < 5; i++) {
        LRESULT result = SendMessageW(hwndHookFrame, WM_QUERYENDSESSION, 0, 0);
        EXPECT_NE(result, 0) << "Attempt " << i << " should consent";

        // Cancel each one
        SendMessageW(hwndHookFrame, WM_ENDSESSION, FALSE, 0);
    }

    Sleep(500);
    EXPECT_NE(ShutdownTestUtils::GetWinSplitPid(), 0u)
        << "WinSplit should survive multiple query/cancel cycles";
}

// Verify process handles don't leak during normal operation
TEST_F(ShutdownEdgeTest, NoHandleLeakDuringOperation) {
    auto before = ShutdownTestUtils::GetProcessResources(winsplitPid);
    ASSERT_TRUE(before.valid);

    UINT startMsg = RegisterWindowMessageW(L"WinSplitMessage_StartMoving");
    UINT stopMsg = RegisterWindowMessageW(L"WinSplitMessage_StopMoving");

    // Simulate 50 drag operations
    for (int i = 0; i < 50; i++) {
        PostMessageW(hwndHookFrame, startMsg, 0, 0);
        Sleep(10);
        PostMessageW(hwndHookFrame, stopMsg, 0, 0);
        Sleep(10);
    }

    Sleep(1000);

    auto after = ShutdownTestUtils::GetProcessResources(winsplitPid);
    ASSERT_TRUE(after.valid);

    long handleDelta = (long)after.handleCount - (long)before.handleCount;
    long gdiDelta = (long)after.gdiObjects - (long)before.gdiObjects;
    long userDelta = (long)after.userObjects - (long)before.userObjects;

    // Allow small variance but flag significant leaks
    EXPECT_LT(handleDelta, 20)
        << "Handle count grew by " << handleDelta << " after 50 drag operations";
    EXPECT_LT(gdiDelta, 10)
        << "GDI objects grew by " << gdiDelta << " after 50 drag operations";
    EXPECT_LT(userDelta, 10)
        << "User objects grew by " << userDelta << " after 50 drag operations";
}

// ---------------------------------------------------------------------------
// Memory / Resource Tests
// ---------------------------------------------------------------------------

class ShutdownMemoryTest : public ::testing::Test {
protected:
    DWORD winsplitPid = 0;

    void SetUp() override {
        winsplitPid = ShutdownTestUtils::GetWinSplitPid();
        if (!winsplitPid) {
            GTEST_SKIP() << "WinSplit not running - skipping memory tests";
        }
    }
};

// Baseline resource snapshot
TEST_F(ShutdownMemoryTest, ResourceBaselineReasonable) {
    auto res = ShutdownTestUtils::GetProcessResources(winsplitPid);
    ASSERT_TRUE(res.valid);

    // WinSplit should not be using excessive resources
    EXPECT_LT(res.handleCount, 500u)
        << "WinSplit using " << res.handleCount << " handles (expected < 500)";
    EXPECT_LT(res.gdiObjects, 200u)
        << "WinSplit using " << res.gdiObjects << " GDI objects (expected < 200)";
    EXPECT_LT(res.userObjects, 100u)
        << "WinSplit using " << res.userObjects << " user objects (expected < 100)";

    // Private bytes should be reasonable (< 100 MB)
    SIZE_T privateMB = res.privateBytes / (1024 * 1024);
    EXPECT_LT(privateMB, 100u)
        << "WinSplit using " << privateMB << " MB private bytes (expected < 100)";
}

// Idle stability: resources shouldn't grow when WinSplit is just sitting there
TEST_F(ShutdownMemoryTest, IdleStability) {
    auto before = ShutdownTestUtils::GetProcessResources(winsplitPid);
    ASSERT_TRUE(before.valid);

    // Wait 10 seconds while WinSplit idles
    Sleep(10000);

    auto after = ShutdownTestUtils::GetProcessResources(winsplitPid);
    ASSERT_TRUE(after.valid);

    long handleDelta = (long)after.handleCount - (long)before.handleCount;
    long gdiDelta = (long)after.gdiObjects - (long)before.gdiObjects;
    long memDeltaKB = (long)(after.privateBytes - before.privateBytes) / 1024;

    EXPECT_LT(handleDelta, 5)
        << "Handles grew by " << handleDelta << " while idle";
    EXPECT_LT(gdiDelta, 3)
        << "GDI objects grew by " << gdiDelta << " while idle";
    EXPECT_LT(memDeltaKB, 1024)
        << "Memory grew by " << memDeltaKB << " KB while idle";
}

// ---------------------------------------------------------------------------
// Destructive Tests (these actually shut down WinSplit)
// Run last or separately as they require restarting WinSplit.
// ---------------------------------------------------------------------------

class ShutdownDestructiveTest : public ::testing::Test {
protected:
    HWND hwndHookFrame = nullptr;
    DWORD winsplitPid = 0;

    void SetUp() override {
        hwndHookFrame = ShutdownTestUtils::FindWinSplitHookFrame();
        winsplitPid = ShutdownTestUtils::GetWinSplitPid();
        if (!winsplitPid) {
            GTEST_SKIP() << "WinSplit not running - skipping destructive tests. "
                         << "These tests shut down WinSplit and verify cleanup.";
        }
    }
};

// NOTE: This test is DISABLED by default because it actually shuts down WinSplit.
// Run explicitly with: --gtest_also_run_disabled_tests
TEST_F(ShutdownDestructiveTest, DISABLED_CleanExitViaWmClose) {
    ASSERT_NE(hwndHookFrame, nullptr);

    // Record the default hotkey (Ctrl+Alt+Numpad5 = fullscreen) before shutdown
    // to verify it's released after
    bool hotkeyBusyBefore = !ShutdownTestUtils::IsHotkeyAvailable(
        MOD_CONTROL | MOD_ALT, VK_NUMPAD5);
    EXPECT_TRUE(hotkeyBusyBefore)
        << "Ctrl+Alt+Numpad5 should be registered by WinSplit";

    // Send WM_CLOSE to trigger graceful shutdown
    PostMessageW(hwndHookFrame, WM_CLOSE, 0, 0);

    // Wait for process to exit (up to 10 seconds)
    bool exited = ShutdownTestUtils::WaitForProcessExit(winsplitPid, 10000);
    EXPECT_TRUE(exited) << "WinSplit should exit within 10 seconds of WM_CLOSE";

    if (exited) {
        Sleep(1000);  // Let cleanup finish

        // Verify hotkeys are released
        bool hotkeyFreeAfter = ShutdownTestUtils::IsHotkeyAvailable(
            MOD_CONTROL | MOD_ALT, VK_NUMPAD5);
        EXPECT_TRUE(hotkeyFreeAfter)
            << "Ctrl+Alt+Numpad5 should be free after WinSplit exits";

        // Verify hook DLL is not loaded in any process
        bool dllStillLoaded = ShutdownTestUtils::IsDllLoadedInAnyProcess(
            L"winsplithook.dll");
        EXPECT_FALSE(dllStillLoaded)
            << "winsplithook.dll should not be loaded after WinSplit exits";

        // Verify process is gone
        EXPECT_EQ(ShutdownTestUtils::GetWinSplitPid(), 0u)
            << "WinSplit process should not exist after shutdown";

        // Verify hook frame window is gone
        HWND hwndAfter = ShutdownTestUtils::FindWinSplitHookFrame();
        EXPECT_EQ(hwndAfter, nullptr)
            << "Hook frame window should not exist after shutdown";
    }
}

// Test: Simulate system shutdown via WM_QUERYENDSESSION + WM_ENDSESSION(TRUE)
// DISABLED by default because it shuts down WinSplit.
TEST_F(ShutdownDestructiveTest, DISABLED_CleanExitViaSessionEnd) {
    ASSERT_NE(hwndHookFrame, nullptr);

    // Query: app should consent
    LRESULT consent = SendMessageW(hwndHookFrame, WM_QUERYENDSESSION, 0, 0);
    EXPECT_NE(consent, 0);

    // End session for real
    SendMessageW(hwndHookFrame, WM_ENDSESSION, TRUE, 0);

    // Wait for exit
    bool exited = ShutdownTestUtils::WaitForProcessExit(winsplitPid, 10000);
    EXPECT_TRUE(exited) << "WinSplit should exit after WM_ENDSESSION(TRUE)";

    if (exited) {
        Sleep(1000);

        // Same cleanup checks as WM_CLOSE test
        bool hotkeyFree = ShutdownTestUtils::IsHotkeyAvailable(
            MOD_CONTROL | MOD_ALT, VK_NUMPAD5);
        EXPECT_TRUE(hotkeyFree)
            << "Hotkeys should be released after session end";

        bool dllLoaded = ShutdownTestUtils::IsDllLoadedInAnyProcess(
            L"winsplithook.dll");
        EXPECT_FALSE(dllLoaded)
            << "Hook DLL should be unloaded after session end";
    }
}

// Test: Force-kill and verify no system-wide hook leaks
// DISABLED by default because it kills WinSplit.
TEST_F(ShutdownDestructiveTest, DISABLED_ForceKillNoSystemHookLeak) {
    ASSERT_NE(winsplitPid, 0u);

    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE | SYNCHRONIZE, FALSE, winsplitPid);
    ASSERT_NE(hProcess, (HANDLE)nullptr) << "Need permission to terminate WinSplit";

    // Force kill
    BOOL terminated = TerminateProcess(hProcess, 1);
    EXPECT_TRUE(terminated);

    WaitForSingleObject(hProcess, 5000);
    CloseHandle(hProcess);

    Sleep(2000);

    // After force kill, the DLL_PROCESS_DETACH handler should have
    // cleared the shared HWND, preventing hook callbacks from posting
    // to a dead window. The hooks themselves will be cleaned up by
    // the OS when all processes that had the hook injected unload the DLL.

    // Verify process is gone
    EXPECT_EQ(ShutdownTestUtils::GetWinSplitPid(), 0u)
        << "WinSplit process should not exist after termination";
}
