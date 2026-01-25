/**
 * DLL Security Tests
 * Using Google Test framework
 */

#include <gtest/gtest.h>
#include <windows.h>
#include <psapi.h>
#include <shlwapi.h>
#include <tlhelp32.h>

#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "shlwapi.lib")

class DllSecurityTest : public ::testing::Test {
protected:
    DWORD winsplitPid = 0;
    wchar_t exePath[MAX_PATH] = {};
    wchar_t exeDir[MAX_PATH] = {};

    void SetUp() override {
        // Find WinSplit process
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE) {
            GTEST_SKIP() << "Cannot enumerate processes";
            return;
        }

        PROCESSENTRY32W pe;
        pe.dwSize = sizeof(pe);

        if (Process32FirstW(snapshot, &pe)) {
            do {
                if (_wcsicmp(pe.szExeFile, L"WinSplit.exe") == 0) {
                    winsplitPid = pe.th32ProcessID;
                    break;
                }
            } while (Process32NextW(snapshot, &pe));
        }
        CloseHandle(snapshot);

        if (winsplitPid == 0) {
            GTEST_SKIP() << "WinSplit not running";
            return;
        }

        // Get exe path
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, winsplitPid);
        if (hProcess) {
            DWORD size = MAX_PATH;
            QueryFullProcessImageNameW(hProcess, 0, exePath, &size);
            CloseHandle(hProcess);

            wcscpy_s(exeDir, exePath);
            PathRemoveFileSpecW(exeDir);
        }
    }

    bool GetDllPath(const wchar_t* dllName, wchar_t* outPath, size_t outSize) {
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, winsplitPid);
        if (!hProcess) return false;

        HMODULE hMods[1024];
        DWORD cbNeeded;

        if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
            for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
                wchar_t modName[MAX_PATH];
                if (GetModuleFileNameExW(hProcess, hMods[i], modName, MAX_PATH)) {
                    if (_wcsicmp(PathFindFileNameW(modName), dllName) == 0) {
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
};

// Test: Hook DLL loads from application directory
TEST_F(DllSecurityTest, HookDllLoadsFromAppDirectory) {
    ASSERT_NE(winsplitPid, 0u);

    wchar_t dllPath[MAX_PATH];
    ASSERT_TRUE(GetDllPath(L"winsplithook.dll", dllPath, MAX_PATH))
        << "winsplithook.dll not found in process";

    wchar_t dllDir[MAX_PATH];
    wcscpy_s(dllDir, dllPath);
    PathRemoveFileSpecW(dllDir);

    EXPECT_STREQ(dllDir, exeDir)
        << "DLL loaded from different directory than EXE - potential hijack!";
}

// Test: No DLL in current working directory (hijack prevention)
TEST_F(DllSecurityTest, NoDllInCurrentDirectory) {
    wchar_t currentDir[MAX_PATH];
    GetCurrentDirectoryW(MAX_PATH, currentDir);

    wchar_t testDllPath[MAX_PATH];
    swprintf_s(testDllPath, L"%s\\winsplithook.dll", currentDir);

    // If there's a DLL in CWD, it shouldn't be the one loaded
    if (PathFileExistsW(testDllPath)) {
        wchar_t loadedDllPath[MAX_PATH];
        if (GetDllPath(L"winsplithook.dll", loadedDllPath, MAX_PATH)) {
            EXPECT_STRNE(loadedDllPath, testDllPath)
                << "DLL loaded from current directory - hijacking vulnerability!";
        }
    }
}

// Test: Verify DLL has expected characteristics
TEST_F(DllSecurityTest, DllCharacteristics) {
    wchar_t dllPath[MAX_PATH];
    if (!GetDllPath(L"winsplithook.dll", dllPath, MAX_PATH)) {
        GTEST_SKIP() << "Cannot find winsplithook.dll";
    }

    // Check file exists
    EXPECT_TRUE(PathFileExistsW(dllPath)) << "DLL file not found at reported path";

    // Check file size is reasonable (not suspiciously small or large)
    HANDLE hFile = CreateFileW(dllPath, GENERIC_READ, FILE_SHARE_READ,
                                nullptr, OPEN_EXISTING, 0, nullptr);
    if (hFile != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER fileSize;
        GetFileSizeEx(hFile, &fileSize);
        CloseHandle(hFile);

        // Should be between 10KB and 10MB
        EXPECT_GT(fileSize.QuadPart, 10 * 1024) << "DLL suspiciously small";
        EXPECT_LT(fileSize.QuadPart, 10 * 1024 * 1024) << "DLL suspiciously large";
    }
}
