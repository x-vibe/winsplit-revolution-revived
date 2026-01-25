#include <Windows.h>

#include <psapi.h>

#include "auto_placement.h"
#include "dialog_fusion.h"
#include "dwm_utils.h"
#include "functions_special.h"
#include "layout_manager.h"
#include "list_windows.h"
#include "multimonitor_move.h"

//=============================
// Resize window
//=============================
bool ResizeWindow(const int hotkey, bool fromKbd)
{
  HWND hwnd = ListWindows::ListWindow();
  bool flag_resizable = true;
  //((GetWindowLong(hwnd,GWL_STYLE)&WS_SIZEBOX)!=0);

  wxRect res = LayoutManager::GetInstance()->GetNext(hwnd, hotkey - 1);

  WINDOWPLACEMENT placement;
  GetWindowPlacement(hwnd, &placement);

  if (placement.showCmd == SW_SHOWMAXIMIZED) {
    placement.showCmd = SW_RESTORE;
    SetWindowPlacement(hwnd, &placement);
  }

  bool bMoveMouse = fromKbd & SettingsManager::Get().getMouseFollowWindow();
  if (bMoveMouse)
    StoreOrSetMousePosition(true, hwnd);

  // Adjust target rect for invisible frame borders (Windows 10/11)
  wxRect adjusted = DwmUtils::AdjustForInvisibleFrame(hwnd, res);

  SetWindowPos(hwnd,
               HWND_TOP,
               adjusted.x,
               adjusted.y,
               adjusted.width,
               adjusted.height,
               flag_resizable ? SWP_SHOWWINDOW : SWP_NOSIZE);

  if (bMoveMouse)
    StoreOrSetMousePosition(false, hwnd);

  return true;
}

void MoveToScreen(DIRECTION sens)
{
  HWND hwnd = GetForegroundWindow();
  if (!ListWindows::ValidateWindow(hwnd))
    return;

  MoveWindowToDirection(hwnd, sens);
}
