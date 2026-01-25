#include <windows.h>

#include "dwm_utils.h"
#include "functions_special.h"
#include "multimonitor_move.h"
#include "settingsmanager.h"

#include <wx/msgdlg.h>

#include <fstream>
#include <math.h>
#include <vector>

using namespace std;

wxRect RectTowXRect(RECT rect)
{
  wxRect val_tmp;

  val_tmp.x = rect.left;
  val_tmp.y = rect.top;
  val_tmp.width = (rect.right - rect.left);
  val_tmp.height = (rect.bottom - rect.top);

  return val_tmp;
}

void MoveToScreen(HWND hwnd, wxRect current_screen, wxRect dest_screen)
{
  wxRect wnd_dest_wx;
  wxRect wnd_current_wx;

  double proportion_x;
  double proportion_y;
  bool flag_resizable;
  bool flag_topmost;
  bool maximized = false;
  WINDOWPLACEMENT placement;

  proportion_x = double(dest_screen.GetRight() - dest_screen.GetLeft()) /
                 (current_screen.GetRight() - current_screen.GetLeft());
  proportion_y = double(dest_screen.GetBottom() - dest_screen.GetTop()) /
                 (current_screen.GetBottom() - current_screen.GetTop());

  // Use DWM-compensated window rect for accurate positioning on Win10/11
  wnd_current_wx = DwmUtils::GetWindowRectCompensated(hwnd);

  wnd_dest_wx = wnd_current_wx;

  wnd_dest_wx.x = dest_screen.GetLeft() +
                  (int)round((wnd_current_wx.x - current_screen.GetLeft()) * proportion_x);
  wnd_dest_wx.y = dest_screen.GetTop() +
                  (int)round((wnd_current_wx.y - current_screen.GetTop()) * proportion_y);

  wnd_dest_wx.width = int(round(wnd_current_wx.width * proportion_x));
  wnd_dest_wx.height = int(round(wnd_current_wx.height * proportion_y));

  if (!dest_screen.Contains(wnd_dest_wx)) {
    wnd_dest_wx.x = dest_screen.x;
    wnd_dest_wx.y = dest_screen.y;
  }

  flag_resizable = ((GetWindowLong(hwnd, GWL_STYLE) & WS_SIZEBOX) != 0);
  flag_topmost = ((GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_TOPMOST) != 0);

  GetWindowPlacement(hwnd, &placement);
  if (placement.showCmd == SW_SHOWMAXIMIZED) {
    placement.showCmd = SW_RESTORE;
    SetWindowPlacement(hwnd, &placement);
    maximized = true;
  }

  // Adjust for invisible frame borders before positioning
  wxRect adjusted = DwmUtils::AdjustForInvisibleFrame(hwnd, wnd_dest_wx);

  bool bMoveMouse = SettingsManager::Get().getMouseFollowWindow();
  if (bMoveMouse)
    StoreOrSetMousePosition(true, hwnd);
  SetWindowPos(hwnd,
               flag_topmost ? HWND_TOPMOST : HWND_TOP,
               adjusted.x,
               adjusted.y,
               adjusted.width,
               adjusted.height,
               flag_resizable ? SWP_SHOWWINDOW : SWP_NOSIZE);
  if (bMoveMouse)
    StoreOrSetMousePosition(false, hwnd);

  if (maximized)
    ShowWindow(hwnd, SW_SHOWMAXIMIZED);
}

bool CALLBACK EnumCallBack(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
  RECT rect_sreen;
  vector<wxRect>* p_vec_screen = ((vector<wxRect>*)dwData);

  MONITORINFO m_monitor_info;
  m_monitor_info.cbSize = sizeof(MONITORINFO);
  GetMonitorInfo(hMonitor, &m_monitor_info);

  p_vec_screen->push_back(RectTowXRect(m_monitor_info.rcWork));

  return true;
}

int GetCurrentScreen(const vector<wxRect>& vec_screen, HWND hwnd_window)
{
  int i;
  int nb_screen;

  MONITORINFO m_monitor_info;
  HMONITOR hMonitor;
  wxRect current_screen;

  hMonitor = MonitorFromWindow(hwnd_window, MONITOR_DEFAULTTONEAREST);

  m_monitor_info.cbSize = sizeof(MONITORINFO);
  GetMonitorInfo(hMonitor, &m_monitor_info);

  current_screen = RectTowXRect(m_monitor_info.rcWork);

  nb_screen = GetSystemMetrics(SM_CMONITORS);
  for (i = 0; i < nb_screen; ++i) {
    if (current_screen == vec_screen[i])
      return i;
  }

  return -1;
}

double DistanceCentreToMiddleEdge(DIRECTION sens, wxRect screen, wxRect window)
{
  double distance;
  double d_x, d_y;
  int win_centre_x, win_centre_y;
  int screen_edge_x, screen_edge_y;

  win_centre_x = (window.GetRight() + window.GetLeft()) / 2;
  win_centre_y = (window.GetBottom() + window.GetTop()) / 2;

  if (sens == RIGHT_SCREEN) {
    screen_edge_x = screen.GetLeft();
  }
  else //(sens == LEFT_SCREEN) Future implementation of Up and down
  {
    screen_edge_x = screen.GetRight();
  }

  screen_edge_y = (screen.GetBottom() - screen.GetTop()) / 2;

  d_x = fabs(win_centre_x - screen_edge_x);
  d_y = fabs(win_centre_y - screen_edge_y);

  distance = sqrt(d_x * d_x + d_y * d_y);

  return distance;
}

void MoveWindowToDirection(HWND hwnd, DIRECTION sens)
{
  int i, current;
  int i_tmp;
  int nb_monitor;
  double distance, distance_tmp = 0;
  vector<wxRect> vec_screen;
  RECT fenetre_tmp;
  wxRect fenetre;

  nb_monitor = GetSystemMetrics(SM_CMONITORS);
  EnumDisplayMonitors(NULL, NULL, (MONITORENUMPROC)EnumCallBack, (LPARAM)&vec_screen);

  if (nb_monitor != int(vec_screen.size())) {
    wxMessageBox(_("Problem to enumerate all screens"), _("Error"));
    return;
  }

  current = GetCurrentScreen(vec_screen, hwnd);
  if (current == -1) {
    wxMessageBox(_("Problem to detect current screen"), _("Error"));
    return;
  }

  // Use DWM-compensated rect for distance calculation
  fenetre = DwmUtils::GetWindowRectCompensated(hwnd);
  i_tmp = current;

  for (i = 0; i < nb_monitor; ++i) {
    if (i == current)
      continue;

    if (sens == RIGHT_SCREEN) {
      if (vec_screen[i].GetLeft() < vec_screen[current].GetLeft())
        continue;

      distance = DistanceCentreToMiddleEdge(RIGHT_SCREEN, vec_screen[i], fenetre);
      if (i_tmp == current) {
        i_tmp = i;
        distance_tmp = distance;
        continue;
      }

      if (distance < distance_tmp)
        i_tmp = i;
    }
    else if (sens == LEFT_SCREEN) {
      if (vec_screen[i].GetRight() > vec_screen[current].GetRight())
        continue;

      distance = DistanceCentreToMiddleEdge(LEFT_SCREEN, vec_screen[i], fenetre);
      if (i_tmp == current) {
        i_tmp = i;
        distance_tmp = distance;
        continue;
      }

      if (distance < distance_tmp)
        i_tmp = i;
    }
  }

  if (i_tmp != current)
    MoveToScreen(hwnd, vec_screen[current], vec_screen[i_tmp]);
}
