// structure auto application...
#ifndef __AUTOAPP_H__
#define __AUTOAPP_H__

#include <windows.h>

#include <wx/arrstr.h>
#include <wx/gdicmn.h>
#include <wx/string.h>

#include <vector>

struct WindowInfos {
  wxString m_strName;
  long m_wndStyle;
  wxRect m_rectxy;
  bool m_flagResize;

  WindowInfos() {}
  ~WindowInfos() {}

  bool operator==(const WindowInfos& rhs);
};

class AutoPlacementManager {
private:
  std::vector<WindowInfos> m_vecWnd;

public:
  AutoPlacementManager() {}
  ~AutoPlacementManager() {}

  WindowInfos GetWindowInfos(const wxString& name);
  void AddWindow(const HWND& hwnd, const wxString& name);
  wxArrayString GetArrayName();
  void DeleteApplication(const int& pos);
  bool IsEmpty();
  bool Exist(const wxString& name);
  bool LoadData();
  bool SaveData();
};

#endif //__AUTOAPP_H__
