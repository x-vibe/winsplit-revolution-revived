// Prevent winsock.h from being included (use winsock2 via wxWidgets)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#if (!defined(WX_PRECOMP))
#  define WX_PRECOMP
#endif

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#  include <wx/wx.h>
#endif
