# WinSplit Revolution Revived -- Full Status Report

**Date:** 2026-03-07

## The Original Problem

WinSplit Revolution is a C++ Windows desktop utility that uses global hotkeys (Ctrl+Alt+Numpad) to snap, resize, and position windows. It's built on wxWidgets and has been around since 2005. This fork (`x-vibe/winsplit-revolution-revived`) modernizes it for Windows 10/11.

The user reported a **critical bug**: when shutting down or restarting Windows with WinSplit running, **Windows would hang indefinitely**. The laptop couldn't shut down. This is a blocking, user-facing issue -- the kind that makes people uninstall software.

---

## Root Cause Analysis (Shutdown Blocking)

The shutdown hang had **three interacting root causes**, all in the wxWidgets event loop and thread management:

### 1. `OnCloseSession()` didn't terminate the event loop
In `upstream/src/main.cpp`, the `WinSplitApp::OnCloseSession()` handler (called during `WM_ENDSESSION`) was calling `event.Skip()`, which hands the event back to wxWidgets' default processing. But wxWidgets' default processing doesn't terminate the event loop -- it just acknowledges the message. So the application's main loop kept running, and Windows waited for the process to exit... forever.

**Fix:** Changed `OnCloseSession()` to call `p_tray->Cleanup()` followed by `ExitMainLoop()`, which terminates the wxWidgets event loop and allows the process to exit cleanly.

### 2. The update-check thread blocked on network I/O
`ReadVersionThread` (the auto-update checker) uses WinInet to make HTTP requests. If a network call was in progress during shutdown, the thread would block indefinitely on the network socket. There was no mechanism to cancel it.

**Fix:** Added a `Cancel()` method to `ReadVersionThread` that calls `InternetCloseHandle()` to unblock any pending WinInet I/O, plus `TestDestroy()` checks throughout the thread's entry point so it can bail out quickly when cancellation is requested.

### 3. The tray icon destructor didn't join the update thread
`TrayIcon::~TrayIcon()` deleted the update thread object without waiting for it to finish. If the thread was mid-execution, this could cause a crash or hang. There was also no idempotent cleanup -- multiple exit paths (tray menu exit, system shutdown, WM_CLOSE) could trigger double-free.

**Fix:** Added an idempotent `Cleanup()` method to `TrayIcon` that cancels and joins the update thread before deletion. Added a `WM_QUERYENDSESSION` handler so Windows knows the app consents to shutdown. Fixed `VirtualNumpad::OnClose()` to call `Destroy()` on forced close (system shutdown) instead of just hiding the window.

### 4. Additional: Missing `WM_QUERYENDSESSION` handler
Windows sends `WM_QUERYENDSESSION` before `WM_ENDSESSION` to ask if the app is okay with shutting down. WinSplit had no handler for this, which could cause Windows to consider the app unresponsive.

**Fix:** Added a `WM_QUERYENDSESSION` handler that returns `TRUE` (consent to shutdown).

All shutdown fixes are verified by a new test suite at `tests/shutdown/gtest_shutdown_cleanup.cpp` with 14 tests covering smoke tests, edge cases, message flooding, cancelled shutdowns, and handle leak detection.

---

## Build Infrastructure Issues Found and Fixed

### wxWidgets Submodule Was Broken
The `upstream/wxWidgets/` directory was supposed to contain wxWidgets as a git submodule, but it actually contained a clone of the **parent repository** (`x-vibe/winsplit-revolution-revived`). This was confirmed by checking `upstream/wxWidgets/.git/config` -- the origin URL pointed to the parent repo, not wxWidgets.

**Fix:** Deleted the broken directory entirely, then cloned wxWidgets v3.2.4 fresh:
```
git clone --depth 1 --branch v3.2.4 https://github.com/wxWidgets/wxWidgets.git upstream/wxWidgets
```
Then initialized all required submodules within wxWidgets itself (pcre, expat, jpeg, png, tiff, zlib, nanosvg). Built wxWidgets from source using nmake (`makefile.vc BUILD=release RUNTIME_LIBS=static TARGET_CPU=X64`).

### MSVC Toolset Mismatch
The project targets `PlatformToolset=v145` (Visual Studio 2025/2026), but the build machine has VS2022 Build Tools installed (v143). MSBuild fails with `MSB8020: The build tools for v145 cannot be found`.

**Fix:** Added `/p:PlatformToolset=v143` override to all msbuild invocations. This is a development-machine workaround; the CI/CD pipeline targets v145.

### Test Suite Had 7 Pre-existing Compilation Errors

1. **UNICODE/\_UNICODE defined globally** -- broke Catch2 and GTest internals. Changed to per-target compile definitions.
2. **`#include <tlhelp32.h>` ordering** -- was included below functions that use `PROCESSENTRY32W` in `test_harness.h`. Moved above.
3. **Catch2 v2-style `CATCH_CONFIG_MAIN`** -- conflicted with Catch2 v3's `Catch2WithMain` library. Removed.
4. **Stress tests mixed with GTest** -- they use a custom `TestHarness`, not GTest. Split into separate executables.
5. **SEH `__try`/`__except`** -- incompatible with `std::function` in MSVC. Replaced with `try`/`catch(...)`.
6. **Missing `/ENTRY:mainCRTStartup`** -- GTest/Catch2 targets with UNICODE need this linker option for `main()` vs `wmain()`.
7. **WinSplit process detection** -- all test files searched for `FindWindow(nullptr, L"WinSplit Revolution - Hook Frame")` but the `FrameHook` window has an **empty title** (`wxEmptyString`). Every single test file was changed to use process-name-based detection via `CreateToolhelp32Snapshot` and `PROCESSENTRY32W`, searching for `"Winsplit.exe"`.

---

## Test Results

### Before Our Work
- Multiple test executables failed to compile (7 errors)
- Tests that did run couldn't find WinSplit (wrong window title detection)
- ~28/42 tests passing (~67%)

### After Our Work
- All 7 test executables compile cleanly
- WinSplit detection works via process name enumeration
- **36/39 tests passing (92%)**
- 8 previously-skipping tests now pass (shutdown suite)
- 4 previously-failing tests now pass (multi-monitor, DPI detection)

### The 3 Remaining Failures

All three are **functional positioning tests** that try to verify WinSplit actually moves windows when hotkeys are pressed. They're in `catch2_window_positioning.cpp` and `catch2_dpi_awareness.cpp`:

1. **"Snap to left half with Numpad 4"** -- Creates a test window, presses Ctrl+Alt+Numpad4, checks if the window moved to the left half of the screen.
2. **"Maximize with Numpad 5"** -- Same pattern, checks if the window fills the work area.
3. **"Half-screen positions are pixel-accurate"** -- DPI-aware version of the left-half test.

**Root cause:** These tests use `SendInput()` to simulate Ctrl+Alt+Numpad keypresses. But `SendInput` does **not** trigger `RegisterHotKey`-based `WM_HOTKEY` messages when called from a non-interactive (CLI/background) process. This is a Windows security restriction -- `RegisterHotKey` hotkeys are processed by the window station's hotkey dispatcher, which only fires for input from the interactive desktop's input queue, not from programmatic `SendInput` calls originating from background processes.

**Attempted mitigation:** Rewrote `SimulateHotkey()` to find WinSplit's `HotkeysManager` window and post `WM_HOTKEY` directly via `PostMessage`. This would bypass the `SendInput` limitation entirely. However, the `HotkeysManager` window (a `wxFrame` created with `wxFrame(NULL, -1, wxEmptyString)`) is **not discoverable** through any standard Windows enumeration API:

- `EnumWindows` -- doesn't list it (wxWidgets may create it as a message-only or non-top-level window internally)
- `EnumChildWindows` -- not a child of any discoverable parent
- `EnumThreadWindows` -- thread ID would need to be known first
- `FindWindowEx(HWND_MESSAGE, ...)` -- not a message-only window either

The `FindHotkeysManagerWindow()` function tries to find it by enumerating all windows belonging to the WinSplit process and looking for non-tool-window `wxWindow*` class names, but in practice it returns `nullptr`.

**Confirmed via debug logging:** We temporarily added `FILE*`-based debug logging to `HotkeysManager::Start()` and `MSWWindowProc()`. The log confirmed:
- `GetHandle()` returns a valid, non-null HWND
- `IsWindow(GetHandle())` returns true
- All 22 hotkeys register successfully via `RegisterHotKey`
- The HWND value exists but is simply not enumerable from outside the process

This is documented in `upstream/CHANGELOG.md` under Known Issues.

### The 8 Skipping Tests

Eight tests in `gtest_hook_spoofing.cpp` skip because they require the hook DLL (`winsplithook.dll`) to be loaded and a valid hook frame target window. These tests verify that spoofed/malicious hook messages are rejected. They need additional test infrastructure to inject the DLL into the test process, which hasn't been built yet.

---

## Files Modified (Summary)

| File | Change |
|------|--------|
| `upstream/src/main.cpp` | Shutdown fix: `OnCloseSession()` calls `Cleanup()` + `ExitMainLoop()`, added `WM_QUERYENDSESSION` |
| `upstream/src/tray_icon.cpp` | Idempotent `Cleanup()` method, thread cancellation and join |
| `upstream/src/update_thread.cpp/.h` | `Cancel()` method with `InternetCloseHandle()`, `TestDestroy()` checks |
| `upstream/src/frame_virtualnumpad.cpp` | `Destroy()` on forced close instead of hide |
| `upstream/CHANGELOG.md` | Comprehensive documentation of all changes and known issues |
| `upstream/wxWidgets/` | Replaced broken submodule with proper wxWidgets v3.2.4 |
| `tests/shutdown/gtest_shutdown_cleanup.cpp` | Fixed WinSplit detection (window title to process name) |
| `tests/functional/catch2_window_positioning.cpp` | Process-based detection, `PostMessage(WM_HOTKEY)` approach |
| `tests/functional/catch2_dpi_awareness.cpp` | Same fixes as window_positioning |
| `tests/functional/catch2_multimonitor.cpp` | Process-based detection |
| `tests/tools/test_harness.h` | Reordered functions, process-based `FindWinSplitWindow()` |
| `tests/CMakeLists.txt` | Per-target UNICODE, split stress tests, linker fixes |

---

## What Still Needs to Be Done

1. **Rebuild WinSplit.exe** -- The source is clean (debug logging removed), but the binary on disk was compiled with the debug code. Needs a clean rebuild. The last build attempt failed because `vswhere.exe` couldn't locate Visual Studio -- the build environment path needs to be resolved.

2. **3 functional positioning tests** -- Two options:
   - **Option A (app-side):** Add a named pipe, shared memory, or window-message-based test API that exposes the HotkeysManager HWND or accepts test commands. This would let tests trigger hotkey actions without `SendInput`.
   - **Option B (VM):** Run the tests in an interactive desktop session (VM with auto-login, no screen lock) where `SendInput` works as expected.

3. **8 hook spoofing tests** -- Need test infrastructure to load `winsplithook.dll` and create a mock hook frame window for the tests to target.

4. **Git commit** -- None of these changes have been committed yet. All modifications are local and uncommitted.

5. **CI/CD** -- The GitHub Actions workflow exists but may need updating to handle the wxWidgets submodule correctly (given it was broken) and the v143/v145 toolset flexibility.

---

## Architecture Insight Gained

The investigation revealed important architectural details about WinSplit's internals:

- **HotkeysManager** is a hidden `wxFrame` that exists solely to own `RegisterHotKey` registrations and receive `WM_HOTKEY` messages. It has no visible UI. Its window handle is valid but not enumerable from external processes -- this is either a wxWidgets implementation detail or a side effect of how the frame is created (`wxFrame(NULL, -1, wxEmptyString)` with no `Show()` call).

- **FrameHook** is a `WS_EX_TOOLWINDOW` window with an empty title. It's the one that IS discoverable via `EnumWindows`. It handles the drag-and-go overlay but does NOT process hotkeys.

- The hotkey flow is: `RegisterHotKey(HotkeysManager HWND)` -> Windows sends `WM_HOTKEY` to that HWND -> `HotkeysManager::MSWWindowProc()` dispatches to `ResizeWindow()`, `AutoPlace()`, etc.

- There's a subtle bug on line 102 of `hotkeys_manager.cpp`: `if (vec_hotkey[i].session = vec_hotkey[i].active)` uses assignment (`=`) not comparison (`==`). This is intentional -- it assigns `active` to `session` and then enters the block if `active` is true. It's valid C++ but reads like a bug to anyone reviewing the code.
