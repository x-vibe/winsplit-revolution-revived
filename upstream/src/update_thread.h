#ifndef __WINSPLIT_UPDATES_H__
#define __WINSPLIT_UPDATES_H__

#include "settingsmanager.h"

#include <wx/tokenzr.h>

/**
 * @brief Background thread for checking GitHub releases for updates
 *
 * SECURITY: Uses HTTPS via WinInet to check GitHub API.
 * No longer uses insecure HTTP to winsplit-revolution.com.
 */
class ReadVersionThread : public wxThread {
private:
  SettingsManager& m_options;
  bool m_flagForceChecking;
  double m_host_version;
  wxString m_strVersion;
  wxString m_strReleaseUrl;
  wxString m_strReleaseNotes;
  unsigned int m_timeout;

public:
  ReadVersionThread(const unsigned int& timeout);
  ~ReadVersionThread();

  void ForceChecking();
  bool HaveToCheck();

  // Version info
  wxString GetVersionString() { return m_strVersion; }
  double GetHostVersion() { return m_host_version; }

  // GitHub release info
  wxString GetReleaseUrl() { return m_strReleaseUrl; }
  wxString GetReleaseNotes() { return m_strReleaseNotes; }

  // Legacy compatibility (deprecated - use GetVersionString instead)
  wxString GetFeatures() { return m_strVersion; }

  void* Entry();
};

#endif // __WINSPLIT_UPDATES_H__
