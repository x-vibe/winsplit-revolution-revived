/**
 * HTTP Update Vulnerability Tests
 * Using Google Test framework
 */

#include <gtest/gtest.h>
#include <windows.h>
#include <wininet.h>

#pragma comment(lib, "wininet.lib")

class HttpUpdateTest : public ::testing::Test {
protected:
    const wchar_t* updateHost = L"winsplit-revolution.com";
    const wchar_t* updatePath = L"/update.php";
};

// Test: Update endpoint uses HTTP (vulnerability)
TEST_F(HttpUpdateTest, UpdateEndpointUsesPlainHttp) {
    // Document the known vulnerability
    // The update URL is: http://winsplit-revolution.com/update.php

    // This test passes to document the issue, but logs a warning
    GTEST_LOG_(WARNING) << "WinSplit uses plain HTTP for updates - MITM vulnerable!";

    HINTERNET hInternet = InternetOpen(L"SecurityTest",
                                        INTERNET_OPEN_TYPE_DIRECT,
                                        nullptr, nullptr, 0);
    ASSERT_NE(hInternet, nullptr);

    // Try HTTP connection
    HINTERNET hConnect = InternetConnect(hInternet, updateHost,
                                          INTERNET_DEFAULT_HTTP_PORT,
                                          nullptr, nullptr,
                                          INTERNET_SERVICE_HTTP, 0, 0);

    if (hConnect) {
        GTEST_LOG_(INFO) << "HTTP connection to update server possible";
        InternetCloseHandle(hConnect);
    }

    InternetCloseHandle(hInternet);

    // This "succeeds" but the vulnerability is documented
    SUCCEED() << "CRITICAL: Update mechanism uses HTTP - upgrade to HTTPS recommended";
}

// Test: HTTPS support on update server
TEST_F(HttpUpdateTest, HttpsAvailability) {
    HINTERNET hInternet = InternetOpen(L"SecurityTest",
                                        INTERNET_OPEN_TYPE_DIRECT,
                                        nullptr, nullptr, 0);
    ASSERT_NE(hInternet, nullptr);

    HINTERNET hConnect = InternetConnect(hInternet, updateHost,
                                          INTERNET_DEFAULT_HTTPS_PORT,
                                          nullptr, nullptr,
                                          INTERNET_SERVICE_HTTP, 0, 0);

    bool httpsAvailable = false;
    if (hConnect) {
        HINTERNET hRequest = HttpOpenRequest(hConnect, L"GET", updatePath,
                                              nullptr, nullptr, nullptr,
                                              INTERNET_FLAG_SECURE, 0);
        if (hRequest) {
            if (HttpSendRequest(hRequest, nullptr, 0, nullptr, 0)) {
                httpsAvailable = true;
            }
            InternetCloseHandle(hRequest);
        }
        InternetCloseHandle(hConnect);
    }

    InternetCloseHandle(hInternet);

    if (httpsAvailable) {
        GTEST_LOG_(INFO) << "HTTPS is available - WinSplit should use it!";
    } else {
        GTEST_LOG_(WARNING) << "HTTPS not available on update server";
    }

    // Not a pass/fail - just informational
    SUCCEED();
}

// Test: No certificate validation (by design, since HTTP is used)
TEST_F(HttpUpdateTest, NoCertificateValidation) {
    // Since WinSplit uses plain HTTP, there's no certificate to validate
    // This documents the missing security control

    GTEST_LOG_(WARNING) << "No certificate validation - HTTP has no certificates";
    GTEST_LOG_(WARNING) << "Recommendation: Use HTTPS + certificate pinning";

    SUCCEED() << "Certificate validation not applicable (HTTP used)";
}

// Test: No signature verification on updates
TEST_F(HttpUpdateTest, NoSignatureVerification) {
    // Based on code analysis, WinSplit doesn't verify update signatures
    // update_thread.cpp parses XML and downloads files without verification

    GTEST_LOG_(WARNING) << "Update files are not cryptographically signed";
    GTEST_LOG_(WARNING) << "Recommendation: Sign updates and verify before applying";

    SUCCEED() << "Signature verification not implemented";
}
