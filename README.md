# WinSplit Revolution Revived

[![Build Status](https://github.com/x-vibe/winsplit-revolution-revived/actions/workflows/build.yml/badge.svg)](https://github.com/x-vibe/winsplit-revolution-revived/actions)
[![Release](https://img.shields.io/github/v/release/x-vibe/winsplit-revolution-revived)](https://github.com/x-vibe/winsplit-revolution-revived/releases)
[![License](https://img.shields.io/badge/license-GPL--3.0-blue.svg)](LICENSE)

**The lightweight, blazing-fast window manager for Windows 10 & 11.**

Boost your productivity with instant window tiling, snapping, and organization — all from your keyboard. WinSplit Revolution is a **free, open-source** alternative to paid window managers, with **zero bloat** and **no background services**.

## Why WinSplit Revolution?

| Feature | WinSplit | Others |
|---------|----------|--------|
| **Lightweight** | ~500KB, minimal RAM | Often 50MB+ |
| **Portable** | No install required | Requires installation |
| **Fast** | Instant response | Noticeable lag |
| **Free & Open Source** | GPL v3, forever free | Paid or freemium |
| **No telemetry** | Your data stays yours | Often tracks usage |

## Key Features

- **Keyboard-First Design** — 22 customizable global hotkeys for instant window control
- **Drag'n'Go** — Drag windows to screen edges for automatic snapping and resizing
- **Virtual Numpad** — Clickable 3x3 grid overlay for mouse-based positioning
- **Multi-Monitor Support** — Seamlessly move windows across displays with hotkeys
- **Highly Customizable** — Configure hotkeys, layouts, transparency, and behavior
- **26 Languages** — Full internationalization support
- **Portable Mode** — Run from USB, no installation or admin rights needed

## Download

**[⬇️ Download Latest Release (v10.2.0)](https://github.com/x-vibe/winsplit-revolution-revived/releases/latest)** — Windows 10/11 x64

Just extract and run. No installation required.

---

## Quick Start

| Hotkey | Action |
|--------|--------|
| `Ctrl+Alt+Numpad` | Position window (1-9 grid positions) |
| `Ctrl+Alt+Arrow` | Move window to next monitor |
| `Ctrl+Alt+M` | Mosaic mode (tile all windows) |
| `Ctrl+Alt+F` | Fusion mode (combine two windows) |

**Tip:** All hotkeys are fully customizable in Settings → Hotkeys.

---

## What's New

### v10.2.0 (Latest)
- **Critical Fix**: Fixed startup crash affecting some users
- **Security Hardened**: HTTPS updates, rate limiting, reduced permissions
- **Debug Logging**: New troubleshooting tools for developers
- **26 Languages**: All language names now display correctly

### v10.1.0
- **Windows 11 Native**: Full compatibility with Windows 11 features
- **High-DPI Support**: Crisp UI on 4K and high-resolution displays
- **Modern Codebase**: Updated to wxWidgets 3.x and VS 2022/2025/2026

[View Full Changelog](upstream/CHANGELOG.md)

---

## Screenshots

*Coming soon — contributions welcome!*

---

## Building from Source

### Requirements

- Visual Studio 2022/2025/2026 with "Desktop development with C++"
- Windows SDK 10.0.26100.0+
- wxWidgets 3.x (included as submodule)

### Quick Build

```cmd
git clone --recursive https://github.com/x-vibe/winsplit-revolution-revived.git
cd winsplit-revolution-revived
build.cmd
```

Output: `upstream/x64/Release/Winsplit.exe`

[Detailed Build Instructions](CLAUDE.md#build-commands)

---

## Contributing

Contributions are welcome! Areas where help is needed:

- [ ] Screenshots and demo GIFs
- [ ] Testing on Windows 11 ARM64
- [ ] Translation improvements
- [ ] Documentation

---

## Credits

**WinSplit Revolution** — Copyright (C) 2005-2009 Raphael Lencrerot

**Original Team:** Raphael Lencrerot, Xavier Perrissoud, Arturo Espinosa, Dan Smith

**Previous Maintainers:**
- [dozius](https://github.com/dozius/winsplit-revolution) — Windows 10 fixes
- [skullzy](https://codeberg.org/skullzy/winsplit-revolution) — Codeberg mirror

---

## License

**GNU General Public License v3** — Free software, forever.

You can redistribute and modify this software under the GPL v3 terms. See [LICENSE](upstream/LICENSE) for details.

---

## Links

- **GitHub**: https://github.com/x-vibe/winsplit-revolution-revived
- **Releases**: https://github.com/x-vibe/winsplit-revolution-revived/releases
- **Issues**: https://github.com/x-vibe/winsplit-revolution-revived/issues

---

**Keywords:** window manager, window tiling, snap windows, keyboard shortcuts, productivity tool, free window manager, open source, Windows 11 window manager, multi-monitor, hotkeys, lightweight
