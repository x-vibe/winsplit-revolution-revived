# WinSplit Revolution Revived

A fork of WinSplit Revolution updated for **Windows 11** compatibility.

WinSplit Revolution is a small utility which allows you to easily organize your open windows by tiling, resizing and positioning them to make the best use of your desktop real estate.

## Features

- Automated window handling (resize, move, reorganize, close tasks)
- Global hotkeys and clickable virtual numpad
- Drag'n'Go - drag windows to screen edges for quick positioning
- Multi-monitor support
- Layout customization
- Fusion between 2 windows
- Mosaic mode

## Download

**[Download Latest Release (v10.2.0)](https://github.com/x-vibe/winsplit-revolution-revived/releases/latest)**

## What's New in This Fork (v10.2.0)

### v10.2.0 (Latest)
- **CRITICAL FIX**: Fixed application startup crash in XML settings parsing
- **Fixed** support for both XML attribute and text content formats in settings
- **Fixed** language dropdown showing garbled characters (now displays English names)
- **Added** file-based debug logging system (`debug_log.h`)
- **Security**: HTTPS update mechanism via GitHub Releases API
- **Security**: Rate limiting on hook messages, DLL path verification
- **Security**: Reduced process permissions, integer overflow fixes

### v10.1.0
- **Windows 11 compatibility** with proper manifest declarations
- **DPI awareness** (Per-Monitor DPI V2) for high-DPI displays
- **wxWidgets 3.x migration** - updated from wxWidgets 2.x to 3.x
- **26 languages** - full internationalization support
- **Visual Studio 2022/2025/2026** build support (v143/v145 toolset)
- **Fixed** logic error in invisible frame detection
- **Fixed** window positioning when moving between monitors
- **Fixed** Drag'n'Go preview now matches actual window placement
- **Fixed** winsock header conflicts with WIN32_LEAN_AND_MEAN
- Centralized DWM utilities for consistent frame handling

## Building

> **Note (January 2026):** Version numbers and download links below were current as of January 2026. When installing dependencies, always check online for the latest versions.

### Requirements

- **Visual Studio 2026** with "Desktop development with C++" workload
- **Windows SDK 10.0.26100.0** (Windows 11 24H2 SDK)
- **wxWidgets 3.x** (included as submodule)

### Option 1: VS Code (Recommended)

The project includes VS Code configuration for building without the Visual Studio IDE.

1. Install [Visual Studio 2026](https://visualstudio.microsoft.com/downloads/) with "Desktop development with C++" workload
2. Open the project folder in VS Code: `E:\VIBECODING\DEV\OTHER\winsplit-revolution-revamped`
3. Press `Ctrl+Shift+B` to build (auto-detects VS installation)

Available build tasks (Terminal → Run Task):
| Task | Description |
|------|-------------|
| Build (Full) | Default - builds wxWidgets if needed, creates dist package |
| Build WinSplit (Release x64) | Quick release build |
| Build WinSplit (Debug x64) | Debug build |
| Clean Solution | Remove build artifacts |

### Option 2: Command Line

Run from a Developer Command Prompt or use the build script:

```cmd
cd E:\VIBECODING\DEV\OTHER\winsplit-revolution-revamped
build.cmd
```

Or manually:

```cmd
cd upstream\wxWidgets\build\msw
nmake -f makefile.vc BUILD=release RUNTIME_LIBS=static TARGET_CPU=X64

cd ..\..\..\
msbuild "Winsplit Revolution.sln" -property:Configuration=Release -property:Platform=x64
```

### Output

Built files are placed in `upstream\x64\Release\`:
- `Winsplit.exe` - Main application
- `winsplithook.dll` - Hook DLL

## Credits

### WINSPLIT REVOLUTION
**Copyright (C) 2005-2009**

Created by **Raphael Lencrerot**

#### WinSplit Revolution Team:
- Raphael Lencrerot (developer)
- Xavier Perrissoud (developer)
- Arturo Espinosa (developer)
- Dan Smith (developer)

Special thanks to **NX** (icons design)

### Previous Maintainers
- [dozius/winsplit-revolution](https://github.com/dozius/winsplit-revolution) - Windows 10 fixes
- [skullzy/winsplit-revolution](https://codeberg.org/skullzy/winsplit-revolution) - Codeberg mirror

## License

This program is free software: you can redistribute it and/or modify it under the terms of the **GNU General Public License v3** as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the [LICENSE](upstream/LICENSE) file for details.

## Links

- **This Fork**: https://github.com/x-vibe/winsplit-revolution-revived
- **Original Project**: https://github.com/dozius/winsplit-revolution
- **Codeberg Mirror**: https://codeberg.org/skullzy/winsplit-revolution
