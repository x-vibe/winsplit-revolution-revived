#ifndef DEBUG_LOG_H
#define DEBUG_LOG_H

#include <fstream>
#include <string>
#include <ctime>
#include <mutex>

// Simple file-based debug logger for WinSplit Revolution
// Logs are written to winsplit_debug.log in the application directory
//
// Usage:
//   DEBUG_LOG("Starting initialization");
//   DEBUG_LOG_FMT("Loaded %d settings", count);
//
// Enable/disable via:
//   DebugLog::Enable(true);   // Enable logging
//   DebugLog::Enable(false);  // Disable logging (default)

class DebugLog {
public:
  static DebugLog& Instance() {
    static DebugLog instance;
    return instance;
  }

  static void Enable(bool enable) {
    Instance().m_enabled = enable;
    if (enable) {
      Instance().Log("=== Debug logging enabled ===");
    }
  }

  static bool IsEnabled() {
    return Instance().m_enabled;
  }

  static void SetLogPath(const std::wstring& path) {
    Instance().m_logPath = path;
  }

  void Log(const char* message) {
    if (!m_enabled) return;

    std::lock_guard<std::mutex> lock(m_mutex);

    std::ofstream file(m_logPath, std::ios::app);
    if (file.is_open()) {
      // Get timestamp
      time_t now = time(nullptr);
      char timeStr[32];
      strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", localtime(&now));

      file << "[" << timeStr << "] " << message << std::endl;
      file.close();
    }
  }

  void Log(const wchar_t* message) {
    if (!m_enabled) return;

    // Convert wide string to narrow for file output
    std::string narrow;
    narrow.reserve(wcslen(message));
    for (const wchar_t* p = message; *p; ++p) {
      if (*p < 128) {
        narrow += static_cast<char>(*p);
      } else {
        narrow += '?';  // Replace non-ASCII
      }
    }
    Log(narrow.c_str());
  }

  void LogFormat(const char* format, ...) {
    if (!m_enabled) return;

    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    Log(buffer);
  }

private:
  DebugLog() : m_enabled(false), m_logPath(L"winsplit_debug.log") {}
  ~DebugLog() = default;

  DebugLog(const DebugLog&) = delete;
  DebugLog& operator=(const DebugLog&) = delete;

  bool m_enabled;
  std::wstring m_logPath;
  std::mutex m_mutex;
};

// Convenience macros
#define DEBUG_LOG(msg) DebugLog::Instance().Log(msg)
#define DEBUG_LOG_FMT(fmt, ...) DebugLog::Instance().LogFormat(fmt, __VA_ARGS__)

#endif // DEBUG_LOG_H
