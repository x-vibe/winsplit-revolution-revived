#include "settingsmanager.h"

#include "main.h"
#include "virtual_key_manager.h"

#include <clocale>
#include <algorithm>  // for std::clamp (C++17) or manual clamp

// Security: Helper to clamp integer values to safe ranges
namespace {
  template<typename T>
  T clampValue(long value, T minVal, T maxVal) {
    if (value < static_cast<long>(minVal)) return minVal;
    if (value > static_cast<long>(maxVal)) return maxVal;
    return static_cast<T>(value);
  }
}
#include <wx/filename.h>
#include <wx/msgdlg.h>
#include <wx/msw/registry.h>
#include <wx/stdpaths.h>

// Definition of the "singleton" object
SettingsManager* SettingsManager::m_instance = NULL;

const wxLanguage SettingsManager::AvailableLangIds[] = {wxLANGUAGE_ENGLISH,
                                                        wxLANGUAGE_FRENCH,
                                                        wxLANGUAGE_ITALIAN,
                                                        wxLANGUAGE_SPANISH,
                                                        wxLANGUAGE_GERMAN,
                                                        wxLANGUAGE_CATALAN,
                                                        wxLANGUAGE_DUTCH,
                                                        wxLANGUAGE_PORTUGUESE,
                                                        wxLANGUAGE_CZECH,
                                                        wxLANGUAGE_POLISH,
                                                        wxLANGUAGE_CHINESE_TRADITIONAL,
                                                        wxLANGUAGE_CHINESE_SIMPLIFIED,
                                                        wxLANGUAGE_ARABIC,
                                                        wxLANGUAGE_BENGALI,
                                                        wxLANGUAGE_GREEK,
                                                        wxLANGUAGE_HINDI,
                                                        wxLANGUAGE_INDONESIAN,
                                                        wxLANGUAGE_JAPANESE,
                                                        wxLANGUAGE_KOREAN,
                                                        wxLANGUAGE_FARSI,
                                                        wxLANGUAGE_ROMANIAN,
                                                        wxLANGUAGE_RUSSIAN,
                                                        wxLANGUAGE_THAI,
                                                        wxLANGUAGE_TURKISH,
                                                        wxLANGUAGE_UKRAINIAN,
                                                        wxLANGUAGE_VIETNAMESE};

const wxString SettingsManager::AvailableLangNames[] = {_T ("English"),
                                                        _T ("French"),
                                                        _T ("Italian"),
                                                        _T ("Spanish"),
                                                        _T ("German"),
                                                        _T ("Catalan"),
                                                        _T ("Dutch"),
                                                        _T ("Portuguese"),
                                                        _T ("Czech"),
                                                        _T ("Polish"),
                                                        _T ("Chinese Traditional"),
                                                        _T ("Chinese Simplified"),
                                                        _T ("Arabic"),
                                                        _T ("Bengali"),
                                                        _T ("Greek"),
                                                        _T ("Hindi"),
                                                        _T ("Indonesian"),
                                                        _T ("Japanese"),
                                                        _T ("Korean"),
                                                        _T ("Persian"),
                                                        _T ("Romanian"),
                                                        _T ("Russian"),
                                                        _T ("Thai"),
                                                        _T ("Turkish"),
                                                        _T ("Ukrainian"),
                                                        _T ("Vietnamese")};

SettingsManager::SettingsManager()
{
  setlocale(LC_NUMERIC, "C");
  m_bInitialized = false;
  m_bPortableMode = false;
  m_bIsModified = false;
}

SettingsManager::~SettingsManager()
{
  //
}

SettingsManager& SettingsManager::Get()
{
  if (m_instance == NULL) {
    m_instance = new SettingsManager();
    m_instance->Initialize();
  }
  return *m_instance;
}

bool SettingsManager::IsOk()
{
  if (!m_bInitialized)
    return false;

  return true;
}

wxString SettingsManager::GetDataDirectory()
{
  return m_sUserDataDir;
}

wxString SettingsManager::getUserName()
{
  return m_sUserName;
}

void SettingsManager::Kill()
{
  if (m_instance != NULL) {
    if (m_instance->m_bIsModified)
      m_instance->SaveSettings();
  }
  delete m_instance;
  m_instance = NULL;
}

int SettingsManager::GetAvailableLanguagesCount()
{
  return (sizeof(AvailableLangIds) / sizeof(wxLanguage));
}

wxString SettingsManager::getAppPath()
{
  return m_sAppPath;
}

bool SettingsManager::getSaveNumpadPosOnExit()
{
  return m_bVN_SavePosOnExit;
}

void SettingsManager::setSaveNumpadPosOnExit(bool save)
{
  if (save != m_bVN_SavePosOnExit) {
    m_bVN_SavePosOnExit = save;
    m_bIsModified = true;
  }
}

bool SettingsManager::getShowNumpadAtBoot()
{
  return m_bVN_ShownAtBoot;
}

void SettingsManager::setShowNumpadAtBoot(bool show)
{
  if (show != m_bVN_ShownAtBoot) {
    m_bVN_ShownAtBoot = show;
    m_bIsModified = true;
  }
}

bool SettingsManager::IsPortable()
{
  return m_bPortableMode;
}

bool SettingsManager::AcceptTopMostWindows()
{
  return m_bAcceptTopmostWindows;
}

void SettingsManager::setAcceptTopMostWindows(bool do_it)
{
  if (do_it != m_bAcceptTopmostWindows) {
    m_bAcceptTopmostWindows = do_it;
    m_bIsModified = true;
  }
}

bool SettingsManager::hasToShowHotkeysWarnings()
{
  return m_bShowHKWarnings;
}

void SettingsManager::setShowHotkeysWarnings(bool show)
{
  if (show != m_bShowHKWarnings) {
    m_bShowHKWarnings = show;
    m_bIsModified = true;
  }
}

bool SettingsManager::hasToCheckForUpdates()
{
  return m_bCheckForUpdates;
}

void SettingsManager::setCheckForUpdates(bool check)
{
  if (check != m_bCheckForUpdates) {
    m_bCheckForUpdates = check;
    m_bIsModified = true;
  }
}

time_t SettingsManager::getLastCheckDate()
{
  return m_tLastUpdateCheck;
}

void SettingsManager::setLastCheckDate(time_t lastdate)
{
  if (lastdate != m_tLastUpdateCheck) {
    m_tLastUpdateCheck = lastdate;
    m_bIsModified = true;
  }
}

int SettingsManager::getUpdateCheckFrequency()
{
  return m_iUpdateCheckFrequency;
}

void SettingsManager::setUpdateCheckFrequency(int freqIndex)
{
  if (freqIndex != m_iUpdateCheckFrequency) {
    m_iUpdateCheckFrequency = freqIndex;
    m_bIsModified = true;
  }
}

int SettingsManager::getLanguageIndex()
{
  return m_iLanguage;
}

void SettingsManager::setLanguageIndex(int index)
{
  if (index >= GetAvailableLanguagesCount()) {
    wxMessageBox(_("Language non supported in WinSplit !"), _("Error"), wxICON_ERROR);
    return;
  }
  if (index != m_iLanguage) {
    if (!m_locale.Init(AvailableLangIds[index])) {
      wxMessageBox(_T ("Unable to initialize language !"), _("Error"), wxICON_ERROR);
    }
    else {
      m_locale.AddCatalog(_T ("winsplit"));
      m_iLanguage = index;
      m_bIsModified = true;
    }
  }
  setlocale(LC_NUMERIC, "C");
}

void SettingsManager::setAutoDeleteTempFiles(bool del)
{
  m_bAutoDelTmpFiles = del;
}

bool SettingsManager::getAutoDeleteTempFiles()
{
  return m_bAutoDelTmpFiles;
}

void SettingsManager::setAutoDeleteTime(int value)
{
  /*
   * the variable can have the following values:
   * 0 = delete files when WinSplit starts
   * 1 = delete files when WinSplit finishes
   */
  m_iAutoDelTime = value;
}

int SettingsManager::getAutoDeleteTime()
{
  return m_iAutoDelTime;
}

int SettingsManager::getNumpadTransparency()
{
  return m_iVN_Transparency;
}

void SettingsManager::setNumpadTransparency(int value)
{
  if (value != m_iVN_Transparency) {
    m_iVN_Transparency = value;
    m_bIsModified = true;
  }
}

bool SettingsManager::getNumpadStyle()
{
  return m_bVN_Reduced;
}

void SettingsManager::setNumpadStyle(bool reduced)
{
  if (reduced != m_bVN_Reduced) {
    m_bVN_Reduced = reduced;
    m_bIsModified = true;
  }
}

bool SettingsManager::getNumpadAutoHide()
{
  return m_bVN_AutoHide;
}

void SettingsManager::setNumpadAutoHide(bool autoHide)
{
  if (autoHide != m_bVN_AutoHide) {
    m_bVN_AutoHide = autoHide;
    m_bIsModified = true;
  }
}

wxPoint SettingsManager::getNumpadPosition()
{
  return wxPoint(m_iVN_PosX, m_iVN_PosY);
}

void SettingsManager::setNumpadPosition(wxPoint pos)
{
  if ((pos.x != m_iVN_PosX) || (pos.y != m_iVN_PosY)) {
    m_iVN_PosX = pos.x;
    m_iVN_PosY = pos.y;
    m_bIsModified = true;
  }
}

bool SettingsManager::IsDragNGoEnabled()
{
  return m_bDNG_Enabled;
}

void SettingsManager::EnableDragNGo(bool enable)
{
  if (enable != m_bDNG_Enabled) {
    m_bDNG_Enabled = enable;
    m_bIsModified = true;
  }
}

int SettingsManager::getDnGTimerFrequency()
{
  return m_iDNG_timer;
}

void SettingsManager::setDnGTimerFrequency(int freq)
{
  if (freq != m_iDNG_timer) {
    m_iDNG_timer = freq;
    m_bIsModified = true;
  }
}

int SettingsManager::getDnGDetectionRadius()
{
  return m_iDNG_Radius;
}

void SettingsManager::setDnGDetectionRadius(int radius)
{
  if (radius != m_iDNG_Radius) {
    m_iDNG_Radius = radius;
    m_bIsModified = true;
  }
}

wxColour SettingsManager::getDnGZoneBgColor()
{
  return m_cDNG_BgColor;
}

void SettingsManager::setDnGZoneBgColor(wxColour col)
{
  if (col != m_cDNG_BgColor) {
    m_cDNG_BgColor = col;
    m_bIsModified = true;
  }
}

wxColour SettingsManager::getDnGZoneFgColor()
{
  return m_cDNG_FgColor;
}

void SettingsManager::setDnGZoneFgColor(wxColour col)
{
  if (col != m_cDNG_FgColor) {
    m_cDNG_FgColor = col;
    m_bIsModified = true;
  }
}

int SettingsManager::getDngZoneTransparency()
{
  return m_iDNG_Transparency;
}

void SettingsManager::setDnGZoneTransparency(int value)
{
  if (value != m_iDNG_Transparency) {
    m_iDNG_Transparency = value;
    m_bIsModified = true;
  }
}

void SettingsManager::setDnGMod1(int value)
{
  if (value != (int)m_modDNG1) {
    m_modDNG1 = value;
    m_bIsModified = true;
  }
}

void SettingsManager::setDnGMod2(int value)
{
  if (value != (int)m_modDNG2) {
    m_modDNG2 = value;
    m_bIsModified = true;
  }
}

unsigned int SettingsManager::getDnGMod1()
{
  return m_modDNG1;
}

unsigned int SettingsManager::getDnGMod2()
{
  return m_modDNG2;
}

void SettingsManager::setDnGMod1(const unsigned int& mod)
{
  if (mod != m_modDNG1) {
    m_modDNG1 = mod;
    m_bIsModified = true;
  }
}

void SettingsManager::setDnGMod2(const unsigned int& mod)
{
  if (mod != m_modDNG2) {
    m_modDNG2 = mod;
    m_bIsModified = true;
  }
}

void SettingsManager::setXMouseActivation(bool enable)
{
  long lVal = (enable) ? 1 : 0;
  wxRegKey rKey(_T ("HKEY_CURRENT_USER\\Control Panel\\Mouse"));
  if (rKey.HasValue(_T ("ActiveWindowTracking"))) {
    rKey.SetValue(_T ("ActiveWindowTracking"), lVal);
    return;
  }

  rKey.SetName(_T ("HKEY_CURRENT_USER\\Control Panel\\Desktop"));
  wxMemoryBuffer mBuff;
  rKey.QueryValue(_T ("UserPreferencesMask"), mBuff);
  unsigned char c = *(unsigned char*)(mBuff.GetData());
  if (enable)
    c |= 1;
  else
    c &= 0xfe;
  *(unsigned char*)(mBuff.GetData()) = c;
  rKey.SetValue(_T ("UserPreferencesMask"), mBuff);
}

bool SettingsManager::IsXMouseActivated()
{
  wxMemoryBuffer mBuff;
  long lVal;

  wxRegKey rKey(_T ("HKEY_CURRENT_USER\\Control Panel\\Mouse"));
  if (rKey.QueryValue(_T ("ActiveWindowTracking"), &lVal))
    return (lVal) ? true : false;

  rKey.SetName(_T ("HKEY_CURRENT_USER\\Control Panel\\Desktop"));
  rKey.QueryValue(_T ("UserPreferencesMask"), mBuff);
  char c = *(char*)(mBuff.GetData());
  return ((c & 1) == 1);
}

void SettingsManager::setAutoZOrderActivation(bool enable)
{
  wxRegKey rKey(_T ("HKEY_CURRENT_USER\\Control Panel\\Desktop"));
  wxMemoryBuffer mBuff;
  rKey.QueryValue(_T ("UserPreferencesMask"), mBuff);
  unsigned char c = *(unsigned char*)(mBuff.GetData());
  if (enable)
    c |= 0x40;
  else
    c &= 0xbf;
  *(unsigned char*)(mBuff.GetData()) = c;
  rKey.SetValue(_T ("UserPreferencesMask"), mBuff);
}

bool SettingsManager::IsAutoZOrderActivated()
{
  wxRegKey rKey(_T ("HKEY_CURRENT_USER\\Control Panel\\Desktop"));
  wxMemoryBuffer mBuff;
  rKey.QueryValue(_T ("UserPreferencesMask"), mBuff);
  char c = *(char*)(mBuff.GetData());
  return ((c & 0x40) == 0x40);
}

void SettingsManager::setAutoZOrderDelay(int value)
{
  wxRegKey rKey(_T ("HKEY_CURRENT_USER\\Control Panel\\Desktop"));
  if (rKey.HasValue(_T ("ActiveWndTrackTimeout")))
    rKey.SetValue(_T ("ActiveWndTrackTimeout"), (long)value);
  else
    rKey.SetValue(_T ("ActiveWndTrkTimeout"), (long)value);
}

int SettingsManager::getAutoZOrderDelay()
{
  wxRegKey rKey(_T ("HKEY_CURRENT_USER\\Control Panel\\Desktop"));
  long lVal = 0;
  if (rKey.HasValue(_T ("ActiveWndTrackTimeout")))
    rKey.QueryValue(_T ("ActiveWndTrackTimeout"), &lVal);
  else
    rKey.QueryValue(_T ("ActiveWndTrkTimeout"), &lVal);
  return (int)lVal;
}

void SettingsManager::setMouseFollowWindow(bool value)
{
  if (value != m_bMouseFollowWnd) {
    m_bMouseFollowWnd = value;
    m_bIsModified = true;
  }
}

bool SettingsManager::getMouseFollowWindow()
{
  return m_bMouseFollowWnd;
}

void SettingsManager::setMouseFollowOnlyWhenIn(bool value)
{
  if (value != m_bMouseFollowOnlyWhenIn) {
    m_bMouseFollowOnlyWhenIn = value;
    m_bIsModified = true;
  }
}

bool SettingsManager::getMouseFollowOnlyWhenIn()
{
  return m_bMouseFollowOnlyWhenIn;
}

void SettingsManager::setMinMaxCycle(bool value)
{
  if (value != m_bMinMaxCycle) {
    m_bMinMaxCycle = value;
    m_bIsModified = true;
  }
}

bool SettingsManager::getMinMaxCycle()
{
  return m_bMinMaxCycle;
}

void SettingsManager::Initialize()
{
  // Do not call this method twice
  if (m_bInitialized)
    return;

  // Retrieve the user name
  m_sUserName = wxGetUserId();

  m_sUserDataDir = wxEmptyString;
  wxFileName FName(wxGetApp().argv[0]);
  m_sAppPath = FName.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);

  // Set working directory to the directory of the application
  wxSetWorkingDirectory(m_sAppPath);
  // Check if settings file exists in the application directory. If it is, we are in portable mode.
  FName.SetFullName(_T ("Settings.xml"));
  m_bPortableMode = FName.FileExists();

  if (!m_bPortableMode) {
    // We are in "Classic" mode
    FName.SetPath(wxStandardPaths::Get().GetUserDataDir());
    // Create the directory if it doesn't exist
    if (!wxDirExists(FName.GetPath(wxPATH_GET_VOLUME)))
      wxMkdir(FName.GetPath(wxPATH_GET_VOLUME));
  }
  m_sUserDataDir = FName.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);

  // Default values of variables
  // Warn about errors with Hotkeys
  m_bShowHKWarnings = true;
  // Update checking uses secure HTTPS via GitHub Releases API
  m_bCheckForUpdates = true;
  m_iUpdateCheckFrequency = CHECK_UPDATES_ON_START;
  // Default language = system language
  m_iLanguage = wxLANGUAGE_DEFAULT;
  // The path where the language files are located
  wxLocale::AddCatalogLookupPathPrefix(m_sAppPath + _T ("languages"));

  // Initialize the variable wxLocale with the default language of the system
  // Defaults to English if the language is not supported by wxWidgets so just ignore errors
  m_locale.Init(wxLANGUAGE_DEFAULT);

  m_locale.AddCatalog(_T ("winsplit"));
  // Check if the system language corresponds to a language supported by Winsplit
  int iCount = GetAvailableLanguagesCount();
  for (int i = 0; i < iCount; i++) {
    if (AvailableLangIds[i] == m_locale.GetLanguage()) {
      m_iLanguage = i;
      break;
    }
  }

  // Use decimal numeric separators
  setlocale(LC_NUMERIC, "C");
  // Behaviour of temporary screen shot files
  m_bAutoDelTmpFiles = true; // Auto delete
  m_iAutoDelTime = 0;        // Supress winsplit start up
  // Position, style, behaviour and transparency of Virtual Numpad
  m_iVN_Transparency = 65;
  m_iVN_PosX = 640;
  m_iVN_PosY = 480;
  m_bVN_Reduced = false;
  m_bVN_AutoHide = false;
  m_bVN_ShownAtBoot = false;
  m_bVN_SavePosOnExit = true;
  // Do not take into account the "TopMost" windows
  m_bAcceptTopmostWindows = false;
  m_tLastUpdateCheck = time(NULL);

  // Drag'N'Go settings
  m_bDNG_Enabled = true;
  m_iDNG_Radius = 100;
  m_iDNG_timer = 100;
  m_cDNG_BgColor = wxColour(65, 105, 225);
  m_cDNG_FgColor = *wxWHITE;
  m_iDNG_Transparency = 45;
  m_modDNG1 = 0x02; // MOD_CONTROL
  m_modDNG2 = 0x01; // MOD_ALT

  // By default, activate the option "Mouse follows window"
  m_bMouseFollowWnd = false;
  // By default the mouse only follows the window if it is in its client area
  m_bMouseFollowOnlyWhenIn = true;
  // By default, the "Minimize" and "Maximize" Hotkeys keep their classic operation
  m_bMinMaxCycle = false;

  // Try to read the options from the xml file
  LoadSettings();

  m_bInitialized = true;
}

void SettingsManager::LoadSettings()
{
  // The name of the file containing the settings
  wxFileName fname(m_sUserDataDir + _T ("Settings.xml"));
  // If the settings file does not exist
  if (!fname.FileExists()) {
    // make sure that Winsplit creates it and declare the default settings as having been modified
    m_bIsModified = true;
    return;
  }

  wxXmlDocument doc;
  wxXmlNode* node;
  wxString sValue;

  // Try to load the file containing the settings
  doc.Load(fname.GetFullPath());
  if (!doc.IsOk()) {
    // If there was an error loading, we make sure that settings are rewritten
    m_bIsModified = true;
    return;
  }

  node = doc.GetRoot()->GetChildren();
  while (node) {
    wxString sectionName = node->GetName();

    if (sectionName == _T ("General")) {
      ReadGeneralSettings(node);
    }
    else if (sectionName == _T ("VirtualNumpad") || sectionName == _T ("Numpad")) {
      ReadPopupSettings(node);
    }
    else if (sectionName == _T ("WebUpdate") || sectionName == _T ("WebUpdates")) {
      ReadWebUpdateSettings(node);
    }
    else if (sectionName == _T ("Drag_N_Go") || sectionName == _T ("DragAndGo")) {
      ReadDragNGoSettings(node);
    }
    else if (sectionName == _T ("Miscellaneous")) {
      ReadMiscSettings(node);
    }

    node = node->GetNext();
  }
  m_bIsModified = false;
}

void SettingsManager::SaveSettings()
{
  wxXmlDocument doc;
  wxXmlNode *node, *root;
  wxXmlAttribute* prop;

  root = new wxXmlNode(NULL, wxXML_ELEMENT_NODE, _T ("WinSplit_Settings"));
  prop = new wxXmlAttribute(_T ("Version"), _T ("1.0"));
  root->SetAttributes(prop);

  root->AddChild(node = new wxXmlNode(NULL, wxXML_ELEMENT_NODE, _T ("General")));
  SaveGeneralSettings(node);

  node->SetNext(new wxXmlNode(NULL, wxXML_ELEMENT_NODE, _T ("VirtualNumpad")));
  node = node->GetNext();
  SavePopupSettings(node);

  node->SetNext(new wxXmlNode(NULL, wxXML_ELEMENT_NODE, _T ("WebUpdate")));
  node = node->GetNext();
  SaveWebUpdateSettings(node);

  node->SetNext(new wxXmlNode(NULL, wxXML_ELEMENT_NODE, _T ("Drag_N_Go")));
  node = node->GetNext();
  SaveDragNGoSettings(node);

  node->SetNext(new wxXmlNode(NULL, wxXML_ELEMENT_NODE, _T ("Miscellaneous")));
  node = node->GetNext();
  SaveMiscSettings(node);

  wxFileName fname(m_sUserDataDir + _T ("Settings.xml"));
  if (fname.FileExists())
    wxRemoveFile(fname.GetFullPath());

  doc.SetRoot(root);
  doc.Save(fname.GetFullPath(), 2);
}

void SettingsManager::ReadGeneralSettings(wxXmlNode* container)
{
  wxXmlNode* node = container->GetChildren();
  wxString nodName, sValue;
  long l;

  // Helper to safely get node value (supports both attribute and text content formats)
  auto getNodeValue = [](wxXmlNode* n) -> wxString {
    // First try attribute format: <Node Value="x"/>
    if (n->GetAttributes()) {
      return n->GetAttributes()->GetValue();
    }
    // Fall back to text content format: <Node>x</Node>
    return n->GetNodeContent();
  };

  while (node) {
    nodName = node->GetName();

    if (nodName == _T ("AcceptTopMostWindow")) {
      m_bAcceptTopmostWindows = (getNodeValue(node) == _T ("True") || getNodeValue(node) == _T ("1"));
    }
    else if (nodName == _T ("ShowHotKeysWarnings")) {
      m_bShowHKWarnings = (getNodeValue(node) == _T ("True") || getNodeValue(node) == _T ("1"));
    }
    else if (nodName == _T ("Language")) {
      if (getNodeValue(node).ToLong(&l)) {
        // Security: Validate language index is within bounds
        if (l >= 0 && l < GetAvailableLanguagesCount()) {
          setLanguageIndex(static_cast<int>(l));
        }
      }
    }
    else if (nodName == _T ("TempFiles")) {
      if (node->GetAttribute(_T ("AutoDelete"), &sValue)) {
        m_bAutoDelTmpFiles = (sValue == _T ("True"));
      }
      if (node->GetAttribute(_T ("AutoDelTime"), &sValue)) {
        sValue.ToLong(&l);
        m_iAutoDelTime = int(l);
      }
    }
    // Handle Settings.xml format with different node names
    else if (nodName == _T ("TopmostWindows")) {
      m_bAcceptTopmostWindows = (getNodeValue(node) == _T ("1"));
    }
    else if (nodName == _T ("CheckForUpdates")) {
      m_bCheckForUpdates = (getNodeValue(node) == _T ("1"));
    }
    else if (nodName == _T ("UpdateCheckFrequency")) {
      if (getNodeValue(node).ToLong(&l)) {
        m_iUpdateCheckFrequency = static_cast<int>(l);
      }
    }
    else if (nodName == _T ("DeleteScreenshotsAtStart")) {
      if (getNodeValue(node) == _T ("1")) {
        m_bAutoDelTmpFiles = true;
        m_iAutoDelTime = 0;
      }
    }
    else if (nodName == _T ("DeleteScreenshotsAtEnd")) {
      if (getNodeValue(node) == _T ("1")) {
        m_bAutoDelTmpFiles = true;
        m_iAutoDelTime = 1;
      }
    }

    node = node->GetNext();
  }
}

void SettingsManager::SaveGeneralSettings(wxXmlNode* container)
{
  wxXmlNode* node;
  container->AddChild(
      node = new wxXmlNode(
          NULL,
          wxXML_ELEMENT_NODE,
          _T ("AcceptTopMostWindow"),
          wxEmptyString,
          new wxXmlAttribute(_T ("Value"), m_bAcceptTopmostWindows ? _T ("True") : _T ("False"))));

  node->SetNext(new wxXmlNode(
      NULL,
      wxXML_ELEMENT_NODE,
      _T ("ShowHotKeysWarnings"),
      wxEmptyString,
      new wxXmlAttribute(_T ("Value"), m_bShowHKWarnings ? _T ("True") : _T ("False"))));
  node = node->GetNext();

  node->SetNext(
      new wxXmlNode(NULL,
                    wxXML_ELEMENT_NODE,
                    _T ("Language"),
                    wxEmptyString,
                    new wxXmlAttribute(_T ("Value"), wxString::Format(_T ("%0d"), m_iLanguage))));
  node = node->GetNext();

  node->SetNext(new wxXmlNode(NULL, wxXML_ELEMENT_NODE, _T ("TempFiles")));
  node = node->GetNext();
  node->AddAttribute(_T ("AutoDelete"), m_bAutoDelTmpFiles ? _T ("True") : _T ("False"));
  node->AddAttribute(_T ("AutoDelTime"), wxString::Format(_T ("%0d"), m_iAutoDelTime));
}

void SettingsManager::ReadPopupSettings(wxXmlNode* container)
{
  wxXmlNode* node = container->GetChildren();
  wxString nodName, sValue;
  long l;

  // Helper to safely get node value (supports both attribute and text content formats)
  auto getNodeValue = [](wxXmlNode* n) -> wxString {
    if (n->GetAttributes()) return n->GetAttributes()->GetValue();
    return n->GetNodeContent();
  };

  while (node) {
    nodName = node->GetName();

    // Original format with attributes
    if (nodName == _T ("Position")) {
      if (node->GetAttribute(_T ("X"), &sValue)) {
        sValue.ToLong(&l);
        m_iVN_PosX = clampValue<int>(l, -10000, 10000);
      }
      if (node->GetAttribute(_T ("Y"), &sValue)) {
        sValue.ToLong(&l);
        m_iVN_PosY = clampValue<int>(l, -10000, 10000);
      }
    }
    else if (nodName == _T ("Style")) {
      // Could be attribute or text content
      if (node->GetAttribute(_T ("Reduced"), &sValue)) {
        m_bVN_Reduced = (sValue == _T ("True"));
      }
      if (node->GetAttribute(_T ("Transparency"), &sValue)) {
        sValue.ToLong(&l);
        m_iVN_Transparency = clampValue<int>(l, 0, 100);
      }
      // Settings.xml format: <Style>1</Style>
      wxString val = getNodeValue(node);
      if (!val.IsEmpty() && val.IsNumber()) {
        val.ToLong(&l);
        m_bVN_Reduced = (l == 1);
      }
    }
    else if (nodName == _T ("Comportment")) {
      if (node->GetAttribute(_T ("ShowAtBoot"), &sValue)) {
        m_bVN_ShownAtBoot = (sValue == _T ("True"));
      }
      if (node->GetAttribute(_T ("SaveAtExit"), &sValue)) {
        m_bVN_SavePosOnExit = (sValue == _T ("True"));
      }
      if (node->GetAttribute(_T ("AutoHide"), &sValue)) {
        m_bVN_AutoHide = (sValue == _T ("True"));
      }
    }
    // Settings.xml format (text content nodes)
    else if (nodName == _T ("PosX")) {
      if (getNodeValue(node).ToLong(&l))
        m_iVN_PosX = clampValue<int>(l, -10000, 10000);
    }
    else if (nodName == _T ("PosY")) {
      if (getNodeValue(node).ToLong(&l))
        m_iVN_PosY = clampValue<int>(l, -10000, 10000);
    }
    else if (nodName == _T ("Opacity")) {
      if (getNodeValue(node).ToLong(&l))
        m_iVN_Transparency = clampValue<int>(l, 0, 255);
    }
    else if (nodName == _T ("SaveState")) {
      m_bVN_SavePosOnExit = (getNodeValue(node) == _T ("1"));
    }
    else if (nodName == _T ("ShowAtStartup")) {
      m_bVN_ShownAtBoot = (getNodeValue(node) == _T ("1"));
    }
    else if (nodName == _T ("HideAfterSelection")) {
      m_bVN_AutoHide = (getNodeValue(node) == _T ("1"));
    }

    node = node->GetNext();
  }
}

void SettingsManager::SavePopupSettings(wxXmlNode* container)
{
  wxXmlNode* node;

  container->AddChild(node = new wxXmlNode(NULL, wxXML_ELEMENT_NODE, _T ("Position")));
  node->AddAttribute(_T ("X"), wxString::Format(_T ("%0d"), m_iVN_PosX));
  node->AddAttribute(_T ("Y"), wxString::Format(_T ("%0d"), m_iVN_PosY));

  node->SetNext(new wxXmlNode(NULL, wxXML_ELEMENT_NODE, _T ("Style")));
  node = node->GetNext();
  node->AddAttribute(_T ("Reduced"), m_bVN_Reduced ? _T ("True") : _T ("False"));
  node->AddAttribute(_T ("Transparency"), wxString::Format(_T ("%0d"), m_iVN_Transparency));

  node->SetNext(new wxXmlNode(NULL, wxXML_ELEMENT_NODE, _T ("Comportment")));
  node = node->GetNext();
  node->AddAttribute(_T ("ShowAtBoot"), (m_bVN_ShownAtBoot ? _T ("True") : _T ("False")));
  node->AddAttribute(_T ("SaveAtExit"), (m_bVN_SavePosOnExit ? _T ("True") : _T ("False")));
  wxString sTmp = (m_bVN_AutoHide ? _T ("True") : _T ("False"));
  node->AddAttribute(_T ("AutoHide"), sTmp);
}

void SettingsManager::ReadWebUpdateSettings(wxXmlNode* container)
{
  wxXmlNode* node = container->GetChildren();
  wxString nodName, sValue;
  long l;

  // Helper to safely get node value (supports both attribute and text content formats)
  auto getNodeValue = [](wxXmlNode* n) -> wxString {
    if (n->GetAttributes()) return n->GetAttributes()->GetValue();
    return n->GetNodeContent();
  };

  while (node) {
    nodName = node->GetName();
    wxString val = getNodeValue(node);

    if (nodName == _T ("AutoCheck")) {
      m_bCheckForUpdates = (val == _T ("True") || val == _T ("1"));
    }
    else if (nodName == _T ("Frequency")) {
      if (val.ToLong(&l))
        m_iUpdateCheckFrequency = static_cast<int>(l);
    }
    else if (nodName == _T ("LastCheck")) {
      if (val.ToLong(&l))
        m_tLastUpdateCheck = static_cast<time_t>(l);
    }

    node = node->GetNext();
  }
}

void SettingsManager::SaveWebUpdateSettings(wxXmlNode* container)
{
  wxXmlNode* node;

  container->AddChild(node = new wxXmlNode(NULL, wxXML_ELEMENT_NODE, _T ("AutoCheck")));
  node->AddAttribute(_T ("Value"), m_bCheckForUpdates ? _T ("True") : _T ("False"));

  node->SetNext(new wxXmlNode(NULL, wxXML_ELEMENT_NODE, _T ("Frequency")));
  node = node->GetNext();
  node->AddAttribute(_T ("Value"), wxString::Format(_T ("%0d"), m_iUpdateCheckFrequency));

  node->SetNext(new wxXmlNode(NULL, wxXML_ELEMENT_NODE, _T ("LastCheck")));
  node = node->GetNext();
  node->AddAttribute(_T ("Value"), wxString::Format(_T ("%0ld"), m_tLastUpdateCheck));
}

void SettingsManager::ReadDragNGoSettings(wxXmlNode* container)
{
  wxXmlNode* node = container->GetChildren();
  wxString nodName, sValue;
  long l;
  VirtualModifierManager modManager;

  // Helper to safely get node value (supports both attribute and text content formats)
  auto getNodeValue = [](wxXmlNode* n) -> wxString {
    if (n->GetAttributes()) return n->GetAttributes()->GetValue();
    return n->GetNodeContent();
  };

  while (node) {
    nodName = node->GetName();
    if (nodName == _T ("Enabled")) {
      wxString val = getNodeValue(node);
      m_bDNG_Enabled = (val == _T ("True") || val == _T ("1"));
    }
    else if (nodName == _T ("TimerFrequency")) {
      if (getNodeValue(node).ToLong(&l))
        m_iDNG_timer = clampValue<int>(l, 10, 1000);
    }
    else if (nodName == _T ("DetectionRadius")) {
      if (getNodeValue(node).ToLong(&l))
        m_iDNG_Radius = clampValue<int>(l, 10, 500);
    }
    else if (nodName == _T ("ZoneColors")) {
      if (node->GetAttribute(_T ("Background"), &sValue))
        m_cDNG_BgColor.Set(sValue);
      if (node->GetAttribute(_T ("Foreground"), &sValue))
        m_cDNG_FgColor.Set(sValue);
    }
    else if (nodName == _T ("Transparency") || nodName == _T ("Opacity")) {
      if (getNodeValue(node).ToLong(&l))
        m_iDNG_Transparency = clampValue<int>(l, 0, 255);
    }
    else if (nodName == _T ("Modifiers")) {
      if (node->GetAttribute(_T ("Modifier1"), &sValue))
        m_modDNG1 = modManager.GetValueFromString(sValue);
      if (node->GetAttribute(_T ("Modifier2"), &sValue))
        m_modDNG2 = modManager.GetValueFromString(sValue);
    }
    // Handle Settings.xml format
    else if (nodName == _T ("BackgroundColor")) {
      if (getNodeValue(node).ToLong(&l))
        m_cDNG_BgColor.Set(static_cast<unsigned long>(l));
    }
    else if (nodName == _T ("TextColor")) {
      if (getNodeValue(node).ToLong(&l))
        m_cDNG_FgColor.Set(static_cast<unsigned long>(l));
    }
    else if (nodName == _T ("Modifier1")) {
      if (getNodeValue(node).ToLong(&l))
        m_modDNG1 = static_cast<int>(l);
    }
    else if (nodName == _T ("Modifier2")) {
      if (getNodeValue(node).ToLong(&l))
        m_modDNG2 = static_cast<int>(l);
    }

    node = node->GetNext();
  }
}

void SettingsManager::SaveDragNGoSettings(wxXmlNode* container)
{
  wxXmlNode* node;
  wxString strMod;
  VirtualModifierManager modManager;

  container->AddChild(node = new wxXmlNode(NULL, wxXML_ELEMENT_NODE, _T ("Enabled")));
  node->AddAttribute(_T ("Value"), m_bDNG_Enabled ? _T ("True") : _T ("False"));

  node->SetNext(new wxXmlNode(NULL, wxXML_ELEMENT_NODE, _T ("TimerFrequency")));
  node = node->GetNext();
  node->AddAttribute(_T ("Value"), wxString::Format(_T ("%0d"), m_iDNG_timer));

  node->SetNext(new wxXmlNode(NULL, wxXML_ELEMENT_NODE, _T ("DetectionRadius")));
  node = node->GetNext();
  node->AddAttribute(_T ("Value"), wxString::Format(_T ("%0d"), m_iDNG_Radius));

  node->SetNext(new wxXmlNode(NULL, wxXML_ELEMENT_NODE, _T ("ZoneColors")));
  node = node->GetNext();
  node->AddAttribute(_T ("Background"), m_cDNG_BgColor.GetAsString(wxC2S_HTML_SYNTAX));
  node->AddAttribute(_T ("Foreground"), m_cDNG_FgColor.GetAsString(wxC2S_HTML_SYNTAX));

  node->SetNext(new wxXmlNode(NULL, wxXML_ELEMENT_NODE, _T ("Transparency")));
  node = node->GetNext();
  node->AddAttribute(_T ("Value"), wxString::Format(_T ("%0d"), m_iDNG_Transparency));

  node->SetNext(new wxXmlNode(NULL, wxXML_ELEMENT_NODE, _T ("Modifiers")));
  node = node->GetNext();

  strMod = modManager.GetStringFromValue(m_modDNG1);
  node->AddAttribute(_T ("Modifier1"), strMod);

  strMod = modManager.GetStringFromValue(m_modDNG2);
  node->AddAttribute(_T ("Modifier2"), strMod);
}

void SettingsManager::ReadMiscSettings(wxXmlNode* container)
{
  wxXmlNode* node = container->GetChildren();
  wxString nodName, sValue;
  long l;

  // Helper to safely get node value (supports both attribute and text content formats)
  auto getNodeValue = [](wxXmlNode* n) -> wxString {
    if (n->GetAttributes()) return n->GetAttributes()->GetValue();
    return n->GetNodeContent();
  };

  while (node) {
    nodName = node->GetName();
    wxString val = getNodeValue(node);

    if (nodName == _T ("MouseFollowWindow")) {
      m_bMouseFollowWnd = (val == _T ("True") || val == _T ("1"));
    }
    else if (nodName == _T ("MouseFollowWindowOnlyWhenIn") || nodName == _T ("MouseFollowOnlyIfOver")) {
      m_bMouseFollowOnlyWhenIn = (val == _T ("True") || val == _T ("1"));
    }
    else if (nodName == _T ("MinMaxCycle")) {
      m_bMinMaxCycle = (val == _T ("True") || val == _T ("1"));
    }
    // Handle Settings.xml format
    else if (nodName == _T ("ShowHKWarnings")) {
      m_bShowHKWarnings = (val == _T ("1"));
    }
    else if (nodName == _T ("AWT_Enabled")) {
      // Active Window Tools setting - store if we have a member for it
    }
    else if (nodName == _T ("AWT_BringToTop")) {
      // Active Window Tools setting
    }
    else if (nodName == _T ("AWT_Delay")) {
      // Active Window Tools delay
    }

    node = node->GetNext();
  }
}

void SettingsManager::SaveMiscSettings(wxXmlNode* container)
{
  wxXmlNode* node;

  container->AddChild(node = new wxXmlNode(NULL, wxXML_ELEMENT_NODE, _T ("MouseFollowWindow")));
  node->AddAttribute(_T ("Value"), m_bMouseFollowWnd ? _T ("True") : _T ("False"));

  node->SetNext(new wxXmlNode(
      NULL,
      wxXML_ELEMENT_NODE,
      _T ("MouseFollowWindowOnlyWhenIn"),
      wxEmptyString,
      new wxXmlAttribute(_T ("Value"), m_bMouseFollowOnlyWhenIn ? _T ("True") : _T ("False"))));
  node = node->GetNext();

  node->SetNext(
      new wxXmlNode(NULL,
                    wxXML_ELEMENT_NODE,
                    _T ("MinMaxCycle"),
                    wxEmptyString,
                    new wxXmlAttribute(_T ("Value"), m_bMinMaxCycle ? _T ("True") : _T ("False"))));
}
