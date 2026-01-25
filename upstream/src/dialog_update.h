#ifndef __DIALOG_UPDATE_H__
#define __DIALOG_UPDATE_H__

#include <wx/wx.h>

/**
 * @brief Simple update notification dialog
 *
 * Shows version comparison and link to GitHub releases.
 * Does NOT download - just notifies and links.
 */
class UpdateNotificationDialog : public wxDialog {
private:
  wxString m_currentVersion;
  wxString m_newVersion;
  wxString m_releaseUrl;
  wxString m_releaseNotes;

  wxStaticText* p_titleText;
  wxStaticText* p_versionInfo;
  wxTextCtrl* p_releaseNotes;
  wxButton* p_btnViewGitHub;
  wxButton* p_btnRemindLater;
  wxButton* p_btnClose;

  void CreateControls();
  void CreateConnections();
  void OnViewGitHub(wxCommandEvent& event);
  void OnRemindLater(wxCommandEvent& event);
  void OnClose(wxCommandEvent& event);

public:
  UpdateNotificationDialog(wxWindow* parent,
                           const wxString& currentVersion,
                           const wxString& newVersion,
                           const wxString& releaseUrl,
                           const wxString& releaseNotes = wxEmptyString);
  ~UpdateNotificationDialog();
};

/**
 * @brief Dialog shown when no update is available (manual check)
 */
class NoUpdateDialog : public wxDialog {
public:
  NoUpdateDialog(wxWindow* parent, const wxString& currentVersion);
};

#endif // __DIALOG_UPDATE_H__
