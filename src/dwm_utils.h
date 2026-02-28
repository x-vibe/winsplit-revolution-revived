#ifndef __DWM_UTILS_H__
#define __DWM_UTILS_H__

#include <Windows.h>
#include <wx/gdicmn.h>

namespace DwmUtils {

// Get window rectangle with DWM invisible frame compensation.
// On Windows 10/11, returns the visible bounds (excluding invisible frame).
// On older Windows or if DWM fails, returns standard GetWindowRect result.
wxRect GetWindowRectCompensated(HWND hwnd);

// Adjust a target position/size to account for invisible frame.
// Use this before calling SetWindowPos to ensure the visible window
// appears at the intended location.
wxRect AdjustForInvisibleFrame(HWND hwnd, const wxRect& targetRect);

// Get the invisible frame borders for a window.
// Returns true if the window has invisible borders, false otherwise.
// left, top, right, bottom contain the border sizes in pixels.
bool GetInvisibleFrameBorders(HWND hwnd, LONG& left, LONG& top, LONG& right, LONG& bottom);

// Get DPI for a specific window.
// Returns the DPI value (96 = 100%, 144 = 150%, 192 = 200%, etc.)
UINT GetDpiForHwnd(HWND hwnd);

// Get DPI for the monitor containing a point.
UINT GetDpiForPoint(int x, int y);

// Scale a rectangle from one DPI to another.
wxRect ScaleRectForDpi(const wxRect& rect, UINT fromDpi, UINT toDpi);

// Check Windows version
bool IsWindows10OrLater();
bool IsWindows11OrLater();
bool IsVistaOrLater();

// Initialize DPI awareness for the process.
// Should be called early in application startup.
void InitializeDpiAwareness();

} // namespace DwmUtils

#endif // __DWM_UTILS_H__
