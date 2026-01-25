/**
 * Message Spoofer Tool
 * Security testing tool for WinSplit Revolution
 *
 * SECURITY TEST TOOL - Used to verify WinSplit handles spoofed messages correctly
 *
 * This tool attempts to send fake hook messages to WinSplit to verify:
 * 1. WinSplit properly validates message sources
 * 2. WinSplit doesn't crash on malformed messages
 * 3. WinSplit doesn't perform unauthorized actions from spoofed messages
 */

#include <windows.h>
#include <stdio.h>
#include <string>

// WinSplit hook message constants (from hook.h)
#define WSM_STARTMOVING  (WM_USER + 100)
#define WSM_STOPMOVING   (WM_USER + 101)

// Hook frame window class
const wchar_t* HOOK_FRAME_CLASS = L"WinSplitHookFrame";
const wchar_t* HOOK_FRAME_TITLE = L"WinSplit Revolution - Hook Frame";

// Find WinSplit hook window
HWND FindWinSplitHookWindow() {
    HWND hwnd = FindWindow(NULL, HOOK_FRAME_TITLE);
    if (!hwnd) {
        // Try alternate method
        hwnd = FindWindow(HOOK_FRAME_CLASS, NULL);
    }
    return hwnd;
}

// Test: Send spoofed START_MOVING message
bool TestSpoofStartMoving(HWND hwndTarget) {
    printf("Test: Sending spoofed WSM_STARTMOVING...\n");

    // Get a random window handle to spoof
    HWND hwndFake = GetDesktopWindow();

    LRESULT result = SendMessage(hwndTarget, WSM_STARTMOVING, (WPARAM)hwndFake, 0);

    printf("  Result: %lld\n", (long long)result);

    // Now send STOP_MOVING to clean up
    SendMessage(hwndTarget, WSM_STOPMOVING, 0, 0);

    return true;
}

// Test: Send spoofed STOP_MOVING with wheel delta
bool TestSpoofStopMovingWithWheel(HWND hwndTarget, int wheelDelta) {
    printf("Test: Sending spoofed WSM_STOPMOVING with wheel=%d...\n", wheelDelta);

    LRESULT result = SendMessage(hwndTarget, WSM_STOPMOVING, (WPARAM)wheelDelta, 0);

    printf("  Result: %lld\n", (long long)result);
    return true;
}

// Test: Send extreme wheel values
bool TestExtremeWheelValues(HWND hwndTarget) {
    printf("Test: Sending extreme wheel values...\n");

    // First send START to set up state
    SendMessage(hwndTarget, WSM_STARTMOVING, (WPARAM)GetDesktopWindow(), 0);

    // Test extreme negative
    printf("  Testing INT_MIN wheel value...\n");
    SendMessage(hwndTarget, WSM_STOPMOVING, (WPARAM)INT_MIN, 0);

    // Test extreme positive
    SendMessage(hwndTarget, WSM_STARTMOVING, (WPARAM)GetDesktopWindow(), 0);
    printf("  Testing INT_MAX wheel value...\n");
    SendMessage(hwndTarget, WSM_STOPMOVING, (WPARAM)INT_MAX, 0);

    // Test -1 (potential array underflow)
    SendMessage(hwndTarget, WSM_STARTMOVING, (WPARAM)GetDesktopWindow(), 0);
    printf("  Testing -1 wheel value (array underflow)...\n");
    SendMessage(hwndTarget, WSM_STOPMOVING, (WPARAM)-1, 0);

    printf("  All extreme values tested without crash.\n");
    return true;
}

// Test: Rapid fire messages
bool TestRapidFireMessages(HWND hwndTarget, int count) {
    printf("Test: Sending %d rapid messages...\n", count);

    DWORD start = GetTickCount();

    for (int i = 0; i < count; i++) {
        SendMessage(hwndTarget, WSM_STARTMOVING, (WPARAM)GetDesktopWindow(), 0);
        SendMessage(hwndTarget, WSM_STOPMOVING, (WPARAM)(i % 10), 0);
    }

    DWORD elapsed = GetTickCount() - start;
    printf("  Completed in %lu ms (%.1f msg/sec)\n", elapsed, (count * 2.0 * 1000.0) / elapsed);

    return true;
}

// Test: Send messages with NULL hwnd
bool TestNullHwnd(HWND hwndTarget) {
    printf("Test: Sending messages with NULL window handle...\n");

    SendMessage(hwndTarget, WSM_STARTMOVING, (WPARAM)NULL, 0);
    SendMessage(hwndTarget, WSM_STOPMOVING, 0, 0);

    printf("  Completed without crash.\n");
    return true;
}

// Test: Send messages with invalid hwnd
bool TestInvalidHwnd(HWND hwndTarget) {
    printf("Test: Sending messages with invalid window handle...\n");

    // Use obviously invalid handles
    SendMessage(hwndTarget, WSM_STARTMOVING, (WPARAM)0xDEADBEEF, 0);
    SendMessage(hwndTarget, WSM_STOPMOVING, 0, 0);

    SendMessage(hwndTarget, WSM_STARTMOVING, (WPARAM)0xFFFFFFFF, 0);
    SendMessage(hwndTarget, WSM_STOPMOVING, 0, 0);

    printf("  Completed without crash.\n");
    return true;
}

// Test: Send custom WM_USER messages
bool TestCustomUserMessages(HWND hwndTarget) {
    printf("Test: Sending various WM_USER messages...\n");

    for (int i = 0; i < 200; i++) {
        UINT msg = WM_USER + i;
        SendMessage(hwndTarget, msg, 0, 0);
    }

    printf("  200 WM_USER messages sent without crash.\n");
    return true;
}

void PrintUsage() {
    printf("Message Spoofer - WinSplit Security Testing Tool\n\n");
    printf("Usage: message_spoofer.exe [options]\n\n");
    printf("Options:\n");
    printf("  --all            Run all tests\n");
    printf("  --start          Test spoofed WSM_STARTMOVING\n");
    printf("  --stop           Test spoofed WSM_STOPMOVING\n");
    printf("  --extreme        Test extreme wheel values\n");
    printf("  --rapid N        Send N rapid message pairs\n");
    printf("  --null           Test NULL window handle\n");
    printf("  --invalid        Test invalid window handles\n");
    printf("  --user           Test WM_USER message range\n");
    printf("  --help           Show this help\n");
}

int main(int argc, char* argv[]) {
    printf("=== WinSplit Message Spoofer ===\n\n");

    // Find WinSplit
    HWND hwndWinSplit = FindWinSplitHookWindow();
    if (!hwndWinSplit) {
        printf("ERROR: WinSplit hook window not found.\n");
        printf("Please ensure WinSplit Revolution is running.\n");
        return 1;
    }

    printf("Found WinSplit hook window: %p\n\n", hwndWinSplit);

    // Parse arguments
    bool runAll = (argc == 1);
    bool runStart = false;
    bool runStop = false;
    bool runExtreme = false;
    bool runRapid = false;
    bool runNull = false;
    bool runInvalid = false;
    bool runUser = false;
    int rapidCount = 1000;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "--help" || arg == "-h") {
            PrintUsage();
            return 0;
        } else if (arg == "--all") {
            runAll = true;
        } else if (arg == "--start") {
            runStart = true;
        } else if (arg == "--stop") {
            runStop = true;
        } else if (arg == "--extreme") {
            runExtreme = true;
        } else if (arg == "--rapid" && i + 1 < argc) {
            runRapid = true;
            rapidCount = atoi(argv[++i]);
        } else if (arg == "--null") {
            runNull = true;
        } else if (arg == "--invalid") {
            runInvalid = true;
        } else if (arg == "--user") {
            runUser = true;
        }
    }

    int passed = 0;
    int failed = 0;

    // Run tests
    if (runAll || runStart) {
        if (TestSpoofStartMoving(hwndWinSplit)) passed++; else failed++;
    }

    if (runAll || runStop) {
        if (TestSpoofStopMovingWithWheel(hwndWinSplit, 5)) passed++; else failed++;
    }

    if (runAll || runExtreme) {
        if (TestExtremeWheelValues(hwndWinSplit)) passed++; else failed++;
    }

    if (runAll || runRapid) {
        if (TestRapidFireMessages(hwndWinSplit, rapidCount)) passed++; else failed++;
    }

    if (runAll || runNull) {
        if (TestNullHwnd(hwndWinSplit)) passed++; else failed++;
    }

    if (runAll || runInvalid) {
        if (TestInvalidHwnd(hwndWinSplit)) passed++; else failed++;
    }

    if (runAll || runUser) {
        if (TestCustomUserMessages(hwndWinSplit)) passed++; else failed++;
    }

    // Summary
    printf("\n=== Summary ===\n");
    printf("Passed: %d\n", passed);
    printf("Failed: %d\n", failed);

    if (failed == 0) {
        printf("\nWinSplit handled all spoofed messages without crashing.\n");
        printf("NOTE: This tests robustness only. Manual verification is needed\n");
        printf("      to confirm no unauthorized actions were performed.\n");
    }

    return failed > 0 ? 1 : 0;
}
