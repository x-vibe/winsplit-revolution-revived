# WinSplit Revolution Revived

[![License: GPL-3.0](https://img.shields.io/badge/license-GPL--3.0-blue.svg)](upstream/LICENSE)
[![Platform: Windows 10/11](https://img.shields.io/badge/platform-Windows%2010%20%7C%2011-0078D4.svg)]()
[![Latest Release](https://img.shields.io/github/v/release/x-vibe/winsplit-revolution-revived)](https://github.com/x-vibe/winsplit-revolution-revived/releases/latest)
[![Architecture: x64](https://img.shields.io/badge/arch-x64-green.svg)]()

A lightweight, open-source window manager for Windows that tiles, snaps, and organizes windows using keyboard hotkeys and a virtual numpad.

---

## Background

WinSplit Revolution was originally developed by Raphael Lencrerot and team from 2005 to 2009, then abandoned. This fork modernizes the codebase for Windows 10 and 11 with high-DPI support, security hardening, and wxWidgets 3.x compatibility.

---

## Why WinSplit Revolution?

| Feature | WinSplit Revolution | PowerToys FancyZones | DisplayFusion | Divvy |
|---------|--------------------|-----------------------|---------------|-------|
| **Size** | ~500 KB, minimal RAM | ~200 MB (full suite) | ~50 MB | ~15 MB |
| **Portable** | Yes, no install needed | No, requires install | No, requires install | No |
| **Response time** | Instant | Slight delay | Noticeable lag | Slight delay |
| **Cost** | Free, forever (GPL v3) | Free | Paid ($30+) | Paid ($14) |
| **Telemetry** | None | Opt-out telemetry | Usage tracking | None |
| **Open source** | Yes | Yes | No | No |
| **Dependencies** | None (static build) | .NET Runtime | .NET Runtime | None |

---

## Features

**Window Positioning** -- 22 global hotkeys map the numpad to a 3x3 screen grid. Press a numpad key to snap the active window to that region. Press the same key again to cycle through layout variations (half, third, two-thirds).

**Drag'n'Go** -- Hold a modifier key, drag any window toward a screen edge, and release to snap it into position. A color-coded preview zone shows where the window will land. Zone colors, transparency, and detection radius are all configurable.

**Virtual Numpad** -- A clickable 3x3 grid overlay that floats on screen for mouse-based window positioning. Supports adjustable transparency, auto-hide, and saved position between sessions.

**Multi-Monitor Support** -- Move windows between monitors with a single hotkey. WinSplit detects all connected displays and handles DWM frame compensation for pixel-accurate placement.

**Mosaic Mode** -- Instantly tile all open windows into a grid across the current monitor with one hotkey.

**Fusion Mode** -- Combine two windows side-by-side (horizontal or vertical split) with a guided two-step hotkey sequence.

**Active Window Tools** -- A dedicated dialog (`Ctrl+Alt+T`) for the active window that provides transparency control via slider, always-on-top toggle, and screenshot capture (save to file or clipboard).

**Always on Top** -- Toggle any window to stay above all others with a single hotkey.

**Directional Maximize** -- Maximize a window horizontally only (full width, keep height) or vertically only (full height, keep width).

**Auto Placement** -- Record a window's position once, then restore it instantly with `Ctrl+Alt+Numpad 0`. Useful for applications you always want in the same spot.

**26 Languages** -- English, French, Italian, Spanish, German, Catalan, Dutch, Portuguese, Czech, Polish, Chinese (Traditional and Simplified), Arabic, Bengali, Greek, Hindi, Indonesian, Japanese, Korean, Persian, Romanian, Russian, Thai, Turkish, Ukrainian, and Vietnamese.

**Portable Mode** -- Run from a USB drive or any folder. When a `portable.txt` file is present alongside the executable, all settings are stored in the application directory instead of the user's AppData.

---

## Installation

1. Download the latest release ZIP from the [Releases page](https://github.com/x-vibe/winsplit-revolution-revived/releases/latest).
2. Extract the ZIP to any folder (for example, `C:\Tools\WinSplit Revolution` or a USB drive).
3. Run `Winsplit.exe`.

No installer is required. No administrator privileges are needed. To uninstall, delete the folder.

---

## Quick Start

After launching WinSplit, it runs in the system tray. All default hotkeys use `Ctrl+Alt` as the modifier.

### Core Hotkeys (Default)

| Hotkey | Action |
|--------|--------|
| `Ctrl+Alt+Numpad 7` | Snap window to top-left |
| `Ctrl+Alt+Numpad 8` | Snap window to top half |
| `Ctrl+Alt+Numpad 9` | Snap window to top-right |
| `Ctrl+Alt+Numpad 4` | Snap window to left half |
| `Ctrl+Alt+Numpad 5` | Full screen (maximize) |
| `Ctrl+Alt+Numpad 6` | Snap window to right half |
| `Ctrl+Alt+Numpad 1` | Snap window to bottom-left |
| `Ctrl+Alt+Numpad 2` | Snap window to bottom half |
| `Ctrl+Alt+Numpad 3` | Snap window to bottom-right |
| `Ctrl+Alt+Numpad 0` | Auto placement (restore saved position) |

### Extended Hotkeys (Default)

| Hotkey | Action |
|--------|--------|
| `Ctrl+Alt+Left Arrow` | Move window to left monitor |
| `Ctrl+Alt+Right Arrow` | Move window to right monitor |
| `Ctrl+Alt+M` | Mosaic mode (tile all windows) |
| `Ctrl+Alt+F` | Fusion mode (combine two windows) |
| `Ctrl+Alt+C` | Close all windows |
| `Ctrl+Alt+T` | Active window tools (transparency, screenshot) |
| `Ctrl+Alt+O` | Toggle always-on-top for active window |
| `Ctrl+Alt+H` | Maximize window horizontally |
| `Ctrl+Alt+V` | Maximize window vertically |
| `Ctrl+Alt+Page Down` | Minimize window |
| `Ctrl+Alt+Page Up` | Restore minimized windows |
| `Ctrl+Alt+N` | Toggle virtual numpad visibility |

### Typical Workflow

1. Launch `Winsplit.exe` -- it appears in the system tray.
2. Open a browser and press `Ctrl+Alt+Numpad 4` to snap it to the left half of the screen.
3. Open a code editor and press `Ctrl+Alt+Numpad 6` to snap it to the right half.
4. Press `Ctrl+Alt+Numpad 6` again on the editor to cycle it to a two-thirds width layout.
5. Need a reference window on a second monitor? Press `Ctrl+Alt+Right Arrow` to send it there.
6. Press `Ctrl+Alt+O` to pin a chat window on top of everything.
7. Press `Ctrl+Alt+T` to open the active window tools dialog, where you can adjust transparency or take a screenshot.

All 22 hotkeys are fully customizable via right-click on the tray icon and selecting the hotkey settings.

---

## Configuration

Right-click the system tray icon to access settings. The options dialog has five tabs:

| Tab | Settings |
|-----|----------|
| **General** | Language selection, auto-placement management, settings import/export |
| **Virtual Numpad** | Transparency slider, auto-hide, show at startup, save position on exit |
| **Web Update** | Automatic update checking frequency (startup, weekly, monthly) |
| **Drag'n'Go** | Enable/disable, detection radius, zone colors, zone transparency, modifier keys |
| **Misc** | Mouse follows window, active window tracking (X-Mouse), minimize/maximize cycling |

### Portable vs. Classic Mode

- **Portable mode:** Place a file named `portable.txt` in the same directory as `Winsplit.exe`. All configuration files (`settings.xml`, `hotkeys.xml`, layout data) are stored alongside the executable.
- **Classic mode:** Without `portable.txt`, settings are stored in `%APPDATA%\WinSplit\`.

### Configuration Files

Settings are stored in XML format:

- `settings.xml` -- General options, numpad settings, Drag'n'Go preferences
- `hotkeys.xml` -- All 22 hotkey definitions (modifiers, virtual key codes, enabled state)
- `layouts.xml` -- Custom layout grid ratios for each numpad position

Settings can be exported and imported via the General tab in the options dialog.

---

## Building from Source

### Prerequisites

- **Visual Studio 2022, 2025, or 2026** with the "Desktop development with C++" workload installed
- **Windows SDK 10.0.26100.0** or newer
- **Git** (to clone with submodules)

The wxWidgets 3.x library is included as a Git submodule and is built automatically by the build script.

### Build Steps

```cmd
git clone --recursive https://github.com/x-vibe/winsplit-revolution-revived.git
cd winsplit-revolution-revived
build.cmd
```

The `build.cmd` script performs four steps:

1. Auto-detects your Visual Studio installation via `vswhere.exe`
2. Builds wxWidgets from source (static library, x64 Release) if not already built
3. Builds the WinSplit Revolution solution (`Winsplit.exe` and `winsplithook.dll`)
4. Creates a portable ZIP package in the `dist/` directory

Output binaries are placed in `upstream/x64/Release/`.

### Manual Build (Developer Command Prompt)

If you prefer to build manually from a Visual Studio Developer Command Prompt:

```cmd
:: Build wxWidgets (one-time)
cd upstream\wxWidgets\build\msw
nmake -f makefile.vc BUILD=release RUNTIME_LIBS=static TARGET_CPU=X64

:: Build WinSplit
cd ..\..\..\
msbuild "Winsplit Revolution.sln" /p:Configuration=Release /p:Platform=x64 /m
```

### Build Output

| File | Description |
|------|-------------|
| `upstream/x64/Release/Winsplit.exe` | Main application |
| `upstream/x64/Release/winsplithook.dll` | Hook DLL for Drag'n'Go and window movement tracking |
| `dist/WinSplit-Revolution-v*-portable-x64.zip` | Ready-to-distribute portable package |

### VS Code Integration

The repository includes `.vscode/tasks.json` for building directly from VS Code with `Ctrl+Shift+B`. Available tasks include full build, release-only build, debug build, clean, and rebuild.

---

## Runtime Requirements

- **Operating System:** Windows 10 or Windows 11 (x64)
- **MSVC Redistributable:** Not required. The Release build uses static CRT linking (`/MT`), so there are no external runtime dependencies.
- **Disk Space:** Under 2 MB
- **RAM:** Minimal (typically under 10 MB resident)

---

## What's Changed Since the Original

This fork modernizes WinSplit Revolution for current Windows versions. Highlights:

### Stability & Compatibility

- Full Windows 10/11 support with DPI awareness (Per-Monitor DPI V2)
- Fixed DWM invisible frame handling for pixel-accurate window positioning
- Fixed application blocking Windows shutdown/restart
- Fixed Options dialog crash on Windows 11 (registry values that don't exist on modern Windows)
- Fixed startup crash from null pointer dereference in XML settings parsing
- Hardened all registry access with null checks and error suppression

### Security

- Replaced insecure HTTP update checking with HTTPS via GitHub Releases API
- Secure DLL loading with restricted search order
- Rate-limited hook message processing to prevent message flood attacks
- Reduced process permissions to minimum required
- Bounds checking on all XML config values
- Privilege elevation detection and warning at startup

### Modernization

- Migrated from wxWidgets 2.x to wxWidgets 3.x
- Builds with Visual Studio 2022+ and current Windows SDK
- Static CRT linking (no runtime dependencies)

[View full changelog](upstream/CHANGELOG.md)

---

## Contributing

Contributions are welcome. Here is how to get started:

1. Fork the repository and clone it with `--recursive` to include the wxWidgets submodule.
2. Create a feature branch from `main`.
3. Make your changes and verify they build with `build.cmd`.
4. Submit a pull request with a clear description of what changed and why.

### Areas Where Help Is Needed

- Testing on Windows 11 ARM64
- Translation corrections and new language contributions
- Accessibility testing (screen readers, high contrast, magnifier)
- Edge case testing with other window managers (PowerToys, DisplayFusion)

### Reporting Issues

Open an issue on the [GitHub issue tracker](https://github.com/x-vibe/winsplit-revolution-revived/issues) with:

- Your Windows version and build number
- Steps to reproduce the problem
- Expected vs. actual behavior

---

## Credits

**Original WinSplit Revolution** (2005-2009) created by Raphael Lencrerot, Xavier Perrissoud, Arturo Espinosa, and Dan Smith.

Kept alive by [dozius](https://github.com/dozius/winsplit-revolution) (Windows 10 fixes) and [skullzy](https://codeberg.org/skullzy/winsplit-revolution) (Codeberg preservation).

Now maintained by [x-vibe](https://github.com/x-vibe) -- contributions welcome!

---

## License

This software is licensed under the **GNU General Public License v3.0**.

You may redistribute and modify it under the terms of the GPL v3. See [LICENSE](upstream/LICENSE) for the full license text.
