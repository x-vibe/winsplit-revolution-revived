/**
 * Mock Window Tool
 * Creates test windows with specific properties for WinSplit testing
 *
 * Usage:
 *   mock_window.exe [options]
 *
 * Options:
 *   --count N      Create N windows (default: 1)
 *   --width W      Window width (default: 400)
 *   --height H     Window height (default: 300)
 *   --x X          Initial X position (default: 100)
 *   --y Y          Initial Y position (default: 100)
 *   --resizable    Make window resizable (default: yes)
 *   --no-resize    Make window non-resizable
 *   --topmost      Make window always on top
 *   --wait         Wait for keypress before closing
 *   --timeout MS   Close after MS milliseconds
 *   --title TEXT   Window title
 */

#include <windows.h>
#include <stdio.h>
#include <string>
#include <vector>

// Window class name
const wchar_t* CLASS_NAME = L"MockWindowClass";

// Global state
struct WindowInfo {
    HWND hwnd;
    int id;
    RECT initialRect;
    RECT currentRect;
};

std::vector<WindowInfo> g_windows;
bool g_running = true;
bool g_resizable = true;
bool g_topmost = false;
int g_timeout = 0;

// Window procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) {
            DestroyWindow(hwnd);
            return 0;
        }
        break;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // Draw window info
        RECT rc;
        GetClientRect(hwnd, &rc);

        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(0, 0, 0));

        wchar_t buf[256];
        RECT winRect;
        GetWindowRect(hwnd, &winRect);

        swprintf(buf, 256, L"Window Rect:\nL: %ld  T: %ld\nR: %ld  B: %ld\nW: %ld  H: %ld",
                 winRect.left, winRect.top,
                 winRect.right, winRect.bottom,
                 winRect.right - winRect.left, winRect.bottom - winRect.top);

        DrawText(hdc, buf, -1, &rc, DT_CENTER | DT_VCENTER);

        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_MOVE:
    case WM_SIZE:
        InvalidateRect(hwnd, NULL, TRUE);
        break;

    case WM_GETMINMAXINFO:
        if (!g_resizable) {
            LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
            RECT rc;
            GetWindowRect(hwnd, &rc);
            int w = rc.right - rc.left;
            int h = rc.bottom - rc.top;
            lpMMI->ptMinTrackSize.x = w;
            lpMMI->ptMinTrackSize.y = h;
            lpMMI->ptMaxTrackSize.x = w;
            lpMMI->ptMaxTrackSize.y = h;
        }
        break;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Create a mock window
HWND CreateMockWindow(int id, int x, int y, int width, int height, const wchar_t* title) {
    DWORD style = WS_OVERLAPPEDWINDOW;
    DWORD exStyle = 0;

    if (g_topmost) {
        exStyle |= WS_EX_TOPMOST;
    }

    wchar_t fullTitle[256];
    if (title && title[0]) {
        swprintf(fullTitle, 256, L"%s [%d]", title, id);
    } else {
        swprintf(fullTitle, 256, L"Mock Window %d", id);
    }

    HWND hwnd = CreateWindowEx(
        exStyle,
        CLASS_NAME,
        fullTitle,
        style,
        x, y, width, height,
        NULL, NULL,
        GetModuleHandle(NULL),
        NULL
    );

    if (hwnd) {
        WindowInfo info;
        info.hwnd = hwnd;
        info.id = id;
        GetWindowRect(hwnd, &info.initialRect);
        info.currentRect = info.initialRect;
        g_windows.push_back(info);

        ShowWindow(hwnd, SW_SHOW);
        UpdateWindow(hwnd);
    }

    return hwnd;
}

// Print usage
void PrintUsage() {
    printf("Mock Window Tool for WinSplit Testing\n\n");
    printf("Usage: mock_window.exe [options]\n\n");
    printf("Options:\n");
    printf("  --count N      Create N windows (default: 1)\n");
    printf("  --width W      Window width (default: 400)\n");
    printf("  --height H     Window height (default: 300)\n");
    printf("  --x X          Initial X position (default: 100)\n");
    printf("  --y Y          Initial Y position (default: 100)\n");
    printf("  --no-resize    Make window non-resizable\n");
    printf("  --topmost      Make window always on top\n");
    printf("  --wait         Wait for keypress before closing\n");
    printf("  --timeout MS   Close after MS milliseconds\n");
    printf("  --title TEXT   Window title\n");
    printf("  --help         Show this help\n");
    printf("\nPress ESC in any window to close it.\n");
}

// Parse command line
struct Options {
    int count;
    int width;
    int height;
    int x;
    int y;
    bool wait;
    int timeout;
    std::wstring title;
};

Options ParseCommandLine(int argc, wchar_t* argv[]) {
    Options opts = { 1, 400, 300, 100, 100, false, 0, L"" };

    for (int i = 1; i < argc; i++) {
        std::wstring arg = argv[i];

        if (arg == L"--help" || arg == L"-h") {
            PrintUsage();
            exit(0);
        } else if (arg == L"--count" && i + 1 < argc) {
            opts.count = _wtoi(argv[++i]);
        } else if (arg == L"--width" && i + 1 < argc) {
            opts.width = _wtoi(argv[++i]);
        } else if (arg == L"--height" && i + 1 < argc) {
            opts.height = _wtoi(argv[++i]);
        } else if (arg == L"--x" && i + 1 < argc) {
            opts.x = _wtoi(argv[++i]);
        } else if (arg == L"--y" && i + 1 < argc) {
            opts.y = _wtoi(argv[++i]);
        } else if (arg == L"--no-resize") {
            g_resizable = false;
        } else if (arg == L"--topmost") {
            g_topmost = true;
        } else if (arg == L"--wait") {
            opts.wait = true;
        } else if (arg == L"--timeout" && i + 1 < argc) {
            opts.timeout = _wtoi(argv[++i]);
        } else if (arg == L"--title" && i + 1 < argc) {
            opts.title = argv[++i];
        }
    }

    return opts;
}

// Timer callback
VOID CALLBACK TimeoutProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
    g_running = false;
    for (auto& wi : g_windows) {
        PostMessage(wi.hwnd, WM_CLOSE, 0, 0);
    }
}

int wmain(int argc, wchar_t* argv[]) {
    // Register window class
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    if (!RegisterClass(&wc)) {
        printf("Failed to register window class\n");
        return 1;
    }

    // Parse options
    Options opts = ParseCommandLine(argc, argv);

    // Create windows
    printf("Creating %d mock window(s)...\n", opts.count);

    for (int i = 0; i < opts.count; i++) {
        int offsetX = (i % 5) * 50;
        int offsetY = (i / 5) * 50;

        HWND hwnd = CreateMockWindow(
            i + 1,
            opts.x + offsetX,
            opts.y + offsetY,
            opts.width,
            opts.height,
            opts.title.c_str()
        );

        if (!hwnd) {
            printf("Failed to create window %d\n", i + 1);
        } else {
            RECT rc;
            GetWindowRect(hwnd, &rc);
            printf("Window %d: HWND=%p, Rect=(%ld,%ld,%ld,%ld)\n",
                   i + 1, hwnd, rc.left, rc.top, rc.right, rc.bottom);
        }
    }

    // Set timeout if specified
    if (opts.timeout > 0) {
        SetTimer(NULL, 0, opts.timeout, TimeoutProc);
    }

    // Message loop
    MSG msg = {};
    while (g_running && GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Report final positions
    printf("\nFinal window positions:\n");
    for (const auto& wi : g_windows) {
        if (IsWindow(wi.hwnd)) {
            RECT rc;
            GetWindowRect(wi.hwnd, &rc);
            printf("Window %d: (%ld,%ld) -> (%ld,%ld)\n",
                   wi.id,
                   wi.initialRect.left, wi.initialRect.top,
                   rc.left, rc.top);
        }
    }

    if (opts.wait) {
        printf("\nPress Enter to exit...\n");
        getchar();
    }

    return 0;
}

// Entry point for non-Unicode builds
int main(int argc, char* argv[]) {
    // Convert to wide strings
    std::vector<wchar_t*> wargv(argc);
    std::vector<std::wstring> wargs(argc);

    for (int i = 0; i < argc; i++) {
        int len = MultiByteToWideChar(CP_UTF8, 0, argv[i], -1, NULL, 0);
        wargs[i].resize(len);
        MultiByteToWideChar(CP_UTF8, 0, argv[i], -1, &wargs[i][0], len);
        wargv[i] = &wargs[i][0];
    }

    return wmain(argc, wargv.data());
}
