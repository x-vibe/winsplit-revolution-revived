/**
 * DLL Hijacking Security Test
 * HIGH PRIORITY SECURITY TEST
 *
 * Tests WinSplit's vulnerability to DLL hijacking attacks.
 *
 * Background:
 * WinSplit loads winsplithook.dll at runtime using LoadLibrary without
 * specifying a full path. This can allow attackers to place a malicious
 * DLL in a directory that gets searched before the application directory.
 *
 * Attack vectors:
 * 1. Current working directory (if not application dir)
 * 2. PATH environment variable directories
 * 3. User's home directory
 *
 * This test:
 * 1. Checks how WinSplit loads its DLL
 * 2. Verifies DLL load path security
 * 3. Reports on potential hijacking vectors
 */

#include "../tools/test_harness.h"
#include <stdio.h>
#include <shlwapi.h>
#include <psapi.h>

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "psapi.lib")

// Check if a path is in a secure location
bool IsSecurePath(const wchar_t* path) {
    // Secure locations: Program Files, Windows directory
    wchar_t programFiles[MAX_PATH];
    wchar_t programFilesX86[MAX_PATH];
    wchar_t windowsDir[MAX_PATH];

    ExpandEnvironmentStrings(L"%ProgramFiles%", programFiles, MAX_PATH);
    ExpandEnvironmentStrings(L"%ProgramFiles(x86)%", programFilesX86, MAX_PATH);
    GetWindowsDirectory(windowsDir, MAX_PATH);

    // Check if path starts with a secure directory
    if (wcsncmp(path, programFiles, wcslen(programFiles)) == 0) return true;
    if (wcsncmp(path, programFilesX86, wcslen(programFilesX86)) == 0) return true;
    if (wcsncmp(path, windowsDir, wcslen(windowsDir)) == 0) return true;

    return false;
}

// Get the path where a DLL is loaded from for a process
bool GetLoadedDllPath(DWORD pid, const wchar_t* dllName, wchar_t* outPath, size_t outSize) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (!hProcess) return false;

    HMODULE hMods[1024];
    DWORD cbNeeded;

    if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
        for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
            wchar_t modName[MAX_PATH];
            if (GetModuleFileNameEx(hProcess, hMods[i], modName, MAX_PATH)) {
                wchar_t* fileName = PathFindFileName(modName);
                if (_wcsicmp(fileName, dllName) == 0) {
                    wcscpy_s(outPath, outSize, modName);
                    CloseHandle(hProcess);
                    return true;
                }
            }
        }
    }

    CloseHandle(hProcess);
    return false;
}

// Test: Check if winsplithook.dll is loaded from application directory
bool Test_DllLoadedFromAppDir() {
    // Find WinSplit process
    DWORD pid = TestHarness::GetWinSplitProcessId();
    if (pid == 0) {
        printf("  WinSplit not running\n");
        return false;
    }

    // Get WinSplit executable path
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!hProcess) {
        printf("  Cannot open WinSplit process\n");
        return false;
    }

    wchar_t exePath[MAX_PATH];
    DWORD exePathSize = MAX_PATH;
    QueryFullProcessImageName(hProcess, 0, exePath, &exePathSize);
    CloseHandle(hProcess);

    // Get directory of exe
    wchar_t exeDir[MAX_PATH];
    wcscpy_s(exeDir, exePath);
    PathRemoveFileSpec(exeDir);

    // Find where winsplithook.dll is loaded from
    wchar_t dllPath[MAX_PATH] = {0};
    if (!GetLoadedDllPath(pid, L"winsplithook.dll", dllPath, MAX_PATH)) {
        printf("  winsplithook.dll not found in process\n");
        return false;
    }

    wchar_t dllDir[MAX_PATH];
    wcscpy_s(dllDir, dllPath);
    PathRemoveFileSpec(dllDir);

    printf("  EXE Dir: %ls\n", exeDir);
    printf("  DLL Path: %ls\n", dllPath);

    // Verify DLL is in same directory as EXE
    if (_wcsicmp(exeDir, dllDir) == 0) {
        printf("  DLL loaded from application directory [GOOD]\n");
        return true;
    } else {
        printf("  WARNING: DLL loaded from different directory! [BAD]\n");
        return false;
    }
}

// Test: Check DLL search order safety
bool Test_DllSearchOrder() {
    printf("  Checking for DLL search order vulnerabilities...\n");

    // Check if SetDllDirectory was called (improved security)
    // We can't directly test this, but we can check the behavior

    wchar_t currentDir[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, currentDir);

    wchar_t testDllPath[MAX_PATH];
    swprintf(testDllPath, MAX_PATH, L"%s\\winsplithook.dll", currentDir);

    // Check if a DLL exists in current directory (potential hijack)
    if (PathFileExists(testDllPath)) {
        DWORD pid = TestHarness::GetWinSplitProcessId();
        if (pid) {
            wchar_t loadedPath[MAX_PATH];
            if (GetLoadedDllPath(pid, L"winsplithook.dll", loadedPath, MAX_PATH)) {
                if (_wcsicmp(loadedPath, testDllPath) == 0) {
                    printf("  CRITICAL: DLL loaded from current directory!\n");
                    printf("  This is a DLL hijacking vulnerability.\n");
                    return false;
                }
            }
        }
    }

    printf("  DLL search order appears safe.\n");
    return true;
}

// Test: Check if DLL is in secure location
bool Test_DllSecureLocation() {
    DWORD pid = TestHarness::GetWinSplitProcessId();
    if (pid == 0) return false;

    wchar_t dllPath[MAX_PATH];
    if (!GetLoadedDllPath(pid, L"winsplithook.dll", dllPath, MAX_PATH)) {
        printf("  Cannot find winsplithook.dll\n");
        return false;
    }

    if (IsSecurePath(dllPath)) {
        printf("  DLL is in secure location (Program Files or Windows)\n");
        return true;
    } else {
        printf("  WARNING: DLL not in secure location\n");
        printf("  Path: %ls\n", dllPath);
        printf("  Consider installing to Program Files\n");
        return false;  // Not a test failure, just a warning
    }
}

// Test: Check DLL signature (if signed)
bool Test_DllSignature() {
    DWORD pid = TestHarness::GetWinSplitProcessId();
    if (pid == 0) return false;

    wchar_t dllPath[MAX_PATH];
    if (!GetLoadedDllPath(pid, L"winsplithook.dll", dllPath, MAX_PATH)) {
        return false;
    }

    // Check if file has Authenticode signature
    // (This is a simplified check - production code should use WinVerifyTrust)
    DWORD certSize = 0;
    HANDLE hFile = CreateFile(dllPath, GENERIC_READ, FILE_SHARE_READ, NULL,
                               OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        return false;
    }

    // Check for certificate table in PE
    IMAGE_DOS_HEADER dosHeader;
    DWORD bytesRead;
    ReadFile(hFile, &dosHeader, sizeof(dosHeader), &bytesRead, NULL);

    if (dosHeader.e_magic != IMAGE_DOS_SIGNATURE) {
        CloseHandle(hFile);
        return false;
    }

    SetFilePointer(hFile, dosHeader.e_lfanew, NULL, FILE_BEGIN);
    IMAGE_NT_HEADERS ntHeaders;
    ReadFile(hFile, &ntHeaders, sizeof(ntHeaders), &bytesRead, NULL);

    CloseHandle(hFile);

    if (ntHeaders.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_SECURITY].VirtualAddress != 0) {
        printf("  DLL has digital signature\n");
        return true;
    } else {
        printf("  DLL is not digitally signed\n");
        printf("  Consider signing the DLL for better security\n");
        return true;  // Not a failure, just informational
    }
}

// Test: Report potential hijack locations
bool Test_ReportHijackLocations() {
    printf("  Potential DLL hijack locations to monitor:\n");

    // Get PATH
    wchar_t pathEnv[32768];
    GetEnvironmentVariable(L"PATH", pathEnv, 32768);

    // Check each PATH directory
    wchar_t* context = NULL;
    wchar_t* token = wcstok_s(pathEnv, L";", &context);
    int count = 0;

    while (token && count < 10) {  // Limit output
        wchar_t testPath[MAX_PATH];
        swprintf(testPath, MAX_PATH, L"%s\\winsplithook.dll", token);

        // Check if writable by current user
        HANDLE hTest = CreateFile(testPath, GENERIC_WRITE, 0, NULL,
                                   CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hTest != INVALID_HANDLE_VALUE) {
            CloseHandle(hTest);
            DeleteFile(testPath);  // Clean up test file
            printf("  [WRITABLE] %ls\n", token);
            count++;
        }

        token = wcstok_s(NULL, L";", &context);
    }

    if (count == 0) {
        printf("  No easily writable PATH directories found.\n");
    }

    return true;
}

int main() {
    TestHarness::Init("DLL Hijacking Security Test");

    if (!TestHarness::IsWinSplitRunning()) {
        printf("ERROR: WinSplit is not running.\n");
        printf("Please start WinSplit Revolution before running this test.\n");
        return 1;
    }

    printf("WinSplit found. Checking DLL loading security...\n\n");

    // Run tests
    TestHarness::RunTest("DLL loaded from app directory", Test_DllLoadedFromAppDir);
    TestHarness::RunTest("DLL search order safety", Test_DllSearchOrder);
    TestHarness::RunTest("DLL in secure location", Test_DllSecureLocation);
    TestHarness::RunTest("DLL signature check", Test_DllSignature);
    TestHarness::RunTest("Report hijack locations", Test_ReportHijackLocations);

    printf("\n");
    TestHarness::PrintColored("RECOMMENDATIONS:\n", COLOR_YELLOW);
    printf("1. Load DLL with full path: LoadLibrary(L\"C:\\\\path\\\\winsplithook.dll\")\n");
    printf("2. Call SetDllDirectory(L\"\") before LoadLibrary to remove CWD from search\n");
    printf("3. Digitally sign the DLL\n");
    printf("4. Install to Program Files (requires elevation)\n\n");

    return TestHarness::Summarize();
}
