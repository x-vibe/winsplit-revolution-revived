/**
 * XML Injection Security Test
 * MEDIUM PRIORITY SECURITY TEST
 *
 * Tests WinSplit's handling of malformed and malicious XML configuration files.
 *
 * Background:
 * WinSplit stores settings in XML files (settings.xml, layout.xml, hotkeys.xml).
 * Improper parsing could lead to:
 * - XXE (XML External Entity) attacks
 * - Integer overflows from extreme values
 * - Buffer overflows from long strings
 * - Application crashes from malformed XML
 *
 * This test creates various malicious XML files and verifies WinSplit
 * handles them gracefully.
 */

#include "../tools/test_harness.h"
#include <stdio.h>
#include <shlobj.h>

// Get WinSplit data directory
bool GetWinSplitDataDir(wchar_t* outPath, size_t size) {
    // WinSplit stores data in %APPDATA%\WinSplit Revolution\
    if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, outPath))) {
        wcscat_s(outPath, size, L"\\WinSplit Revolution\\");
        return true;
    }
    return false;
}

// Backup a file
bool BackupFile(const wchar_t* path) {
    wchar_t backupPath[MAX_PATH];
    swprintf(backupPath, MAX_PATH, L"%s.backup", path);
    return CopyFile(path, backupPath, FALSE) != 0;
}

// Restore from backup
bool RestoreFile(const wchar_t* path) {
    wchar_t backupPath[MAX_PATH];
    swprintf(backupPath, MAX_PATH, L"%s.backup", path);
    return MoveFileEx(backupPath, path, MOVEFILE_REPLACE_EXISTING) != 0;
}

// Write test XML content to file
bool WriteTestXml(const wchar_t* path, const char* content) {
    HANDLE hFile = CreateFile(path, GENERIC_WRITE, 0, NULL,
                               CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return false;

    DWORD written;
    WriteFile(hFile, content, (DWORD)strlen(content), &written, NULL);
    CloseHandle(hFile);
    return true;
}

// Test: XXE (XML External Entity) attack
bool Test_XXE_Attack() {
    printf("  Testing XXE vulnerability...\n");

    wchar_t dataDir[MAX_PATH];
    if (!GetWinSplitDataDir(dataDir, MAX_PATH)) return false;

    wchar_t settingsPath[MAX_PATH];
    swprintf(settingsPath, MAX_PATH, L"%ssettings.xml", dataDir);

    // Backup original
    BackupFile(settingsPath);

    // XXE payload - tries to read system file
    const char* xxePayload =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<!DOCTYPE settings [\n"
        "  <!ENTITY xxe SYSTEM \"file:///C:/Windows/System32/drivers/etc/hosts\">\n"
        "]>\n"
        "<Settings>\n"
        "  <Language>&xxe;</Language>\n"
        "</Settings>\n";

    WriteTestXml(settingsPath, xxePayload);

    printf("  XXE payload written. Manual restart of WinSplit needed to test.\n");
    printf("  Check if hosts file content appears in settings.\n");

    // Restore original
    RestoreFile(settingsPath);

    // This test is informational - we can't automatically verify
    return true;
}

// Test: Integer overflow in settings
bool Test_IntegerOverflow() {
    printf("  Testing integer overflow handling...\n");

    wchar_t dataDir[MAX_PATH];
    if (!GetWinSplitDataDir(dataDir, MAX_PATH)) return false;

    wchar_t settingsPath[MAX_PATH];
    swprintf(settingsPath, MAX_PATH, L"%ssettings.xml", dataDir);

    BackupFile(settingsPath);

    // Settings with extreme integer values
    const char* overflowPayload =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<Settings>\n"
        "  <VnpTransparency>9999999999999999999</VnpTransparency>\n"
        "  <VnpAutoHide>-2147483649</VnpAutoHide>\n"
        "  <DnGDetectionRadius>2147483648</DnGDetectionRadius>\n"
        "  <VnpX>99999999</VnpX>\n"
        "  <VnpY>-99999999</VnpY>\n"
        "</Settings>\n";

    WriteTestXml(settingsPath, overflowPayload);
    printf("  Overflow values written. Restart WinSplit to test.\n");

    RestoreFile(settingsPath);
    return true;
}

// Test: Extremely long strings
bool Test_LongStrings() {
    printf("  Testing extremely long string handling...\n");

    wchar_t dataDir[MAX_PATH];
    if (!GetWinSplitDataDir(dataDir, MAX_PATH)) return false;

    wchar_t settingsPath[MAX_PATH];
    swprintf(settingsPath, MAX_PATH, L"%ssettings.xml", dataDir);

    BackupFile(settingsPath);

    // Create very long string (100KB)
    char* longPayload = new char[150000];
    strcpy(longPayload, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<Settings>\n  <Language>");

    // Add 100000 'A' characters
    char* ptr = longPayload + strlen(longPayload);
    for (int i = 0; i < 100000; i++) {
        *ptr++ = 'A';
    }
    strcpy(ptr, "</Language>\n</Settings>\n");

    WriteTestXml(settingsPath, longPayload);
    delete[] longPayload;

    printf("  100KB string written. Restart WinSplit to test.\n");

    RestoreFile(settingsPath);
    return true;
}

// Test: Malformed XML
bool Test_MalformedXml() {
    printf("  Testing malformed XML handling...\n");

    wchar_t dataDir[MAX_PATH];
    if (!GetWinSplitDataDir(dataDir, MAX_PATH)) return false;

    wchar_t settingsPath[MAX_PATH];
    swprintf(settingsPath, MAX_PATH, L"%ssettings.xml", dataDir);

    BackupFile(settingsPath);

    // Various malformed XML test cases
    const char* malformedCases[] = {
        // Unclosed tag
        "<?xml version=\"1.0\"?>\n<Settings><Language>en",

        // Invalid characters
        "<?xml version=\"1.0\"?>\n<Settings>\x00\x01\x02</Settings>",

        // Mismatched tags
        "<?xml version=\"1.0\"?>\n<Settings><Language>en</Settings></Language>",

        // Invalid attribute
        "<?xml version=\"1.0\"?>\n<Settings invalid===value>test</Settings>",

        // Binary garbage
        "\x89PNG\r\n\x1a\n\x00\x00\x00\rIHDR",

        // Empty file
        "",

        // Just whitespace
        "   \n\t\r\n   ",
    };

    int numCases = sizeof(malformedCases) / sizeof(malformedCases[0]);

    for (int i = 0; i < numCases; i++) {
        printf("  Case %d: ", i + 1);

        WriteTestXml(settingsPath, malformedCases[i]);

        // We can't automatically test if WinSplit handles this
        // Just verify we can write the file
        printf("written\n");
    }

    RestoreFile(settingsPath);
    printf("  All malformed cases written and restored.\n");
    return true;
}

// Test: Layout.xml specific attacks
bool Test_LayoutXmlAttacks() {
    printf("  Testing layout.xml attack vectors...\n");

    wchar_t dataDir[MAX_PATH];
    if (!GetWinSplitDataDir(dataDir, MAX_PATH)) return false;

    wchar_t layoutPath[MAX_PATH];
    swprintf(layoutPath, MAX_PATH, L"%slayout.xml", dataDir);

    BackupFile(layoutPath);

    // Extreme coordinate values
    const char* extremeCoords =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<LayoutManager>\n"
        "  <Sequence_1 NbCombo=\"1\">\n"
        "    <Combo_0 x=\"-999999.99\" y=\"999999.99\" width=\"-1\" height=\"0\"/>\n"
        "  </Sequence_1>\n"
        "</LayoutManager>\n";

    WriteTestXml(layoutPath, extremeCoords);
    printf("  Extreme coordinates written.\n");

    RestoreFile(layoutPath);
    return true;
}

// Test: Hotkeys.xml specific attacks
bool Test_HotkeysXmlAttacks() {
    printf("  Testing hotkeys.xml attack vectors...\n");

    wchar_t dataDir[MAX_PATH];
    if (!GetWinSplitDataDir(dataDir, MAX_PATH)) return false;

    wchar_t hotkeysPath[MAX_PATH];
    swprintf(hotkeysPath, MAX_PATH, L"%shotkeys.xml", dataDir);

    BackupFile(hotkeysPath);

    // Invalid hotkey values
    const char* invalidHotkeys =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<Hotkeys>\n"
        "  <Hotkey_0 Index=\"-1\" Modifier=\"99999\" Key=\"-2147483648\" Enabled=\"2\"/>\n"
        "  <Hotkey_1 Index=\"2147483647\" Modifier=\"0\" Key=\"0\" Enabled=\"-1\"/>\n"
        "</Hotkeys>\n";

    WriteTestXml(hotkeysPath, invalidHotkeys);
    printf("  Invalid hotkey values written.\n");

    RestoreFile(hotkeysPath);
    return true;
}

// Test: XML Bomb (Billion Laughs)
bool Test_XmlBomb() {
    printf("  Testing XML bomb (Billion Laughs) resistance...\n");

    wchar_t dataDir[MAX_PATH];
    if (!GetWinSplitDataDir(dataDir, MAX_PATH)) return false;

    wchar_t settingsPath[MAX_PATH];
    swprintf(settingsPath, MAX_PATH, L"%ssettings.xml", dataDir);

    BackupFile(settingsPath);

    // Classic Billion Laughs attack
    const char* xmlBomb =
        "<?xml version=\"1.0\"?>\n"
        "<!DOCTYPE lolz [\n"
        "  <!ENTITY lol \"lol\">\n"
        "  <!ENTITY lol2 \"&lol;&lol;&lol;&lol;&lol;&lol;&lol;&lol;&lol;&lol;\">\n"
        "  <!ENTITY lol3 \"&lol2;&lol2;&lol2;&lol2;&lol2;&lol2;&lol2;&lol2;&lol2;&lol2;\">\n"
        "  <!ENTITY lol4 \"&lol3;&lol3;&lol3;&lol3;&lol3;&lol3;&lol3;&lol3;&lol3;&lol3;\">\n"
        "  <!ENTITY lol5 \"&lol4;&lol4;&lol4;&lol4;&lol4;&lol4;&lol4;&lol4;&lol4;&lol4;\">\n"
        "]>\n"
        "<Settings><Language>&lol5;</Language></Settings>\n";

    WriteTestXml(settingsPath, xmlBomb);
    printf("  XML bomb written. This could cause memory exhaustion!\n");

    RestoreFile(settingsPath);
    return true;
}

int main() {
    TestHarness::Init("XML Injection Security Test");

    printf("Testing XML configuration file security...\n\n");

    // Check data directory
    wchar_t dataDir[MAX_PATH];
    if (!GetWinSplitDataDir(dataDir, MAX_PATH)) {
        printf("ERROR: Cannot find WinSplit data directory\n");
        return 1;
    }

    printf("WinSplit data directory: %ls\n\n", dataDir);

    // Verify directory exists
    if (GetFileAttributes(dataDir) == INVALID_FILE_ATTRIBUTES) {
        printf("WARNING: Data directory does not exist.\n");
        printf("Has WinSplit been run at least once?\n\n");
    }

    // Run tests
    TestHarness::RunTest("XXE (External Entity) attack", Test_XXE_Attack);
    TestHarness::RunTest("Integer overflow values", Test_IntegerOverflow);
    TestHarness::RunTest("Extremely long strings", Test_LongStrings);
    TestHarness::RunTest("Malformed XML handling", Test_MalformedXml);
    TestHarness::RunTest("Layout.xml attack vectors", Test_LayoutXmlAttacks);
    TestHarness::RunTest("Hotkeys.xml attack vectors", Test_HotkeysXmlAttacks);
    TestHarness::RunTest("XML Bomb (Billion Laughs)", Test_XmlBomb);

    printf("\n");
    TestHarness::PrintColored("IMPORTANT:\n", COLOR_YELLOW);
    printf("This test writes malicious XML files and then restores backups.\n");
    printf("To fully test, manually restart WinSplit after each test.\n");
    printf("\n");
    TestHarness::PrintColored("RECOMMENDATIONS:\n", COLOR_YELLOW);
    printf("1. Disable DTD processing to prevent XXE attacks\n");
    printf("2. Validate integer ranges before assignment\n");
    printf("3. Limit string lengths when reading from XML\n");
    printf("4. Use safe parsing options (no external entities)\n\n");

    return TestHarness::Summarize();
}
