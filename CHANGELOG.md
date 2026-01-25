# Changelog

All notable changes to this project will be documented in this file.

The format is loosely based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- GitHub Actions CI/CD workflow for automated builds on Windows Server 2022
- Portable ZIP package with all required files and portable mode marker
- Local build script (build.cmd) for Windows developers
- Comprehensive edge case testing framework (235 new test scenarios)
- Windows 11 edge case tests: Virtual Desktops, Snap Layouts conflicts, WSLg, ARM64
- Accessibility testing: Screen readers (Narrator, NVDA, JAWS), High Contrast, Magnifier
- Special window handling tests: UAC elevated, UWP apps, fullscreen games, gaming overlays
- Hardware/display tests: HDR, docking stations, orientation changes, multi-GPU
- Software conflict detection: PowerToys FancyZones, DisplayFusion, remote access tools
- System state tests: Sleep/wake, hibernate, power modes, Windows Update
- Input method tests: Touch, pen/stylus, voice control
- Race condition and stress tests: Rapid operations, resource exhaustion
- CI/CD testing environment documentation for GitHub Actions and VMs

### Changed

- Updated NSIS installer script to version 10.0.2
- Updated publisher info and URLs in installer
- Expanded TASKS.md from 280 to 515 granular tasks
- Added priority matrix for task ordering

---

## 10.0.2

### Added

- Windows 11 compatibility with proper manifest declarations
- DPI awareness support (Per-Monitor DPI V2) for high-DPI displays
- Centralized DWM utilities module for consistent window frame handling

### Fixed

- Logic error in invisible frame detection that caused incorrect positioning
- Window positioning when moving between monitors on Windows 10/11
- Drag'n'Go preview now matches actual window placement
- Multi-monitor support with proper DWM frame compensation

### Changed

- Refactored window positioning code to use centralized DwmUtils module
- Updated Windows SDK target to support Windows 11 features

## 10.0.1

### Fixed

- Nuisance error if locale is not supported

## 10.0.0

### Added

- Hotkey for showing/hiding virtual number pad
- Additional registry locations for wallpaper lookup
- Attempt to add window tracking for Windows Vista and newer
- Button that restores virtual number pad position to a sane place

### Changed

- Order of appearance for virtual number pad

### Fixed

- Incorrect window size and position when using compositing themes i.e. win8,
  Win10 and others

- Inverted window transparency

## 9.0.2

- Option added to allow the "Minimize" and Maximize" hotkeys cycling between differents states
  - Minimize will cycle through "Minimize" => "Maximize" => "Restore" => "Minimize" => ...
  - Maximize will cycle through "Maximize" => "Minimize" => "Restore" => "Maximize" => ...
  - If this option isn't enabled (default state), these hotkeys will keep their actual comportment.

- Option added to allow (or prevent) moving the mouse pointer when moving a Window with hotkeys :
  - Another option added to allow this only when the mouse pointer is over the active window's area
  - These 2 options will be activated by default
  - The mouse pointer will stay in place when a window is moved with the Virtual Numpad

- Windows "Active Window Tracking" built-in functionality:
  - Activation or deactivation of this functionality.
  - Activation or deactivation of the "Auto bring to top" with "Active Window Tracking".
  - Setting the delay value before the "Active Window Tracking" acts.

- Drag'n'Go activation keys customization
- Settings importation and exportation
- Tools for the active window : allow you to set the transparency and/or the "always on top" style of the window, saving a screenshot of this window.
- New translations available : Czech, Polish, Chinese Traditional, Chinese simplified
- Drag'n'Go was no more working after registering a window position with the "Automatic placement" feature.
- Some windows (like Google Chrome) were not resized but only moved (bug not yet corrected for the "mosaic" function witch needs to be re-implemented)

## 8.0.7

- "Drag'n'Go" feature problem on Vista 32bits
- Layout values rounded to integer when the native language does not use the point as a decimal separator
- A grey window appeared sometimes on the upper left corner of the screen
- Settings were not written when WinSplit was exited by the end of the Windows session.
- VirtuaNumpad transparency not correctly managed

## 8.0.6

- Automated Update manager.
- Drag'n'Go feature added.
- Layout manager implemented
- Several bugs have been fixed.

## 1.9.0

- Minimize/restore window(s) with Ctrl+Alt+Page Down/Up implemented
- Options dialog has been changed for more effectiveness/simplicity
- Saving Virtual numpad position when windows (OS) shutdown corrected
- Moving always on top windows can be taken into account (doesn't work with virtual numpad)
- Icons in menu appear better
- Moving tooltips instead of windows corrected
- xml format used for saving datas
- Several bugs fixed

## 1.8.0

- Move a window from a screen to another (multimonitor) implemented Ctrl+Alt+ Arrow Left/Right
- Warmhotkey frame implemented, for hotkeys allocated by other programs
- Third left clic on the icon tray to hide virtual numpad frame
- Horizontal fuzion added
- Several bugs fixed

## 1.7.0

- Auto update detection implemented
- Added language Catalan

## 1.6.0

- Taking into account multimonitors
- Added languages German and Spanish

## 1.5.0

- Fixing several bugs
- Adding global hotkey configurable

## 1.4.0

- Fixing several bugs
- Multilanguage implemented
- Ability to have up to 6 windows for a same view

## 1.3.0

- Adding virtual numpad frame

## 1.2.0

- Rewriting some parts of the code for simplicity
- Adding icons in the tray menu
- Adding about frame
- Upgrading fuzion features
- Deactivate global hotkeys allocated by other programs

## 1.1.0

- Adding Ctrl+Alt+F function for fuzion mode

## 1.0.9

- Only one instance of WinSplit allowed
- Adding Ctr+Alt+M function for Mosaic mode
- Adding Ctr+Alt+C function for closing all opened windows
- Adding Ctr+Alt+Num pad 0 function for auto-positioning applications
- Avoid resizing non-resizable windows (detected with winamp)

## 1.0.0

- Implementation in C/C++ with Code::Bloks and wxWidgets
- Basic features for move windows
