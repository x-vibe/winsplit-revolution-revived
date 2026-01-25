#include "dialog_update.h"

#include <wx/statline.h>
#include <wx/hyperlink.h>

#include "../images/icone.xpm"

// ============================================================================
// UpdateNotificationDialog - Shows when a new version is available
// ============================================================================

UpdateNotificationDialog::UpdateNotificationDialog(wxWindow* parent,
                                                   const wxString& currentVersion,
                                                   const wxString& newVersion,
                                                   const wxString& releaseUrl,
                                                   const wxString& releaseNotes)
    : wxDialog(parent, wxID_ANY, _("Update Available"), wxDefaultPosition, wxDefaultSize,
               wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
    , m_currentVersion(currentVersion)
    , m_newVersion(newVersion)
    , m_releaseUrl(releaseUrl)
    , m_releaseNotes(releaseNotes)
    , p_titleText(NULL)
    , p_versionInfo(NULL)
    , p_releaseNotes(NULL)
    , p_btnViewGitHub(NULL)
    , p_btnRemindLater(NULL)
    , p_btnClose(NULL)
{
  CreateControls();
  CreateConnections();

  SetIcon(wxIcon(icone_xpm));
  GetSizer()->Fit(this);
  GetSizer()->SetSizeHints(this);
  Centre();
}

UpdateNotificationDialog::~UpdateNotificationDialog() {}

void UpdateNotificationDialog::CreateControls()
{
  wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
  SetSizer(mainSizer);

  // Title
  p_titleText = new wxStaticText(this, wxID_ANY,
      _("A new version of WinSplit Revolution is available!"));
  wxFont titleFont = p_titleText->GetFont();
  titleFont.SetPointSize(titleFont.GetPointSize() + 2);
  titleFont.SetWeight(wxFONTWEIGHT_BOLD);
  p_titleText->SetFont(titleFont);
  mainSizer->Add(p_titleText, 0, wxALL | wxALIGN_CENTER, 15);

  // Version comparison
  wxString versionText;
  versionText << _("Current version: ") << m_currentVersion << _T("\n")
              << _("New version: ") << m_newVersion;
  p_versionInfo = new wxStaticText(this, wxID_ANY, versionText);
  mainSizer->Add(p_versionInfo, 0, wxLEFT | wxRIGHT | wxALIGN_CENTER, 15);

  mainSizer->AddSpacer(10);

  // Release notes (if available)
  if (!m_releaseNotes.IsEmpty()) {
    wxStaticText* notesLabel = new wxStaticText(this, wxID_ANY, _("What's new:"));
    mainSizer->Add(notesLabel, 0, wxLEFT | wxTOP, 15);

    p_releaseNotes = new wxTextCtrl(this, wxID_ANY, m_releaseNotes,
        wxDefaultPosition, wxSize(400, 150),
        wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH | wxTE_AUTO_URL);
    mainSizer->Add(p_releaseNotes, 1, wxALL | wxEXPAND, 15);
  }

  // Separator
  mainSizer->Add(new wxStaticLine(this), 0, wxEXPAND | wxLEFT | wxRIGHT, 15);

  // Buttons
  wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);

  p_btnViewGitHub = new wxButton(this, wxID_ANY, _("View on GitHub"));
  p_btnViewGitHub->SetDefault();
  buttonSizer->Add(p_btnViewGitHub, 0, wxALL, 5);

  buttonSizer->AddStretchSpacer();

  p_btnRemindLater = new wxButton(this, wxID_ANY, _("Remind Me Later"));
  buttonSizer->Add(p_btnRemindLater, 0, wxALL, 5);

  p_btnClose = new wxButton(this, wxID_CANCEL, _("Close"));
  buttonSizer->Add(p_btnClose, 0, wxALL, 5);

  mainSizer->Add(buttonSizer, 0, wxEXPAND | wxALL, 10);
}

void UpdateNotificationDialog::CreateConnections()
{
  p_btnViewGitHub->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(UpdateNotificationDialog::OnViewGitHub), NULL, this);
  p_btnRemindLater->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(UpdateNotificationDialog::OnRemindLater), NULL, this);
  p_btnClose->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(UpdateNotificationDialog::OnClose), NULL, this);
}

void UpdateNotificationDialog::OnViewGitHub(wxCommandEvent& event)
{
  // Open GitHub releases page in default browser
  wxLaunchDefaultBrowser(m_releaseUrl);
  EndModal(wxOK);
}

void UpdateNotificationDialog::OnRemindLater(wxCommandEvent& event)
{
  // Just close - will check again next time based on settings
  EndModal(wxID_CANCEL);
}

void UpdateNotificationDialog::OnClose(wxCommandEvent& event)
{
  EndModal(wxID_CANCEL);
}

// ============================================================================
// NoUpdateDialog - Shows when user manually checks and no update is available
// ============================================================================

NoUpdateDialog::NoUpdateDialog(wxWindow* parent, const wxString& currentVersion)
    : wxDialog(parent, wxID_ANY, _("No Update Available"), wxDefaultPosition, wxDefaultSize,
               wxDEFAULT_DIALOG_STYLE)
{
  wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
  SetSizer(mainSizer);

  // Icon and message
  wxStaticText* message = new wxStaticText(this, wxID_ANY,
      _("You are running the latest version of WinSplit Revolution."));
  mainSizer->Add(message, 0, wxALL | wxALIGN_CENTER, 20);

  wxString versionText;
  versionText << _("Current version: ") << currentVersion;
  wxStaticText* versionLabel = new wxStaticText(this, wxID_ANY, versionText);
  mainSizer->Add(versionLabel, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxALIGN_CENTER, 20);

  // OK button
  wxButton* btnOK = new wxButton(this, wxID_OK, _("OK"));
  btnOK->SetDefault();
  mainSizer->Add(btnOK, 0, wxALL | wxALIGN_CENTER, 10);

  SetIcon(wxIcon(icone_xpm));
  GetSizer()->Fit(this);
  GetSizer()->SetSizeHints(this);
  Centre();
}
