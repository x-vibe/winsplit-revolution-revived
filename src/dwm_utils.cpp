#include "dwm_utils.h"

#include <dwmapi.h>
#include <shellscalingapi.h>
#include <VersionHelpers.h>

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "shcore.lib")

namespace DwmUtils {

// Cache for Windows version checks
static int s_cachedMajorVersion = -1;
static int s_cachedBuildNumber = -1;

static void EnsureVersionCached()
{
  if (s_cachedMajorVersion >= 0)
    return;

  // Use RtlGetVersion to get accurate version info
  // (GetVersionEx is deprecated and may lie due to manifests)
  typedef NTSTATUS(WINAPI * RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);
  HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
  if (hNtdll) {
    RtlGetVersionPtr rtlGetVersion = (RtlGetVersionPtr)GetProcAddress(hNtdll, "RtlGetVersion");
    if (rtlGetVersion) {
      RTL_OSVERSIONINFOW osvi = {0};
      osvi.dwOSVersionInfoSize = sizeof(osvi);
      if (rtlGetVersion(&osvi) == 0) {
        s_cachedMajorVersion = osvi.dwMajorVersion;
        s_cachedBuildNumber = osvi.dwBuildNumber;
        return;
      }
    }
  }

  // Fallback
  s_cachedMajorVersion = 6;
  s_cachedBuildNumber = 0;
}

bool IsVistaOrLater()
{
  EnsureVersionCached();
  return s_cachedMajorVersion >= 6;
}

bool IsWindows10OrLater()
{
  EnsureVersionCached();
  // Windows 10 is version 10.0
  return s_cachedMajorVersion >= 10;
}

bool IsWindows11OrLater()
{
  EnsureVersionCached();
  // Windows 11 is version 10.0 with build >= 22000
  return s_cachedMajorVersion >= 10 && s_cachedBuildNumber >= 22000;
}

bool GetInvisibleFrameBorders(HWND hwnd, LONG& left, LONG& top, LONG& right, LONG& bottom)
{
  left = top = right = bottom = 0;

  if (!IsVistaOrLater())
    return false;

  if (!hwnd || !IsWindow(hwnd))
    return false;

  RECT windowRect;
  if (!GetWindowRect(hwnd, &windowRect))
    return false;

  RECT extFrame;
  HRESULT hr = DwmGetWindowAttribute(hwnd, DWMWA_EXTENDED_FRAME_BOUNDS, &extFrame, sizeof(RECT));
  if (FAILED(hr))
    return false;

  // Calculate the invisible borders
  left = extFrame.left - windowRect.left;
  top = extFrame.top - windowRect.top;
  right = windowRect.right - extFrame.right;
  bottom = windowRect.bottom - extFrame.bottom;

  // Check if there actually are invisible borders
  return (left != 0 || top != 0 || right != 0 || bottom != 0);
}

wxRect GetWindowRectCompensated(HWND hwnd)
{
  wxRect result(0, 0, 0, 0);

  if (!hwnd || !IsWindow(hwnd))
    return result;

  if (IsVistaOrLater()) {
    // Try to get DWM extended frame bounds (visible area)
    RECT extFrame;
    HRESULT hr = DwmGetWindowAttribute(hwnd, DWMWA_EXTENDED_FRAME_BOUNDS, &extFrame, sizeof(RECT));
    if (SUCCEEDED(hr)) {
      result.x = extFrame.left;
      result.y = extFrame.top;
      result.width = extFrame.right - extFrame.left;
      result.height = extFrame.bottom - extFrame.top;
      return result;
    }
  }

  // Fallback to standard GetWindowRect
  RECT windowRect;
  if (GetWindowRect(hwnd, &windowRect)) {
    result.x = windowRect.left;
    result.y = windowRect.top;
    result.width = windowRect.right - windowRect.left;
    result.height = windowRect.bottom - windowRect.top;
  }

  return result;
}

wxRect AdjustForInvisibleFrame(HWND hwnd, const wxRect& targetRect)
{
  wxRect adjusted = targetRect;

  LONG left, top, right, bottom;
  if (GetInvisibleFrameBorders(hwnd, left, top, right, bottom)) {
    // Expand the target rect to account for invisible borders
    // so the visible portion ends up at the target location
    adjusted.x -= left;
    adjusted.y -= top;
    adjusted.width += left + right;
    adjusted.height += top + bottom;
  }

  return adjusted;
}

UINT GetDpiForHwnd(HWND hwnd)
{
  // Try GetDpiForWindow (Windows 10 1607+)
  typedef UINT(WINAPI * GetDpiForWindowPtr)(HWND);
  static GetDpiForWindowPtr pGetDpiForWindow = nullptr;
  static bool initialized = false;

  if (!initialized) {
    HMODULE hUser32 = GetModuleHandleW(L"user32.dll");
    if (hUser32) {
      pGetDpiForWindow = (GetDpiForWindowPtr)GetProcAddress(hUser32, "GetDpiForWindow");
    }
    initialized = true;
  }

  if (pGetDpiForWindow && hwnd) {
    UINT dpi = pGetDpiForWindow(hwnd);
    if (dpi > 0)
      return dpi;
  }

  // Fallback: get DPI from monitor
  if (hwnd) {
    HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    UINT dpiX = 96, dpiY = 96;
    if (SUCCEEDED(GetDpiForMonitor(hMonitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY))) {
      return dpiX;
    }
  }

  // Ultimate fallback: system DPI
  HDC hdc = GetDC(NULL);
  if (hdc) {
    int dpi = GetDeviceCaps(hdc, LOGPIXELSX);
    ReleaseDC(NULL, hdc);
    return dpi;
  }

  return 96; // Default DPI
}

UINT GetDpiForPoint(int x, int y)
{
  POINT pt = {x, y};
  HMONITOR hMonitor = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);

  UINT dpiX = 96, dpiY = 96;
  if (SUCCEEDED(GetDpiForMonitor(hMonitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY))) {
    return dpiX;
  }

  return 96;
}

wxRect ScaleRectForDpi(const wxRect& rect, UINT fromDpi, UINT toDpi)
{
  if (fromDpi == toDpi || fromDpi == 0 || toDpi == 0)
    return rect;

  double scale = static_cast<double>(toDpi) / static_cast<double>(fromDpi);

  wxRect scaled;
  scaled.x = static_cast<int>(rect.x * scale);
  scaled.y = static_cast<int>(rect.y * scale);
  scaled.width = static_cast<int>(rect.width * scale);
  scaled.height = static_cast<int>(rect.height * scale);

  return scaled;
}

void InitializeDpiAwareness()
{
  // Try SetProcessDpiAwarenessContext (Windows 10 1703+)
  typedef BOOL(WINAPI * SetProcessDpiAwarenessContextPtr)(DPI_AWARENESS_CONTEXT);
  HMODULE hUser32 = GetModuleHandleW(L"user32.dll");
  if (hUser32) {
    SetProcessDpiAwarenessContextPtr pSetContext =
        (SetProcessDpiAwarenessContextPtr)GetProcAddress(hUser32, "SetProcessDpiAwarenessContext");
    if (pSetContext) {
      if (pSetContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2))
        return;
    }
  }

  // Fallback: SetProcessDpiAwareness (Windows 8.1+)
  HMODULE hShcore = LoadLibraryW(L"shcore.dll");
  if (hShcore) {
    typedef HRESULT(WINAPI * SetProcessDpiAwarenessPtr)(PROCESS_DPI_AWARENESS);
    SetProcessDpiAwarenessPtr pSetAwareness =
        (SetProcessDpiAwarenessPtr)GetProcAddress(hShcore, "SetProcessDpiAwareness");
    if (pSetAwareness) {
      pSetAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
    }
  }
}

} // namespace DwmUtils
