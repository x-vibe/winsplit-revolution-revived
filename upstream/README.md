# WinSplit Revolution Revived

[![Build Status](https://github.com/x-vibe/winsplit-revolution-revived/actions/workflows/build.yml/badge.svg)](https://github.com/x-vibe/winsplit-revolution-revived/actions)
[![Release](https://img.shields.io/github/v/release/x-vibe/winsplit-revolution-revived)](https://github.com/x-vibe/winsplit-revolution-revived/releases)
[![License](https://img.shields.io/badge/license-GPL--3.0-blue.svg)](LICENSE)

A Windows utility for organizing windows by tiling, resizing, and positioning using hotkeys and a virtual numpad. **Now with full Windows 10/11 support.**

## Download

**[Download Latest Release](https://github.com/x-vibe/winsplit-revolution-revived/releases/latest)**

- **Portable ZIP** - Extract and run, no installation needed
- Works on Windows 10 and Windows 11 (x64)

## Features

- **Hotkey Window Management** - Resize and position windows using Ctrl+Alt+Numpad
- **Virtual Numpad** - Clickable 3x3 grid for mouse-based positioning
- **Multi-Monitor Support** - Move windows between monitors with Ctrl+Alt+Arrow
- **Drag'n'Go** - Hold modifier keys and drag windows to snap positions
- **Layout Manager** - Customizable grid layouts (25%, 33%, 50%, 67%, 75%)
- **Window Fusion** - Combine two windows side by side
- **Mosaic Mode** - Tile all open windows
- **Auto Placement** - Remember and restore window positions per application
- **26 Languages** - Full internationalization support

## Quick Start

1. Download the portable ZIP from [Releases](https://github.com/x-vibe/winsplit-revolution-revived/releases)
2. Extract to any folder
3. Run `Winsplit.exe`
4. Use **Ctrl+Alt+Numpad** keys to position the active window:

```
┌─────┬─────┬─────┐
│  7  │  8  │  9  │  Top row
├─────┼─────┼─────┤
│  4  │  5  │  6  │  Middle row (5 = maximize)
├─────┼─────┼─────┤
│  1  │  2  │  3  │  Bottom row
└─────┴─────┴─────┘
```

Press the same key multiple times to cycle through width percentages (25% → 33% → 50% → 67% → 75%).

## System Requirements

- Windows 10 or Windows 11 (x64)
- No additional runtime dependencies

## Building from Source

### Prerequisites

- Visual Studio 2022 or newer with "Desktop development with C++" workload
- Windows SDK 10.0.26100.0 or newer

### Build

```cmd
git clone https://github.com/x-vibe/winsplit-revolution-revived.git
cd winsplit-revolution-revived
build.cmd
```

Output: `x64/Release/Winsplit.exe`

### Manual Build

```cmd
# Build wxWidgets (first time only)
cd wxWidgets\build\msw
nmake -f makefile.vc BUILD=release RUNTIME_LIBS=static TARGET_CPU=X64

# Build WinSplit
cd ..\..
msbuild "Winsplit Revolution.sln" /p:Configuration=Release /p:Platform=x64
```

## Credits

**Original WinSplit Revolution** (2005-2009):
- Raphael Lencrerot (lead developer)
- Xavier Perrissoud
- Arturo Espinosa
- Dan Smith

**Previous Maintainers**:
- [dozius](https://github.com/dozius/winsplit-revolution) - Windows 10 invisible frame fix
- [skullzy](https://codeberg.org/skullzy/winsplit-revolution) - Codeberg maintenance

**This Fork**:
- [x-vibe](https://github.com/x-vibe) - Windows 11 support, wxWidgets 3.x migration

## License

GNU General Public License v3.0 - See [LICENSE](LICENSE) for details.

## Changelog

See [CHANGELOG.md](CHANGELOG.md) for version history.
