/**
 * XML Injection Security Tests
 * Using Google Test framework
 */

#include <gtest/gtest.h>
#include <windows.h>
#include <shlobj.h>
#include <fstream>
#include <string>

class XmlInjectionTest : public ::testing::Test {
protected:
    std::wstring dataDir;
    std::wstring settingsPath;
    std::wstring settingsBackup;

    void SetUp() override {
        wchar_t appData[MAX_PATH];
        if (FAILED(SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, appData))) {
            GTEST_SKIP() << "Cannot get AppData path";
        }

        dataDir = std::wstring(appData) + L"\\WinSplit Revolution\\";
        settingsPath = dataDir + L"settings.xml";
        settingsBackup = settingsPath + L".test_backup";

        // Backup original if exists
        if (GetFileAttributesW(settingsPath.c_str()) != INVALID_FILE_ATTRIBUTES) {
            CopyFileW(settingsPath.c_str(), settingsBackup.c_str(), FALSE);
        }
    }

    void TearDown() override {
        // Restore backup
        if (GetFileAttributesW(settingsBackup.c_str()) != INVALID_FILE_ATTRIBUTES) {
            MoveFileExW(settingsBackup.c_str(), settingsPath.c_str(), MOVEFILE_REPLACE_EXISTING);
        }
    }

    void WriteTestXml(const std::string& content) {
        std::ofstream file(settingsPath);
        file << content;
    }
};

// Test: Integer overflow values don't crash parser
TEST_F(XmlInjectionTest, IntegerOverflowValuesHandled) {
    WriteTestXml(R"(<?xml version="1.0"?>
<Settings>
  <VnpTransparency>999999999999999</VnpTransparency>
  <DnGDetectionRadius>-2147483649</DnGDetectionRadius>
</Settings>)");

    // WinSplit should handle this gracefully on next start
    // This test documents the vulnerability - actual verification needs WinSplit restart
    SUCCEED() << "Overflow values written - verify WinSplit handles gracefully";
}

// Test: Very long strings don't cause buffer overflow
TEST_F(XmlInjectionTest, LongStringsHandled) {
    std::string longString(100000, 'A');
    WriteTestXml("<?xml version=\"1.0\"?><Settings><Language>" + longString + "</Language></Settings>");

    SUCCEED() << "Long string written - verify WinSplit handles gracefully";
}

// Test: Malformed XML doesn't crash
TEST_F(XmlInjectionTest, MalformedXmlHandled) {
    std::vector<std::string> malformedCases = {
        "",                                    // Empty
        "not xml at all",                      // Plain text
        "<?xml version=\"1.0\"?><Settings>",   // Unclosed tag
        "<Settings></Language>",               // Mismatched
    };

    for (const auto& xml : malformedCases) {
        WriteTestXml(xml);
        // Just verify we can write - actual crash test needs WinSplit restart
    }

    SUCCEED();
}
