/**
 * Memory Leak Detection Test
 *
 * Monitors WinSplit for memory and handle leaks during extended operation.
 * Can integrate with external tools like Dr. Memory and Application Verifier.
 */

#include "../tools/test_harness.h"
#include <stdio.h>
#include <psapi.h>
#include <vector>
#include <string>

#pragma comment(lib, "psapi.lib")

// Snapshot of process resources
struct ResourceSnapshot {
    DWORD timestamp;
    SIZE_T workingSet;
    SIZE_T privateBytes;
    SIZE_T virtualSize;
    DWORD handleCount;
    DWORD gdiObjects;
    DWORD userObjects;
};

std::vector<ResourceSnapshot> g_snapshots;

// Take a resource snapshot
bool TakeSnapshot(DWORD pid) {
    HANDLE hProcess = OpenProcess(
        PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
        FALSE, pid
    );
    if (!hProcess) return false;

    ResourceSnapshot snap = {};
    snap.timestamp = GetTickCount();

    // Memory info
    PROCESS_MEMORY_COUNTERS_EX pmc;
    pmc.cb = sizeof(pmc);
    if (GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
        snap.workingSet = pmc.WorkingSetSize;
        snap.privateBytes = pmc.PrivateUsage;
    }

    // Handle count
    GetProcessHandleCount(hProcess, &snap.handleCount);

    CloseHandle(hProcess);

    // GDI and User objects (need different handle)
    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (hProcess) {
        snap.gdiObjects = GetGuiResources(hProcess, GR_GDIOBJECTS);
        snap.userObjects = GetGuiResources(hProcess, GR_USEROBJECTS);
        CloseHandle(hProcess);
    }

    g_snapshots.push_back(snap);
    return true;
}

// Print snapshot
void PrintSnapshot(const ResourceSnapshot& snap, const ResourceSnapshot* baseline = nullptr) {
    printf("  Time: +%lu ms\n", baseline ? snap.timestamp - baseline->timestamp : 0);
    printf("  Working Set:  %8zu KB", snap.workingSet / 1024);
    if (baseline) printf(" (%+zd)", (snap.workingSet - baseline->workingSet) / 1024);
    printf("\n");

    printf("  Private:      %8zu KB", snap.privateBytes / 1024);
    if (baseline) printf(" (%+zd)", (snap.privateBytes - baseline->privateBytes) / 1024);
    printf("\n");

    printf("  Handles:      %8lu", snap.handleCount);
    if (baseline) printf(" (%+ld)", (long)snap.handleCount - (long)baseline->handleCount);
    printf("\n");

    printf("  GDI Objects:  %8lu", snap.gdiObjects);
    if (baseline) printf(" (%+ld)", (long)snap.gdiObjects - (long)baseline->gdiObjects);
    printf("\n");

    printf("  User Objects: %8lu", snap.userObjects);
    if (baseline) printf(" (%+ld)", (long)snap.userObjects - (long)baseline->userObjects);
    printf("\n");
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

    for (int i = 0; i < count; i++) {
        inputs[i].ki.dwFlags = KEYEVENTF_KEYUP;
    }
    SendInput(count, inputs, sizeof(INPUT));
}

// Create test window
HWND CreateTestWindow() {
    static const wchar_t* className = L"MemLeakTestWindow";
    static bool registered = false;

    if (!registered) {
        WNDCLASS wc = {};
        wc.lpfnWndProc = DefWindowProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.lpszClassName = className;
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        RegisterClass(&wc);
        registered = true;
    }

    HWND hwnd = CreateWindow(className, L"Leak Test",
        WS_OVERLAPPEDWINDOW, 100, 100, 400, 300,
        NULL, NULL, GetModuleHandle(NULL), NULL);

    ShowWindow(hwnd, SW_SHOW);
    return hwnd;
}

// Test: Extended runtime monitoring
bool Test_ExtendedMonitoring() {
    DWORD pid = TestHarness::GetWinSplitProcessId();
    if (!pid) {
        printf("  WinSplit not running\n");
        return false;
    }

    printf("  Extended monitoring (60 seconds)...\n");
    printf("  Taking snapshots every 10 seconds.\n\n");

    g_snapshots.clear();

    // Create test window
    HWND hwnd = CreateTestWindow();
    SetForegroundWindow(hwnd);
    Sleep(100);

    // Baseline
    TakeSnapshot(pid);
    printf("  Baseline:\n");
    PrintSnapshot(g_snapshots[0]);

    DWORD start = GetTickCount();
    int iterations = 0;

    // Run for 60 seconds
    while (GetTickCount() - start < 60000) {
        // Simulate activity
        SimulateHotkey(MOD_CONTROL | MOD_ALT, VK_NUMPAD1 + (iterations % 9));
        iterations++;

        // Snapshot every 10 seconds
        if ((GetTickCount() - start) / 10000 > (g_snapshots.size() - 1)) {
            TakeSnapshot(pid);
            printf("\n  Snapshot %zu:\n", g_snapshots.size());
            PrintSnapshot(g_snapshots.back(), &g_snapshots[0]);
        }

        Sleep(10);
    }

    // Final snapshot
    TakeSnapshot(pid);
    printf("\n  Final:\n");
    PrintSnapshot(g_snapshots.back(), &g_snapshots[0]);

    DestroyWindow(hwnd);

    // Analyze for leaks
    const ResourceSnapshot& first = g_snapshots[0];
    const ResourceSnapshot& last = g_snapshots.back();

    bool hasLeak = false;

    // Check for steady growth
    long memGrowth = (long)(last.privateBytes - first.privateBytes) / 1024;
    long handleGrowth = (long)last.handleCount - (long)first.handleCount;
    long gdiGrowth = (long)last.gdiObjects - (long)first.gdiObjects;
    long userGrowth = (long)last.userObjects - (long)first.userObjects;

    printf("\n  Analysis:\n");

    if (memGrowth > 5120) {  // >5MB
        printf("  WARNING: Memory grew by %ld KB - possible leak!\n", memGrowth);
        hasLeak = true;
    } else {
        printf("  Memory growth: %ld KB - OK\n", memGrowth);
    }

    if (handleGrowth > 50) {
        printf("  WARNING: Handle count grew by %ld - leak!\n", handleGrowth);
        hasLeak = true;
    } else {
        printf("  Handle growth: %ld - OK\n", handleGrowth);
    }

    if (gdiGrowth > 20) {
        printf("  WARNING: GDI objects grew by %ld - leak!\n", gdiGrowth);
        hasLeak = true;
    } else {
        printf("  GDI growth: %ld - OK\n", gdiGrowth);
    }

    if (userGrowth > 20) {
        printf("  WARNING: User objects grew by %ld - leak!\n", userGrowth);
        hasLeak = true;
    } else {
        printf("  User growth: %ld - OK\n", userGrowth);
    }

    return !hasLeak;
}

// Test: Check for GDI leaks specifically
bool Test_GdiLeaks() {
    DWORD pid = TestHarness::GetWinSplitProcessId();
    if (!pid) return false;

    printf("  Testing GDI object leaks during window operations...\n");

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (!hProcess) return false;

    DWORD gdiStart = GetGuiResources(hProcess, GR_GDIOBJECTS);

    // Create and destroy many windows, triggering WinSplit
    for (int i = 0; i < 100; i++) {
        HWND hwnd = CreateTestWindow();
        SetForegroundWindow(hwnd);

        SimulateHotkey(MOD_CONTROL | MOD_ALT, VK_NUMPAD5);
        Sleep(20);

        DestroyWindow(hwnd);
    }

    Sleep(500);  // Let things settle

    DWORD gdiEnd = GetGuiResources(hProcess, GR_GDIOBJECTS);
    CloseHandle(hProcess);

    long delta = (long)gdiEnd - (long)gdiStart;
    printf("  GDI objects: %lu -> %lu (delta: %ld)\n", gdiStart, gdiEnd, delta);

    if (delta > 10) {
        printf("  WARNING: GDI leak detected!\n");
        return false;
    }

    return true;
}

// Test: Check for User object leaks
bool Test_UserObjectLeaks() {
    DWORD pid = TestHarness::GetWinSplitProcessId();
    if (!pid) return false;

    printf("  Testing User object leaks...\n");

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (!hProcess) return false;

    DWORD userStart = GetGuiResources(hProcess, GR_USEROBJECTS);

    // Trigger many operations
    HWND hwnd = CreateTestWindow();

    for (int i = 0; i < 500; i++) {
        SetForegroundWindow(hwnd);
        SimulateHotkey(MOD_CONTROL | MOD_ALT, VK_NUMPAD1 + (i % 9));
    }

    Sleep(500);

    DestroyWindow(hwnd);
    Sleep(500);

    DWORD userEnd = GetGuiResources(hProcess, GR_USEROBJECTS);
    CloseHandle(hProcess);

    long delta = (long)userEnd - (long)userStart;
    printf("  User objects: %lu -> %lu (delta: %ld)\n", userStart, userEnd, delta);

    if (delta > 10) {
        printf("  WARNING: User object leak detected!\n");
        return false;
    }

    return true;
}

// Test: Integration check for Dr. Memory
bool Test_DrMemoryIntegration() {
    printf("  Checking for Dr. Memory integration...\n");

    // Check if running under Dr. Memory
    HMODULE hDrmem = GetModuleHandle(L"dynamorio.dll");

    if (hDrmem) {
        printf("  Running under Dr. Memory - detailed leak report will be generated.\n");
        return true;
    }

    printf("  Not running under Dr. Memory.\n");
    printf("  For detailed leak detection, run:\n");
    printf("    drmemory.exe -light -- WinSplit.exe\n");

    return true;
}

int main() {
    TestHarness::Init("Memory Leak Detection Test");

    if (!TestHarness::IsWinSplitRunning()) {
        printf("ERROR: WinSplit not running.\n");
        printf("Please start WinSplit before running this test.\n");
        return 1;
    }

    printf("Monitoring WinSplit for memory and handle leaks...\n\n");

    // Run tests
    TestHarness::RunTest("Extended monitoring (60 sec)", Test_ExtendedMonitoring);
    TestHarness::RunTest("GDI object leaks", Test_GdiLeaks);
    TestHarness::RunTest("User object leaks", Test_UserObjectLeaks);
    TestHarness::RunTest("Dr. Memory integration", Test_DrMemoryIntegration);

    printf("\n");
    TestHarness::PrintColored("RECOMMENDATIONS:\n", COLOR_YELLOW);
    printf("For thorough leak detection:\n");
    printf("1. Use Dr. Memory: drmemory.exe -light -- WinSplit.exe\n");
    printf("2. Use Application Verifier from Windows SDK\n");
    printf("3. Use Visual Studio's built-in memory profiler\n\n");

    return TestHarness::Summarize();
}
