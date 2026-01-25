# Fluent UI Icon Replacement for WinSplit Revolution

This folder contains scripts to download and convert icons from Microsoft's Fluent UI System Icons library to give WinSplit Revolution a modern Windows 11-style appearance.

**Source**: https://github.com/microsoft/fluentui-system-icons
**License**: MIT (free for commercial use)

## Quick Start

1. **Download icons** (requires PowerShell):
   ```powershell
   cd upstream/images/fluent-icons
   .\download-icons.ps1
   ```

2. **Convert icons** (requires ImageMagick):
   ```powershell
   .\convert-icons.ps1
   ```

3. **Rebuild WinSplit Revolution** to use the new icons.

## Prerequisites

- **PowerShell 5+** (included with Windows 10/11)
- **ImageMagick** for format conversion
  - Download: https://imagemagick.org/script/download.php#windows
  - Make sure `magick.exe` is in your PATH

## Icon Mapping

### Menu Icons (PNG 20x20)

| Output File | Fluent Icon | Style | Purpose |
|-------------|-------------|-------|---------|
| `options.png` | Settings | Regular | Options/Settings menu |
| `hotkeys.png` | Keyboard | Regular | Hotkey settings |
| `help.png` | Question Circle | Regular | Help menu |
| `about.png` | Info | Regular | About dialog |
| `exit.png` | Power | Regular | Exit application |
| `auto_start_true.png` | Checkmark Circle | Filled | Auto-start enabled |
| `auto_start_false.png` | Circle | Regular | Auto-start disabled |
| `layout.png` | Grid | Regular | Layout settings |

### Tray Icons (XPM 16x16)

| Output File | Fluent Icon | Purpose |
|-------------|-------------|---------|
| `icone.xpm` | Apps | Main tray icon |
| `tray2-6.xpm` | Arrow Sync (rotated) | Update check animation |

### Application Icon (ICO multi-resolution)

| Output File | Sizes | Purpose |
|-------------|-------|---------|
| `winsplit.ico` | 16, 32, 48, 256 | Main application icon |

## Manual Conversion Commands

If you need to convert icons manually:

```bash
# Convert SVG to PNG
magick -background none -density 96 -resize 20x20 input.svg output.png

# Convert PNG to XPM (for tray icons)
magick input.png -colors 256 output.xpm

# Create multi-resolution ICO
magick icon_16.png icon_32.png icon_48.png icon_256.png output.ico
```

## Notes

- XPM files may need manual color palette adjustment for proper transparency
- The Fluent icons use a dark fill color (#212121) by default
- For light-on-dark themes, consider using `-negate` or editing the SVG fill color

## Files in this Directory

After running the scripts, you'll have:

```
fluent-icons/
├── README.md           # This file
├── download-icons.ps1  # Downloads SVGs from GitHub
├── convert-icons.ps1   # Converts SVGs to PNG/XPM/ICO
├── *.svg               # Downloaded source files
```

The converted icons are placed in the parent `images/` directory.
