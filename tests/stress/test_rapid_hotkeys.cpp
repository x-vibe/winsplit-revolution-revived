/**
 * Rapid Hotkey Stress Test
 *
 * Stress tests WinSplit's hotkey handling by:
 * - Firing hotkeys in rapid succession
 * - Testing concurrent operations
 * - Monitoring for crashes and memory issues
 */

#include "../tools/test_harness.h"
#include <stdio.h>
#include <psapi.h>
#include <vector>
#include <thread>
#include <atomic>

#pragma comment(lib, "psapi.lib")

// Test configuration
#define RAPID_FIRE_COUNT    1000
#define CONCURRENT_WINDOWS  10
#define STRESS_DURATION_MS  30000

std::atomic<bool> g_stopStress(false);
std::atomic<int> g_hotkeysSent(0);

// Create a test window
HWND CreateTestWindow(int x, int y, int w, int h, int id) {
    static bool registered = false;
    static const wchar_t* className = L"WinSplitStressTest";

    if (!registered) {
        WNDCLASS wc = {};
        wc.lpfnWndProc = DefWindowProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.lpszClassName = className;
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        RegisterClass(&wc);
        registered = true;
    }

    wchar_t title[64];
    swprintf(title, 64, L"Stress Test Window %d", id);

    HWND hwnd = CreateWindowEx(
        0, className, title,
        WS_OVERLAPPEDWINDOW,
        x, y, w, h,
        NULL, NULL, GetModuleHandle(NULL), NULL
    );

    ShowWindow(hwnd, SW_SHOW);
    return hwnd;
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

    g_hotkeysSent++;
}

// Get process memory usage
SIZE_T GetProcessMemory(DWORD pid) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (!hProcess) return 0;

    PROCESS_MEMORY_COUNTERS pmc;
    SIZE_T mem = 0;
    if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
        mem = pmc.WorkingSetSize;
    }

    CloseHandle(hProcess);
    return mem;
}

// Get process handle count
DWORD GetProcessHandles(DWORD pid) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (!hProcess) return 0;

    DWORD handles = 0;
    GetProcessHandleCount(hProcess, &handles);

    CloseHandle(hProcess);
    return handles;
}

// Test: Rapid fire same hotkey
bool Test_RapidFireSameHotkey() {
    printf("  Firing Ctrl+Alt+5 %d times rapidly...\n", RAPID_FIRE_COUNT);

    HWND hwnd = CreateTestWindow(100, 100, 400, 300, 0);
    if (!hwnd) return false;

    SetForegroundWindow(hwnd);
    Sleep(100);

    DWORD start = GetTickCount();

    for (int i = 0; i < RAPID_FIRE_COUNT; i++) {
        SimulateHotkey(MOD_CONTROL | MOD_ALT, VK_NUMPAD5);
        // No sleep - maximum stress
    }

    DWORD elapsed = GetTickCount() - start;

    printf("  Sent %d hotkeys in %lu ms (%.1f/sec)\n",
           RAPID_FIRE_COUNT, elapsed,
           (float)RAPID_FIRE_COUNT * 1000.0f / elapsed);

    Sleep(500);  // Let things settle
    DestroyWindow(hwnd);

    return true;
}

// Test: Rapid fire different hotkeys
bool Test_RapidFireDifferentHotkeys() {
    printf("  Firing all numpad hotkeys %d times each...\n", RAPID_FIRE_COUNT / 9);

    HWND hwnd = CreateTestWindow(100, 100, 400, 300, 0);
    if (!hwnd) return false;

    SetForegroundWindow(hwnd);
    Sleep(100);

    WORD keys[] = { VK_NUMPAD1, VK_NUMPAD2, VK_NUMPAD3, VK_NUMPAD4,
                    VK_NUMPAD5, VK_NUMPAD6, VK_NUMPAD7, VK_NUMPAD8, VK_NUMPAD9 };

    DWORD start = GetTickCount();

    for (int i = 0; i < RAPID_FIRE_COUNT; i++) {
        SimulateHotkey(MOD_CONTROL | MOD_ALT, keys[i % 9]);
    }

    DWORD elapsed = GetTickCount() - start;

    printf("  Sent %d varied hotkeys in %lu ms\n", RAPID_FIRE_COUNT, elapsed);

    Sleep(500);
    DestroyWindow(hwnd);

    return true;
}

// Test: Multiple windows concurrent
bool Test_MultipleWindowsConcurrent() {
    printf("  Creating %d windows and moving them simultaneously...\n", CONCURRENT_WINDOWS);

    std::vector<HWND> windows;

    // Create windows
    for (int i = 0; i < CONCURRENT_WINDOWS; i++) {
        HWND hwnd = CreateTestWindow(
            50 + (i % 5) * 100,
            50 + (i / 5) * 100,
            200, 150, i
        );
        if (hwnd) windows.push_back(hwnd);
    }

    printf("  Created %zu windows\n", windows.size());

    // Rapid hotkeys while switching between windows
    DWORD start = GetTickCount();

    for (int round = 0; round < 100; round++) {
        for (size_t i = 0; i < windows.size(); i++) {
            SetForegroundWindow(windows[i]);
            SimulateHotkey(MOD_CONTROL | MOD_ALT, VK_NUMPAD1 + (round % 9));
        }
    }

    DWORD elapsed = GetTickCount() - start;

    printf("  %d hotkeys across %zu windows in %lu ms\n",
           100 * (int)windows.size(), windows.size(), elapsed);

    // Cleanup
    for (HWND hwnd : windows) {
        DestroyWindow(hwnd);
    }

    return true;
}

// Test: Monitor WinSplit memory during stress
bool Test_MemoryStability() {
    DWORD pid = TestHarness::GetWinSplitProcessId();
    if (!pid) {
        printf("  WinSplit not running\n");
        return false;
    }

    printf("  Monitoring WinSplit memory during stress (30 sec)...\n");

    SIZE_T memStart = GetProcessMemory(pid);
    DWORD handlesStart = GetProcessHandles(pid);

    printf("  Initial: Memory=%zu KB, Handles=%lu\n",
           memStart / 1024, handlesStart);

    // Create test window
    HWND hwnd = CreateTestWindow(100, 100, 400, 300, 0);
    SetForegroundWindow(hwnd);
    Sleep(100);

    // Stress for 30 seconds
    DWORD start = GetTickCount();
    int iterations = 0;

    while (GetTickCount() - start < STRESS_DURATION_MS) {
        SimulateHotkey(MOD_CONTROL | MOD_ALT, VK_NUMPAD1 + (iterations % 9));
        iterations++;

        // Check memory every 1000 iterations
        if (iterations % 1000 == 0) {
            SIZE_T mem = GetProcessMemory(pid);
            DWORD handles = GetProcessHandles(pid);
            printf("  @%d: Memory=%zu KB (+%zd), Handles=%lu (+%ld)\n",
                   iterations, mem / 1024,
                   (mem - memStart) / 1024,
                   handles, handles - handlesStart);
        }

        Sleep(1);  // Small delay to not overwhelm
    }

    SIZE_T memEnd = GetProcessMemory(pid);
    DWORD handlesEnd = GetProcessHandles(pid);

    printf("  Final: Memory=%zu KB, Handles=%lu\n",
           memEnd / 1024, handlesEnd);

    long memDelta = (long)((memEnd - memStart) / 1024);
    long handleDelta = (long)(handlesEnd - handlesStart);

    printf("  Delta: Memory=%+ld KB, Handles=%+ld\n", memDelta, handleDelta);

    DestroyWindow(hwnd);

    // Warn if significant growth
    bool memOk = memDelta < 10240;  // Less than 10MB growth
    bool handlesOk = handleDelta < 100;

    if (!memOk) {
        printf("  WARNING: Significant memory growth detected!\n");
    }
    if (!handlesOk) {
        printf("  WARNING: Handle leak detected!\n");
    }

    return memOk && handlesOk;
}

// Test: Stress with rapid window creation/destruction
bool Test_WindowCreateDestroyStress() {
    printf("  Creating/destroying windows during hotkey stress...\n");

    DWORD start = GetTickCount();
    int windowsCreated = 0;

    while (GetTickCount() - start < 10000) {  // 10 seconds
        HWND hwnd = CreateTestWindow(
            rand() % 800, rand() % 600,
            200 + rand() % 200, 150 + rand() % 150,
            windowsCreated
        );

        if (hwnd) {
            SetForegroundWindow(hwnd);

            // Random hotkeys
            for (int i = 0; i < 10; i++) {
                SimulateHotkey(MOD_CONTROL | MOD_ALT, VK_NUMPAD1 + rand() % 9);
            }

            DestroyWindow(hwnd);
            windowsCreated++;
        }
    }

    printf("  Created/destroyed %d windows with hotkeys\n", windowsCreated);

    return windowsCreated > 0;
}

int main() {
    TestHarness::Init("Rapid Hotkey Stress Test");

    if (!TestHarness::IsWinSplitRunning()) {
        printf("ERROR: WinSplit not running.\n");
        return 1;
    }

    printf("Running stress tests...\n");
    printf("NOTE: These tests may take several minutes.\n\n");

    // Run tests
    TestHarness::RunTest("Rapid fire same hotkey", Test_RapidFireSameHotkey);
    TestHarness::RunTest("Rapid fire different hotkeys", Test_RapidFireDifferentHotkeys);
    TestHarness::RunTest("Multiple windows concurrent", Test_MultipleWindowsConcurrent);
    TestHarness::RunTest("Memory stability (30 sec)", Test_MemoryStability);
    TestHarness::RunTest("Window create/destroy stress", Test_WindowCreateDestroyStress);

    printf("\n");
    printf("Total hotkeys sent: %d\n\n", g_hotkeysSent.load());

    return TestHarness::Summarize();
}
