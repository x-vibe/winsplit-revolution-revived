/**
 * WinSplit Revolution Test Harness
 * Common testing framework for all tests
 */

#ifndef TEST_HARNESS_H
#define TEST_HARNESS_H

#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <functional>

// Console colors for output
#define COLOR_GREEN  10
#define COLOR_RED    12
#define COLOR_YELLOW 14
#define COLOR_WHITE  15

namespace TestHarness {

// Test result structure
struct TestResult {
    std::string name;
    bool passed;
    std::string message;
    DWORD duration_ms;
};

// Global test results
static std::vector<TestResult> g_results;
static int g_passed = 0;
static int g_failed = 0;

// Set console color
inline void SetColor(WORD color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

// Print with color
inline void PrintColored(const char* text, WORD color) {
    SetColor(color);
    printf("%s", text);
    SetColor(COLOR_WHITE);
}

// Initialize test suite
inline void Init(const char* suiteName) {
    g_results.clear();
    g_passed = 0;
    g_failed = 0;

    printf("\n");
    PrintColored("=== ", COLOR_WHITE);
    PrintColored(suiteName, COLOR_YELLOW);
    PrintColored(" ===\n\n", COLOR_WHITE);
}

// Record a test result
inline void RecordResult(const char* testName, bool passed, const char* message = "") {
    TestResult result;
    result.name = testName;
    result.passed = passed;
    result.message = message;
    result.duration_ms = 0;

    g_results.push_back(result);

    if (passed) {
        g_passed++;
        PrintColored("[PASS] ", COLOR_GREEN);
    } else {
        g_failed++;
        PrintColored("[FAIL] ", COLOR_RED);
    }

    printf("%s", testName);
    if (message && message[0]) {
        printf(" - %s", message);
    }
    printf("\n");
}

// Assertion macros
#define TEST_ASSERT(condition, testName) \
    TestHarness::RecordResult(testName, (condition), (condition) ? "" : "Assertion failed")

#define TEST_ASSERT_MSG(condition, testName, msg) \
    TestHarness::RecordResult(testName, (condition), (condition) ? "" : msg)

#define TEST_ASSERT_EQUAL(expected, actual, testName) \
    TestHarness::RecordResult(testName, (expected) == (actual), \
        ((expected) == (actual)) ? "" : "Values not equal")

#define TEST_ASSERT_NOT_NULL(ptr, testName) \
    TestHarness::RecordResult(testName, (ptr) != nullptr, \
        ((ptr) != nullptr) ? "" : "Pointer is null")

// Run a test function with timing
inline void RunTest(const char* testName, std::function<bool()> testFunc) {
    DWORD start = GetTickCount();
    bool result = false;

    try {
        result = testFunc();
    }
    catch (...) {
        RecordResult(testName, false, "Exception occurred");
        return;
    }

    DWORD duration = GetTickCount() - start;

    TestResult tr;
    tr.name = testName;
    tr.passed = result;
    tr.duration_ms = duration;

    if (result) {
        g_passed++;
        PrintColored("[PASS] ", COLOR_GREEN);
    } else {
        g_failed++;
        PrintColored("[FAIL] ", COLOR_RED);
    }

    printf("%s (%lu ms)\n", testName, duration);
    g_results.push_back(tr);
}

// Print summary and return exit code
inline int Summarize() {
    printf("\n");
    PrintColored("=== Summary ===\n", COLOR_YELLOW);

    int total = g_passed + g_failed;

    SetColor(COLOR_GREEN);
    printf("Passed: %d\n", g_passed);

    SetColor(COLOR_RED);
    printf("Failed: %d\n", g_failed);

    SetColor(COLOR_WHITE);
    printf("Total:  %d\n\n", total);

    if (g_failed == 0) {
        PrintColored("All tests passed!\n\n", COLOR_GREEN);
        return 0;
    } else {
        PrintColored("Some tests failed.\n\n", COLOR_RED);
        return 1;
    }
}

// Check if WinSplit is running
inline bool IsWinSplitRunning() {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return false;

    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(pe);

    bool found = false;
    if (Process32FirstW(snapshot, &pe)) {
        do {
            if (_wcsicmp(pe.szExeFile, L"WinSplit.exe") == 0) {
                found = true;
                break;
            }
        } while (Process32NextW(snapshot, &pe));
    }

    CloseHandle(snapshot);
    return found;
}

// Get WinSplit process ID
inline DWORD GetWinSplitProcessId() {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return 0;

    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(pe);

    DWORD pid = 0;
    if (Process32FirstW(snapshot, &pe)) {
        do {
            if (_wcsicmp(pe.szExeFile, L"WinSplit.exe") == 0) {
                pid = pe.th32ProcessID;
                break;
            }
        } while (Process32NextW(snapshot, &pe));
    }

    CloseHandle(snapshot);
    return pid;
}

// Find WinSplit main window (FrameHook has an empty title, find by process)
inline HWND FindWinSplitWindow() {
    DWORD pid = GetWinSplitProcessId();
    if (!pid) return NULL;

    struct EnumData { DWORD pid; HWND result; };
    EnumData data = { pid, NULL };

    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
        auto* d = reinterpret_cast<EnumData*>(lParam);
        DWORD windowPid = 0;
        GetWindowThreadProcessId(hwnd, &windowPid);
        if (windowPid != d->pid) return TRUE;
        LONG exStyle = GetWindowLongW(hwnd, GWL_EXSTYLE);
        if (exStyle & WS_EX_TOOLWINDOW) {
            d->result = hwnd;
            return FALSE;
        }
        return TRUE;
    }, reinterpret_cast<LPARAM>(&data));

    return data.result;
}

// Wait for window to appear with timeout
inline HWND WaitForWindow(const wchar_t* className, const wchar_t* windowName, DWORD timeoutMs) {
    DWORD start = GetTickCount();
    HWND hwnd = NULL;

    while (GetTickCount() - start < timeoutMs) {
        hwnd = FindWindow(className, windowName);
        if (hwnd) break;
        Sleep(100);
    }

    return hwnd;
}

// Compare rectangles with tolerance
inline bool RectsEqual(const RECT& a, const RECT& b, int tolerance = 0) {
    return abs(a.left - b.left) <= tolerance &&
           abs(a.top - b.top) <= tolerance &&
           abs(a.right - b.right) <= tolerance &&
           abs(a.bottom - b.bottom) <= tolerance;
}

// Get work area of primary monitor
inline RECT GetPrimaryWorkArea() {
    RECT rc;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, 0);
    return rc;
}

// Get work area of monitor containing a point
inline RECT GetMonitorWorkArea(POINT pt) {
    HMONITOR hMon = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi;
    mi.cbSize = sizeof(mi);
    GetMonitorInfo(hMon, &mi);
    return mi.rcWork;
}

} // namespace TestHarness

#endif // TEST_HARNESS_H
