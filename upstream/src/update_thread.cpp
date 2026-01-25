/**
 * @file update_thread.cpp
 * @brief Secure update checker using GitHub Releases API (HTTPS)
 *
 * SECURITY: This replaces the old insecure HTTP update mechanism.
 * Now uses GitHub API over HTTPS for secure version checking.
 *
 * GitHub API endpoint:
 * https://api.github.com/repos/x-vibe/winsplit-revolution-revived/releases/latest
 */

#include <time.h>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <wininet.h>

#pragma comment(lib, "wininet.lib")

#include <wx/sstream.h>
#include <wx/txtstrm.h>

#include "main.h"
#include "update_thread.h"

// GitHub repository for update checks
static const wchar_t* GITHUB_API_HOST = L"api.github.com";
static const wchar_t* GITHUB_API_PATH = L"/repos/x-vibe/winsplit-revolution-revived/releases/latest";
static const wchar_t* GITHUB_RELEASES_URL = L"https://github.com/x-vibe/winsplit-revolution-revived/releases";

ReadVersionThread::ReadVersionThread(const unsigned int& timeout)
    : wxThread(wxTHREAD_JOINABLE)
    , m_options(SettingsManager::Get())
    , m_flagForceChecking(false)
    , m_host_version(0.)
    , m_strVersion()
    , m_strReleaseUrl(GITHUB_RELEASES_URL)
    , m_strReleaseNotes()
    , m_timeout(timeout)
{
}

ReadVersionThread::~ReadVersionThread() {}

/**
 * @brief Simple JSON value extractor (no external dependency)
 *
 * Extracts a string value for a given key from JSON.
 * This is intentionally simple - only handles the specific GitHub API response format.
 */
static wxString ExtractJsonValue(const wxString& json, const wxString& key)
{
  wxString searchKey = _T("\"") + key + _T("\"");
  int keyPos = json.Find(searchKey);
  if (keyPos == wxNOT_FOUND) return wxEmptyString;

  // Find the colon after the key
  int colonPos = json.find(_T(':'), keyPos);
  if (colonPos == wxNOT_FOUND) return wxEmptyString;

  // Skip whitespace
  size_t valueStart = colonPos + 1;
  while (valueStart < json.length() && (json[valueStart] == ' ' || json[valueStart] == '\t'))
    valueStart++;

  if (valueStart >= json.length()) return wxEmptyString;

  // Check if value is a string (starts with quote)
  if (json[valueStart] == '"') {
    valueStart++;
    size_t valueEnd = valueStart;

    // Find closing quote, handling escaped quotes
    while (valueEnd < json.length()) {
      if (json[valueEnd] == '"' && (valueEnd == 0 || json[valueEnd - 1] != '\\')) {
        break;
      }
      valueEnd++;
    }

    wxString result = json.Mid(valueStart, valueEnd - valueStart);

    // Unescape common sequences
    result.Replace(_T("\\n"), _T("\n"));
    result.Replace(_T("\\r"), _T(""));
    result.Replace(_T("\\\""), _T("\""));
    result.Replace(_T("\\\\"), _T("\\"));

    return result;
  }

  return wxEmptyString;
}

/**
 * @brief Parse version string to double for comparison
 *
 * Handles formats like "v10.2.0", "10.2.0", "10.2"
 */
static double ParseVersion(const wxString& versionStr)
{
  wxString cleanVersion = versionStr;

  // Remove 'v' prefix if present
  if (cleanVersion.StartsWith(_T("v")) || cleanVersion.StartsWith(_T("V"))) {
    cleanVersion = cleanVersion.Mid(1);
  }

  // Parse major.minor.patch
  double version = 0.0;
  long major = 0, minor = 0, patch = 0;

  wxStringTokenizer tokenizer(cleanVersion, _T("."));
  if (tokenizer.HasMoreTokens()) {
    tokenizer.GetNextToken().ToLong(&major);
  }
  if (tokenizer.HasMoreTokens()) {
    tokenizer.GetNextToken().ToLong(&minor);
  }
  if (tokenizer.HasMoreTokens()) {
    tokenizer.GetNextToken().ToLong(&patch);
  }

  // Combine: 10.2.1 -> 10.0201
  version = major + (minor / 100.0) + (patch / 10000.0);

  return version;
}

void* ReadVersionThread::Entry()
{
  if (!HaveToCheck()) {
    return nullptr;
  }

  // SECURITY: Use WinInet for HTTPS support
  HINTERNET hInternet = InternetOpenW(
      L"WinSplit-Revolution-UpdateChecker/1.0",
      INTERNET_OPEN_TYPE_PRECONFIG,
      NULL, NULL, 0);

  if (!hInternet) {
    return nullptr;
  }

  // Connect to GitHub API over HTTPS
  HINTERNET hConnect = InternetConnectW(
      hInternet,
      GITHUB_API_HOST,
      INTERNET_DEFAULT_HTTPS_PORT,
      NULL, NULL,
      INTERNET_SERVICE_HTTP,
      0, 0);

  if (!hConnect) {
    InternetCloseHandle(hInternet);
    return nullptr;
  }

  // Create HTTPS request
  DWORD flags = INTERNET_FLAG_SECURE |
                INTERNET_FLAG_NO_CACHE_WRITE |
                INTERNET_FLAG_RELOAD;

  HINTERNET hRequest = HttpOpenRequestW(
      hConnect,
      L"GET",
      GITHUB_API_PATH,
      NULL, NULL, NULL,
      flags, 0);

  if (!hRequest) {
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);
    return nullptr;
  }

  // GitHub API requires User-Agent header
  HttpAddRequestHeadersW(hRequest,
      L"User-Agent: WinSplit-Revolution-UpdateChecker/1.0\r\n"
      L"Accept: application/vnd.github.v3+json\r\n",
      -1L, HTTP_ADDREQ_FLAG_ADD);

  // Set timeout
  DWORD timeout = m_timeout * 1000;  // Convert to milliseconds
  InternetSetOptionW(hRequest, INTERNET_OPTION_RECEIVE_TIMEOUT, &timeout, sizeof(timeout));
  InternetSetOptionW(hRequest, INTERNET_OPTION_CONNECT_TIMEOUT, &timeout, sizeof(timeout));

  // Send request
  if (!HttpSendRequestW(hRequest, NULL, 0, NULL, 0)) {
    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);
    return nullptr;
  }

  // Read response
  std::string responseData;
  char buffer[4096];
  DWORD bytesRead = 0;

  while (InternetReadFile(hRequest, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead > 0) {
    buffer[bytesRead] = '\0';
    responseData += buffer;
  }

  InternetCloseHandle(hRequest);
  InternetCloseHandle(hConnect);
  InternetCloseHandle(hInternet);

  // Parse JSON response
  if (!responseData.empty()) {
    wxString jsonResponse = wxString::FromUTF8(responseData.c_str());

    // Extract version from tag_name (e.g., "v10.2.0")
    wxString tagName = ExtractJsonValue(jsonResponse, _T("tag_name"));
    if (!tagName.IsEmpty()) {
      m_strVersion = tagName;
      m_host_version = ParseVersion(tagName);
    }

    // Extract release URL
    wxString htmlUrl = ExtractJsonValue(jsonResponse, _T("html_url"));
    if (!htmlUrl.IsEmpty()) {
      m_strReleaseUrl = htmlUrl;
    }

    // Extract release notes (body)
    wxString body = ExtractJsonValue(jsonResponse, _T("body"));
    if (!body.IsEmpty()) {
      m_strReleaseNotes = body;
    }
  }

  return nullptr;
}

void ReadVersionThread::ForceChecking()
{
  m_flagForceChecking = true;
}

bool ReadVersionThread::HaveToCheck()
{
  time_t now, last, tmp;
  struct tm timeinfo_last;
  struct tm timeinfo_now;

  if (m_flagForceChecking)
    return true;

  if (!m_options.hasToCheckForUpdates())
    return false;
  if (m_options.getUpdateCheckFrequency() == CHECK_UPDATES_ON_START)
    return true;

  time(&now);
  tmp = now;
  last = m_options.getLastCheckDate();

  timeinfo_now = *(localtime(&now));
  timeinfo_last = *(localtime(&last));

  if ((timeinfo_now.tm_mon != timeinfo_last.tm_mon) &&
      (m_options.getUpdateCheckFrequency() == CHECK_UPDATES_MONTHLY)) {
    m_options.setLastCheckDate(tmp);
    return true;
  }

  if (m_options.getUpdateCheckFrequency() == CHECK_UPDATES_WEEKLY) {
    if ((timeinfo_now.tm_mday == timeinfo_last.tm_mday) &&
        ((timeinfo_now.tm_mon != timeinfo_last.tm_mon))) {
      m_options.setLastCheckDate(tmp);
      return true;
    }

    if ((timeinfo_now.tm_mday - timeinfo_last.tm_mday) >= 7) {
      m_options.setLastCheckDate(tmp);
      return true;
    }

    if (timeinfo_now.tm_mday < timeinfo_last.tm_mday) {
      if ((30 + timeinfo_now.tm_mday - timeinfo_last.tm_mday) >= 7) {
        m_options.setLastCheckDate(tmp);
        return true;
      }
    }
  }
  return false;
}
