#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <winuser.h>
#include <shlwapi.h>  // For PathRemoveFileSpecW

#pragma comment(lib, "shlwapi.lib")

#include "dwm_utils.h"
#include "frame_hook.h"
#include "hook.h"
#include "main.h"

using namespace std;

FrameHook::FrameHook(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos,
                     const wxSize& size, long style)
    : wxFrame(parent, id, title, pos, size, style)
    , p_panel(NULL)
    , p_stcTxtInfo(NULL)
    , m_timer()
    , m_vec_solution()
    , WSM_STARTMOVING(0)
    , WSM_STOPMOVING(0)
    , WSM_MOLETTE(0)
    , m_options(SettingsManager::Get())
    , m_iTransparency(45)
    , m_is_near(false)
    , m_isInstalled(false)
    , m_rectPrevious(-1, 0, 0, 0)
    , m_wheelpos(0)
    , m_wheelposPrevious(-1)
{
  // To be sure that nothing appears
  SetTransparent(0);

  CreateControls();
  CreateConnection();

  SetHook();
}

FrameHook::~FrameHook()
{
  StopAllHook();

  // Security: Get DLL handle with full path verification to prevent DLL hijacking
  wchar_t exePath[MAX_PATH] = {0};
  wchar_t expectedDllPath[MAX_PATH] = {0};

  if (GetModuleFileNameW(NULL, exePath, MAX_PATH)) {
    // Get directory of EXE
    PathRemoveFileSpecW(exePath);
    swprintf_s(expectedDllPath, MAX_PATH, L"%s\\winsplithook.dll", exePath);

    // Only unload if loaded from our expected directory
    HMODULE module_dll = GetModuleHandleW(L"winsplithook.dll");
    if (module_dll) {
      wchar_t loadedPath[MAX_PATH] = {0};
      if (GetModuleFileNameW(module_dll, loadedPath, MAX_PATH)) {
        // Verify the loaded DLL is from our application directory
        if (_wcsicmp(loadedPath, expectedDllPath) == 0) {
          FreeLibrary(module_dll);
        }
        // If paths don't match, don't unload - it may be a hijacked DLL
      }
    }
  }

  p_panel->Show(false);
  p_panel->Destroy();
}

void FrameHook::SetHook()
{
  // Security: Set safe DLL search order before any DLL operations
  // This prevents DLL hijacking from current working directory
  SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_APPLICATION_DIR | LOAD_LIBRARY_SEARCH_SYSTEM32);

  if (!(m_isInstalled = InstallAllHook((HWND)GetHandle())))
    wxMessageBox(_("Impossible to install hooks"));
}

void FrameHook::UnSetHook()
{
  if (!StopAllHook()) {
    wxMessageBox(_("Next WinSplit Revolution run in this loging session will probably cause Drag "
                   "n'go crash\n"
                   "Restart your computer may solve this problem"),
                 _("Impossible to uninstall some hooks"),
                 wxICON_ERROR);
  }
  else {
    m_isInstalled = false;
  }
}

void FrameHook::CreateControls()
{
  p_panel = new wxFrame(NULL,
                        wxID_ANY,
                        wxEmptyString,
                        wxDefaultPosition,
                        wxSize(0, 0),
                        wxSTAY_ON_TOP | wxFRAME_NO_TASKBAR);
  p_panel->SetBackgroundColour(m_options.getDnGZoneBgColor());
  m_iTransparency = m_options.getDngZoneTransparency();
  p_panel->SetTransparent((255 * m_iTransparency) / 100);

  wxBoxSizer* vsizer = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer* hsizer = new wxBoxSizer(wxVERTICAL);

  p_stcTxtInfo = new wxStaticText(
      p_panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(250, 200), wxALIGN_CENTRE);
  p_stcTxtInfo->SetForegroundColour(m_options.getDnGZoneFgColor());
  p_stcTxtInfo->SetFont(wxFont(24, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));

  hsizer->AddStretchSpacer();
  hsizer->Add(p_stcTxtInfo, 0, wxALIGN_CENTER | wxALL, 0);
  hsizer->AddStretchSpacer();

  vsizer->AddStretchSpacer();
  vsizer->Add(hsizer, 0, wxALIGN_CENTER | wxALL, 0);
  vsizer->AddStretchSpacer();

  p_panel->SetSizer(vsizer);
  p_panel->Disable();
}

void FrameHook::CreateConnection()
{
  WSM_STARTMOVING = RegisterWindowMessage(_T ("WinSplitMessage_StartMoving"));
  WSM_STOPMOVING = RegisterWindowMessage(_T ("WinSplitMessage_StopMoving"));
  WSM_MOLETTE = RegisterWindowMessage(_T ("WinSplitMessage_Wheel"));

  m_timer.SetOwner(this, wxID_ANY);
  Connect(wxEVT_TIMER, wxTimerEventHandler(FrameHook::OnTimer), NULL, this);

  WSM_TASKBAR_CREATED = RegisterWindowMessage(_T ("TaskbarCreated"));
}

void FrameHook::OnTimer(wxTimerEvent& event)
{
  if (IsDown()) {
    wxString message;

    m_is_near = LayoutManager::GetInstance()->GetNearestFromCursor(m_vec_solution);

    if (m_is_near && !m_vec_solution.empty()) {
      if ((m_vec_solution[0] != m_rectPrevious) || (m_wheelpos != m_wheelposPrevious)) {
        // Security: Safe index calculation (m_wheelpos is already normalized in WSM_MOLETTE handler)
        size_t safeIndex = static_cast<size_t>(m_wheelpos) % m_vec_solution.size();
        p_panel->SetSize(m_vec_solution[safeIndex]);

        p_panel->Show(true);
        m_rectPrevious = m_vec_solution[0];
        m_wheelposPrevious = m_wheelpos;

        if (m_vec_solution.size() > 1) {
          message.Clear();
          message.Printf(_T ("%d\n"), (int)(m_vec_solution.size()));
          message << _("Possibilities scroll to switch");

          p_stcTxtInfo->SetLabel(message);
          p_panel->GetSizer()->Layout();
        }
        else {
          p_stcTxtInfo->SetLabel(wxEmptyString);
        }
      }
    }
    else {
      p_panel->Show(false);
      m_rectPrevious.x = -1;
      m_wheelpos = 0;
      m_wheelposPrevious = 0;
    }
    //}
  }
  else {
    p_panel->Show(false);
  }

  if (!(GetKeyState(VK_LBUTTON) & 0x0100)) {
    if (m_timer.IsRunning())
      m_timer.Stop();

    p_panel->Show(false);
    m_rectPrevious.x = -1;
  }

  event.Skip(false);
}

WXLRESULT FrameHook::MSWWindowProc(WXUINT nMsg, WXWPARAM wparam, WXLPARAM lparam)
{
  // Security: Rate limiting for hook messages to prevent flooding attacks
  if (nMsg == WSM_STARTMOVING || nMsg == WSM_STOPMOVING || nMsg == WSM_MOLETTE) {
    static DWORD lastMsgTime = 0;
    static int msgCount = 0;
    DWORD now = GetTickCount();

    if (now - lastMsgTime < 100) {
      msgCount++;
      if (msgCount > 50) {  // Max 50 messages per 100ms
        return 0;  // Silently ignore flood
      }
    } else {
      lastMsgTime = now;
      msgCount = 0;
    }
  }

  if (nMsg == WSM_STARTMOVING) {
    if (m_options.getDnGZoneBgColor() != p_panel->GetBackgroundColour())
      p_panel->SetBackgroundColour(m_options.getDnGZoneBgColor());
    if (m_options.getDnGZoneFgColor() != p_panel->GetForegroundColour())
      p_stcTxtInfo->SetForegroundColour(m_options.getDnGZoneFgColor());
    if (m_iTransparency != m_options.getDngZoneTransparency()) {
      m_iTransparency = m_options.getDngZoneTransparency();
      p_panel->SetTransparent((255 * m_iTransparency) / 100);
    }

    if (!m_timer.IsRunning()) {
      m_wheelpos = 0;
      m_wheelposPrevious = 0;
      m_rectPrevious.x = -1;

      m_timer.Start(100);
    }

    return 0;
  }
  else if (nMsg == WSM_STOPMOVING) {
    if (m_timer.IsRunning())
      m_timer.Stop();

    p_panel->Show(false);

    MoveWindowToDestination();

    return 0;
  }
  else if (nMsg == WSM_MOLETTE) {
    if (m_timer.IsRunning() && !m_vec_solution.empty()) {
      m_wheelposPrevious = m_wheelpos;
      if (int(wparam) > 0)
        ++m_wheelpos;
      else
        --m_wheelpos;

      // Security: Safe modulo that handles negative values correctly
      // This prevents integer overflow and array out-of-bounds access
      int size = static_cast<int>(m_vec_solution.size());
      m_wheelpos = ((m_wheelpos % size) + size) % size;
    }
    return 0;
  }
  else if (nMsg == WSM_TASKBAR_CREATED) {
    wxGetApp().ShowTrayIcon();
  }

  return wxFrame::MSWWindowProc(nMsg, wparam, lparam);
}

void FrameHook::MoveWindowToDestination()
{
  if (IsDown()) {
    p_panel->Show(false);

    if (m_is_near && !m_vec_solution.empty()) {
      HWND hwnd = GetForegroundWindow();
      // Security: Safe index calculation
      size_t safeIndex = static_cast<size_t>(m_wheelpos) % m_vec_solution.size();
      wxRect rect_dest = m_vec_solution[safeIndex];

      // Adjust for invisible frame borders (Windows 10/11)
      wxRect adjusted = DwmUtils::AdjustForInvisibleFrame(hwnd, rect_dest);

      SetWindowPos(hwnd,
                   HWND_TOP,
                   adjusted.x,
                   adjusted.y,
                   adjusted.width,
                   adjusted.height,
                   SWP_SHOWWINDOW);
      m_rectPrevious.x = -1;
    }
  }
}

bool FrameHook::IsDown()
{
  if ((GetKeyState(ModifierToVK(m_options.getDnGMod1())) & 0x0100) ||
      (m_options.getDnGMod1() == 0)) {
    if ((GetKeyState(ModifierToVK(m_options.getDnGMod2())) & 0x0100) ||
        (m_options.getDnGMod2() == 0)) {
      return true;
    }
  }
  return false;
}

unsigned int FrameHook::ModifierToVK(const unsigned int& mod)
{
  switch (mod) {
    case MOD_CONTROL:
      return VK_CONTROL;
    case MOD_ALT:
      return VK_MENU;
    case MOD_SHIFT:
      return VK_SHIFT;
    case MOD_WIN:
      return VK_LWIN;
    case 10:
      return VK_MBUTTON;
    case 11:
      return VK_RBUTTON;
    default:
      return 0;
  }

  return 0;
}
