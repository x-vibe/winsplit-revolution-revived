# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

WinSplit Revolution is a Windows desktop utility (C++ with wxWidgets) for window positioning and resizing through hotkeys and a virtual numpad. This fork updates it for reliable Windows 11 support.

**Repository:** [x-vibe/winsplit-revolution-revived](https://github.com/x-vibe/winsplit-revolution-revived)

**Repository Structure:**

- `upstream/` - WinSplit Revolution source (originally from [Codeberg](https://codeberg.org/skullzy/winsplit-revolution))
- Root level - Project files, scripts, and documentation for this fork

## Credits

Original WinSplit Revolution Copyright (C) 2005-2009 by Raphael Lencrerot.
Team: Raphael Lencrerot, Xavier Perrissoud, Arturo Espinosa, Dan Smith.
Previous maintainer: [dozius](https://github.com/dozius/winsplit-revolution).

## Build Commands

> **Note (January 2026):** Version numbers and paths below were current as of January 2026. When setting up a new development environment, verify versions online for any updates.

### VS Code Build (Recommended)

The project includes `.vscode/` configuration for building directly from VS Code without the Visual Studio IDE.

**Prerequisites:**
- Visual Studio 2026 with "Desktop development with C++" workload
- VS Code with C/C++ extension (optional)

**To build:**
1. Open project folder in VS Code: `E:\VIBECODING\DEV\OTHER\winsplit-revolution-revamped`
2. Press `Ctrl+Shift+B` for default build (runs `build.cmd`)
3. Or use Terminal → Run Task for specific tasks:
   - `Build (Full)` - Default, builds everything including wxWidgets if needed
   - `Build WinSplit (Release x64)` - Quick release build
   - `Build WinSplit (Debug x64)` - Debug build for debugging
   - `Clean Solution` - Remove build artifacts
   - `Rebuild WinSplit (Release x64)` - Clean + build

The tasks use `vswhere.exe` to auto-detect Visual Studio installation.

### Command Line Build

Run from project root on Windows:
```cmd
build.cmd
```

Or manually from a Developer Command Prompt:

```cmd
cd upstream\wxWidgets\build\msw
nmake -f makefile.vc BUILD=release RUNTIME_LIBS=static TARGET_CPU=X64

cd ..\..\..\upstream
msbuild "Winsplit Revolution.sln" -property:Configuration=Release -property:Platform=x64
```

Output: `upstream/x64/Release/Winsplit.exe` and `winsplithook.dll`

## Architecture

### Two Build Targets
1. **Winsplit.exe** - Main application (wxWidgets GUI)
2. **winsplithook.dll** - Hook DLL for drag'n'go and window movement tracking

The Hook DLL must build first (dependency in solution).

### Core Components

| Component | Files | Purpose |
|-----------|-------|---------|
| WinSplitApp | `main.h/cpp` | wxApp entry, single instance check |
| HotkeysManager | `hotkeys_manager.h/cpp` | 22 global hotkeys, Windows message processing |
| LayoutManager | `layout_manager.h/cpp` | Grid layouts via ratio-based positioning |
| SettingsManager | `settingsmanager.h/cpp` | XML config persistence (singleton) |
| TrayIcon | `tray_icon.h/cpp` | System tray, spawns dialogs |
| VirtualNumpad | `frame_virtualnumpad.h/cpp` | Clickable 3x3 numpad UI |
| FrameHook | `frame_hook.h/cpp` | Drag'n'go window, hook DLL interface |
| Hook DLL | `hook_src/main.cpp` | WH_MOUSE and WH_CBT Windows hooks |

### Windows 10/11 Frame Fix

Located in `upstream/src/functions_resize.cpp` (lines 39-58):
- Uses `DwmGetWindowAttribute(DWMWA_EXTENDED_FRAME_BOUNDS)` to detect invisible border padding
- Adjusts `SetWindowPos()` target rectangle to compensate
- Only applies on Windows Vista+ (major version >= 6)

**Fixed in v10.1.0:**
- Windows 10/11 invisible frame handling (DWM frame compensation)
- Window positioning when moving between monitors
- Drag'n'Go preview matching actual placement

**Remaining Known Issues:**
- Left side of monitor may extend into taskbar (edge case)

### Key Windows APIs Used
- `GetForegroundWindow()`, `SetWindowPos()`, `GetWindowRect()`
- `DwmGetWindowAttribute()` - DWM compositor queries
- `SetWindowsHookEx()` - Global keyboard/mouse hooks
- `EnumDisplayMonitors()`, `MonitorFromWindow()` - Multi-monitor
- `SetLayeredWindowAttributes()` - Window transparency

### Hotkey System Flow
1. `HotkeysManager` reads XML config or loads defaults
2. Hotkeys registered via Windows `RegisterHotKey()`
3. `FrameHook` window processes `WM_HOTKEY` messages
4. Corresponding function called (resize, fusion, mosaic, etc.)

## Configuration

Settings stored in XML format in user's AppData directory. Supports:
- Portable mode (config in app directory)
- Classic mode (registry + AppData)
- Import/export for backup

## Dependencies

> **Note (January 2026):** Verify latest versions online before installing. SDK and toolset versions may have updates.

- **wxWidgets 3.x** - Embedded as git submodule in `upstream/wxWidgets/`
- **Windows 11 SDK 10.0.26100.0** (Win11 24H2 or newer)
- **MSVC v145 (VS2025/2026)** - Primary supported toolset
- **MSVC v143 (VS2022)** - Also compatible

## wxWidgets 3.x Compatibility Notes

The codebase has been updated from wxWidgets 2.x to wxWidgets 3.x. Key API changes:

| Old API (wxWidgets 2.x) | New API (wxWidgets 3.x) | Files Affected |
|-------------------------|-------------------------|----------------|
| `GetPropVal()` | `GetAttribute()` | settingsmanager.cpp |
| `AddProperty()` | `AddAttribute()` | settingsmanager.cpp |
| `SetTickFreq(freq, pos)` | `SetTickFreq(freq)` | dialog_options.cpp |
| `wxLOCALE_CONV_ENCODING` | Removed (use default) | settingsmanager.cpp |

### Header Conflict Prevention

All header files that include `<windows.h>` must use the `WIN32_LEAN_AND_MEAN` guard to prevent winsock.h/winsock2.h conflicts:

```cpp
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
```

This is defined in the project's preprocessor definitions and in individual headers.

---

## Developer Environment Notes

**Important:** The developer uses **VS Code 2026** as their IDE, not the full Visual Studio IDE.

- **IDE:** VS Code 2026 with C/C++ extension
- **Compiler:** Visual Studio 2026 (full installation, for `msbuild`, `nmake`, `cl.exe`)
- **Environment:** Primary development in WSL2 Linux, Windows builds via command line
- **Toolset:** v145 (VS2025/2026) primary, v143 (VS2022) also compatible
- **Windows SDK:** 10.0.26100.0

**VS Code configuration files:**
- `.vscode/tasks.json` - Build tasks (Ctrl+Shift+B)
- `.vscode/c_cpp_properties.json` - IntelliSense configuration
- `.vscode/launch.json` - Debug configurations

When providing build instructions:
- Prefer VS Code tasks or command-line builds (`msbuild`, `nmake`)
- Use `build.cmd` script which auto-detects Visual Studio installation
- Avoid instructions that require Visual Studio IDE interaction

## Code Style

Project includes `.clang-format` in upstream directory.

---

## Important: Changelog Maintenance

**ALWAYS update [upstream/CHANGELOG.md](upstream/CHANGELOG.md) when making changes.**

- Add a new `## [Unreleased]` section at the top for ongoing changes
- Follow [Keep a Changelog](https://keepachangelog.com/) format
- Categories: Added, Changed, Deprecated, Removed, Fixed, Security
- Move unreleased items to versioned section on release

**Latest Release:** [v10.2.0](https://github.com/x-vibe/winsplit-revolution-revived/releases/tag/v10.2.0) (2026-01-25)

---

## Project Status & Tasks

**Full task list: [TASKS.md](TASKS.md) (515 granular tasks)**

### Progress Summary

**Current Version:** v10.2.0 (Released 2026-01-25)

| Category | Done | Total |
|----------|------|-------|
| Setup & Build | 7 | 20 |
| Security Critical | 0 | 25 |
| Security High | 0 | 25 |
| Security Medium | 0 | 20 |
| Windows 11 Compat | 20 | 30 |
| Multi-Monitor | 5 | 20 |
| Code Quality | 5 | 25 |
| Testing | 23 | 35 |
| Documentation | 10 | 20 |
| CI/CD | 0 | 15 |
| Release | 5 | 15 |
| **Edge Case Testing** | **0** | **235** |
| Future Features | 0 | 10 |
| **TOTAL** | **75** | **515** |

### Edge Case Testing Categories (#281-515)

- Win11 Edge Cases: Virtual Desktops, Snap Layouts, WSL, ARM
- Accessibility: Screen readers, High Contrast, Magnifier
- Special Windows: UAC, UWP, Fullscreen, Gaming overlays
- Hardware/Display: HDR, Docking, Orientation changes
- Software Conflicts: Other WMs, Remote access, Security software
- System State: Sleep/wake, Updates, Power modes
- Input Methods: Touch, Pen, Voice
- Race Conditions: Rapid ops, Resource exhaustion
- CI/CD Environments: GitHub Actions, VMs, Self-hosted

### Test Commands

```cmd
cd tests && build_cmake.cmd && cd build && ctest
```

```cmd
cd tests/pentest && setup_pentest_tools.cmd && run_pentest.cmd
```

### Priority Security Fixes (TASKS.md #21-70)

| Task # | Issue | Risk |
|--------|-------|------|
| 21-35 | HTTP updates → HTTPS | CRITICAL |
| 36-45 | Hook message validation | CRITICAL |
| 46-55 | DLL load path hardening | HIGH |
| 56-65 | Minimum process permissions | HIGH |
| 66-70 | Input validation | MEDIUM |
