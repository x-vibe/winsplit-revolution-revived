# WinSplit Revolution - Comprehensive Task List

Master task list for the Windows 11 compatibility update and security hardening.

**Legend:** `[ ]` Pending | `[x]` Complete | `[~]` In Progress | `[!]` Blocked

---

## 1. Project Setup & Build System (20 tasks)

### 1.1 Repository Setup
- [x] 1. Clone from Codeberg (skullzy/winsplit-revolution)
- [x] 2. Create upstream/ subfolder structure
- [x] 3. Move all source files to upstream/
- [x] 4. Create root-level CLAUDE.md
- [x] 5. Create root-level README.md with credits
- [x] 6. Set up GitHub remote (x-vibe/winsplit-revolution-revived)
- [x] 7. Configure git remotes (origin=GitHub, codeberg=upstream)
- [ ] 8. Add .gitignore for build artifacts
- [ ] 9. Add .editorconfig for consistent formatting
- [ ] 10. Create CONTRIBUTING.md with guidelines

### 1.2 Build Configuration
- [ ] 11. Verify wxWidgets submodule is properly initialized
- [ ] 12. Update wxWidgets to latest stable (3.2.x)
- [ ] 13. Update Windows SDK target to 10.0.26100.0 (Win11 24H2 SDK)
- [ ] 14. Update platform toolset to v144 (VS2026)
- [ ] 15. Add backward compatibility for v143 (VS2022)
- [ ] 16. Add x64 as default platform
- [ ] 17. Remove Win32 configurations (deprecated)
- [ ] 18. Enable /W4 warning level
- [ ] 19. Enable /WX treat warnings as errors
- [ ] 20. Add /analyze static analysis
- [ ] 20a. Configure PDB generation for release builds

### 1.3 State-of-the-Art Build Tooling
- [ ] 20b. Add CMake build system as alternative to MSBuild
- [ ] 20c. Add vcpkg manifest for dependency management
- [ ] 20d. Add Clang/LLVM build support (clang-cl)
- [ ] 20e. Add Ninja build generator support
- [ ] 20f. Add C++23 language standard support
- [ ] 20g. Add module support (C++20 modules)
- [ ] 20h. Add sanitizer builds (ASan, UBSan)
- [ ] 20i. Add LTO (Link-Time Optimization) for release
- [ ] 20j. Add reproducible builds configuration
- [ ] 20k. Add SBOM (Software Bill of Materials) generation

---

## 2. Security Fixes - CRITICAL (25 tasks)

### 2.1 HTTP Update Vulnerability (MITM Attack)
- [ ] 21. Audit update_thread.cpp for all HTTP URLs
- [ ] 22. Change update URL to HTTPS
- [ ] 23. Implement SSL/TLS certificate validation
- [ ] 24. Add certificate pinning for update server
- [ ] 25. Implement update package signing
- [ ] 26. Create RSA key pair for signing
- [ ] 27. Add signature verification before applying updates
- [ ] 28. Implement secure hash verification (SHA-256)
- [ ] 29. Add fallback timeout for failed connections
- [ ] 30. Log all update check attempts
- [ ] 31. Add user notification for failed verification
- [ ] 32. Disable auto-update until fixed (temporary)
- [ ] 33. Write test for HTTPS enforcement
- [ ] 34. Write test for certificate validation
- [ ] 35. Document secure update protocol

### 2.2 Hook Message Spoofing Vulnerability
- [ ] 36. Audit frame_hook.cpp message handling
- [ ] 37. Document WSM_STARTMOVING/STOPMOVING protocol
- [ ] 38. Add message source validation
- [ ] 39. Implement shared secret between DLL and EXE
- [ ] 40. Use named pipe instead of window messages
- [ ] 41. Add process ID verification for messages
- [ ] 42. Implement message sequence numbers
- [ ] 43. Add rate limiting for messages
- [ ] 44. Validate wheel delta range (0-8 only)
- [ ] 45. Add bounds checking for array indexing

---

## 3. Security Fixes - HIGH (25 tasks)

### 3.1 DLL Hijacking Prevention
- [ ] 46. Audit LoadLibrary calls in frame_hook.cpp
- [ ] 47. Change to LoadLibraryEx with full path
- [ ] 48. Call SetDllDirectory(L"") before loading
- [ ] 49. Use SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_SYSTEM32)
- [ ] 50. Verify DLL path before loading
- [ ] 51. Check DLL digital signature before loading
- [ ] 52. Log DLL load location for debugging
- [ ] 53. Add SafeDllSearchMode registry check
- [ ] 54. Document DLL search order mitigation
- [ ] 55. Write test for DLL load path

### 3.2 Process Access Permissions
- [ ] 56. Audit all OpenProcess calls
- [ ] 57. Replace PROCESS_ALL_ACCESS with minimum rights
- [ ] 58. Use PROCESS_QUERY_LIMITED_INFORMATION where possible
- [ ] 59. Use PROCESS_VM_READ only when needed
- [ ] 60. Handle access denied gracefully
- [ ] 61. Skip protected processes (antivirus, system)
- [ ] 62. Add SeDebugPrivilege handling
- [ ] 63. Log permission failures
- [ ] 64. Write test for minimum permissions
- [ ] 65. Document required permissions

### 3.3 Input Validation
- [ ] 66. Add integer overflow checks in XML parsing
- [ ] 67. Validate VnpTransparency range (0-255)
- [ ] 68. Validate DnGDetectionRadius bounds
- [ ] 69. Validate hotkey modifier combinations
- [ ] 70. Validate window coordinates are on-screen

---

## 4. Security Fixes - MEDIUM (20 tasks)

### 4.1 XML Configuration Security
- [ ] 71. Disable DTD processing (prevent XXE)
- [ ] 72. Disable external entity resolution
- [ ] 73. Set maximum XML file size limit
- [ ] 74. Add schema validation for settings.xml
- [ ] 75. Add schema validation for layout.xml
- [ ] 76. Add schema validation for hotkeys.xml
- [ ] 77. Sanitize string inputs from XML
- [ ] 78. Handle malformed XML gracefully
- [ ] 79. Write test for XXE prevention
- [ ] 80. Write test for malformed XML

### 4.2 Memory Safety
- [ ] 81. Replace raw pointers with smart pointers
- [ ] 82. Add nullptr checks before dereferences
- [ ] 83. Fix potential buffer overflows in string handling
- [ ] 84. Add bounds checking for array access
- [ ] 85. Initialize all variables at declaration
- [ ] 86. Use RAII for resource management
- [ ] 87. Fix negative array index in frame_hook.cpp:202
- [ ] 88. Add memory sanitizer to CI
- [ ] 89. Run Dr. Memory in CI pipeline
- [ ] 90. Document memory management patterns

---

## 5. Windows 11 Compatibility (30 tasks)

### 5.1 DWM Frame Compensation
- [x] 91. Create dwm_utils.h header
- [x] 92. Create dwm_utils.cpp implementation
- [x] 93. Implement GetWindowRectCompensated()
- [x] 94. Implement AdjustForInvisibleFrame()
- [x] 95. Implement GetInvisibleFrameBorders()
- [x] 96. Fix logic error in functions_resize.cpp
- [x] 97. Add DWM compensation to multimonitor_move.cpp
- [x] 98. Add DWM compensation to frame_hook.cpp
- [x] 99. Add DWM compensation to layout_manager.cpp
- [ ] 100. Test on Windows 11 22H2
- [ ] 101. Test on Windows 11 23H2
- [ ] 102. Test with rounded corners enabled
- [ ] 103. Test with snap layouts enabled
- [ ] 104. Handle DWM disabled scenario
- [ ] 105. Document DWM frame behavior

### 5.2 Manifest & Compatibility
- [x] 106. Create winsplit.manifest file
- [x] 107. Add Windows 11 supportedOS GUID
- [x] 108. Add Windows 10 supportedOS GUID
- [x] 109. Add Windows 8.1 supportedOS GUID
- [x] 110. Embed manifest in ressource.rc
- [ ] 111. Add longPathAware element
- [ ] 112. Add heapType="SegmentHeap" for Win10+
- [ ] 113. Test manifest embedding
- [ ] 114. Verify compatibility mode not triggered
- [ ] 115. Document manifest requirements

### 5.3 DPI Awareness
- [x] 116. Add PerMonitorV2 DPI awareness to manifest
- [x] 117. Implement InitializeDpiAwareness() in dwm_utils
- [x] 118. Add SetProcessDpiAwarenessContext call
- [x] 119. Add fallback to SetProcessDpiAwareness
- [x] 120. Add fallback to SetProcessDPIAware
- [ ] 121. Implement GetDpiForHwnd() wrapper
- [ ] 122. Implement ScaleRectForDpi()
- [ ] 123. Scale UI elements for DPI
- [ ] 124. Handle WM_DPICHANGED message
- [ ] 125. Test at 100% scaling
- [ ] 126. Test at 125% scaling
- [ ] 127. Test at 150% scaling
- [ ] 128. Test at 200% scaling
- [ ] 129. Test at 250% scaling
- [ ] 130. Test with mixed DPI monitors

---

## 6. Multi-Monitor Support (20 tasks)

### 6.1 Monitor Detection
- [ ] 131. Audit EnumDisplayMonitors usage
- [ ] 132. Handle monitor hotplug events
- [ ] 133. Cache monitor information properly
- [ ] 134. Invalidate cache on WM_DISPLAYCHANGE
- [ ] 135. Handle monitors with different resolutions
- [ ] 136. Handle monitors with different orientations
- [ ] 137. Support virtual desktops (Win10+)
- [ ] 138. Document monitor enumeration

### 6.2 Cross-Monitor Operations
- [x] 139. Fix DWM compensation on monitor move
- [ ] 140. Fix proportional sizing across monitors
- [ ] 141. Handle negative coordinates (left monitor)
- [ ] 142. Test with 2 monitors same resolution
- [ ] 143. Test with 2 monitors different resolution
- [ ] 144. Test with 3+ monitors
- [ ] 145. Test with vertical monitor arrangement
- [ ] 146. Test with diagonal monitor arrangement
- [ ] 147. Test monitor disconnect during move
- [ ] 148. Document multi-monitor behavior

---

## 7. Code Quality & Refactoring (25 tasks)

### 7.1 Code Modernization
- [ ] 149. Update to C++17 standard
- [ ] 150. Replace NULL with nullptr
- [ ] 151. Replace typedef with using
- [ ] 152. Use auto where appropriate
- [ ] 153. Use range-based for loops
- [ ] 154. Use std::string_view for read-only strings
- [ ] 155. Use constexpr for compile-time constants
- [ ] 156. Remove unused code and includes
- [ ] 157. Fix all compiler warnings
- [ ] 158. Run clang-tidy analysis

### 7.2 Architecture Improvements
- [ ] 159. Document singleton pattern usage
- [ ] 160. Consider dependency injection for testing
- [ ] 161. Separate UI from business logic
- [ ] 162. Create interface for settings storage
- [ ] 163. Create interface for hotkey registration
- [ ] 164. Add logging framework
- [ ] 165. Implement structured logging
- [ ] 166. Add debug log file option
- [ ] 167. Create error code enumeration
- [ ] 168. Standardize error handling

### 7.3 Code Documentation
- [ ] 169. Add file header comments
- [ ] 170. Document all public functions
- [ ] 171. Document complex algorithms
- [ ] 172. Add inline comments for tricky code
- [ ] 173. Create architecture diagram

---

## 8. Testing (35 tasks)

### 8.1 Test Infrastructure
- [x] 174. Create tests/ directory structure
- [x] 175. Add CMakeLists.txt for tests
- [x] 176. Integrate Google Test framework
- [x] 177. Integrate Catch2 framework
- [x] 178. Create test_harness.h utilities
- [x] 179. Create mock_window.cpp utility
- [x] 180. Create message_spoofer.cpp tool
- [ ] 181. Add code coverage reporting
- [ ] 182. Set up CI test runner
- [ ] 183. Add test result reporting

### 8.2 Security Tests
- [x] 184. Create gtest_hook_spoofing.cpp
- [x] 185. Create gtest_dll_security.cpp
- [x] 186. Create gtest_xml_injection.cpp
- [x] 187. Create gtest_http_updates.cpp
- [x] 188. Create test_hook_message_spoofing.cpp (standalone)
- [x] 189. Create test_dll_hijacking.cpp (standalone)
- [x] 190. Create test_xml_injection.cpp (standalone)
- [x] 191. Create test_http_updates.cpp (standalone)
- [ ] 192. Test buffer overflow scenarios
- [ ] 193. Test privilege escalation attempts
- [ ] 194. Test malicious input handling

### 8.3 Functional Tests
- [x] 195. Create catch2_window_positioning.cpp
- [x] 196. Create catch2_multimonitor.cpp
- [x] 197. Create catch2_dpi_awareness.cpp
- [x] 198. Create test_regression.cpp
- [ ] 199. Create test_hotkey_registration.cpp
- [ ] 200. Create test_drag_n_go.cpp
- [ ] 201. Create test_virtual_numpad.cpp
- [ ] 202. Create test_settings_persistence.cpp
- [ ] 203. Create test_layout_cycling.cpp
- [ ] 204. Create test_window_fusion.cpp

### 8.4 Stress Tests
- [x] 205. Create test_rapid_hotkeys.cpp
- [x] 206. Create test_memory_leaks.cpp
- [ ] 207. Create test_long_running.cpp (24hr test)
- [ ] 208. Create test_high_load.cpp
- [ ] 209. Test with 100+ windows open
- [ ] 210. Test with rapid monitor changes

### 8.5 Pentest Integration
- [x] 211. Create setup_pentest_tools.cmd
- [x] 212. Create run_pentest.cmd
- [x] 213. Integrate Dr. Memory download
- [x] 214. Integrate PE-bear download
- [x] 215. Integrate Dependencies download
- [ ] 216. Integrate WinAFL fuzzing
- [ ] 217. Create fuzzing harness
- [ ] 218. Run fuzzing campaign (1M iterations)

---

## 9. Documentation (20 tasks)

### 9.1 User Documentation
- [x] 219. Create README.md with overview
- [ ] 220. Create INSTALL.md with build instructions
- [ ] 221. Create USER_GUIDE.md with usage
- [ ] 222. Document all hotkey combinations
- [ ] 223. Document configuration options
- [ ] 224. Create troubleshooting guide
- [ ] 225. Add FAQ section
- [ ] 226. Create screenshot gallery
- [ ] 227. Record demo video/GIF

### 9.2 Developer Documentation
- [x] 228. Update CLAUDE.md with architecture
- [ ] 229. Create ARCHITECTURE.md
- [ ] 230. Document build process
- [ ] 231. Document test process
- [ ] 232. Document release process
- [ ] 233. Create API documentation
- [ ] 234. Document hook DLL protocol
- [ ] 235. Document IPC mechanism
- [ ] 236. Create security design document

### 9.3 Project Documentation
- [x] 237. Create CHANGELOG.md
- [ ] 238. Create SECURITY.md policy
- [ ] 239. Create CODE_OF_CONDUCT.md
- [ ] 240. Create LICENSE file (verify GPL v3)

---

## 10. CI/CD & Automation (15 tasks)

### 10.1 GitHub Actions
- [ ] 241. Create build workflow
- [ ] 242. Create test workflow
- [ ] 243. Create release workflow
- [ ] 244. Add Windows Server 2022 runner
- [ ] 245. Add Windows 11 runner (if available)
- [ ] 246. Cache wxWidgets build
- [ ] 247. Cache vcpkg packages
- [ ] 248. Add build status badge

### 10.2 Quality Gates
- [ ] 249. Require passing tests for merge
- [ ] 250. Require code review for merge
- [ ] 251. Add CodeQL security scanning
- [ ] 252. Add dependency vulnerability scanning
- [ ] 253. Add commit message linting
- [ ] 254. Add branch protection rules
- [ ] 255. Set up automatic releases

---

## 11. Release Preparation (15 tasks)

### 11.1 Code Signing
- [ ] 256. Obtain code signing certificate
- [ ] 257. Sign WinSplit.exe
- [ ] 258. Sign winsplithook.dll
- [ ] 259. Add timestamp to signatures
- [ ] 260. Verify signatures in CI
- [ ] 261. Document signing process

### 11.2 Installer
- [ ] 262. Create Inno Setup installer script
- [ ] 263. Add desktop shortcut option
- [ ] 264. Add start menu entry
- [ ] 265. Add uninstaller
- [ ] 266. Add upgrade detection
- [ ] 267. Sign installer
- [x] 268. Create portable ZIP package
- [ ] 269. Test silent install
- [ ] 270. Test upgrade from v9.x

---

## 12. Windows 11 Edge Cases (30 tasks)

### 12.1 Virtual Desktops & Task View
- [ ] 281. Test window positioning only affects current virtual desktop
- [ ] 282. Test hotkeys don't trigger on inactive virtual desktops
- [ ] 283. Test window move to different virtual desktop mid-operation
- [ ] 284. Test Task View (Win+Tab) interaction with drag'n'go
- [ ] 285. Test virtual desktop switch during window resize
- [ ] 286. Handle IVirtualDesktopManager COM interface
- [ ] 287. Document virtual desktop behavior

### 12.2 Snap Layouts Conflict Detection
- [ ] 288. Test with Windows Snap Layouts enabled (default)
- [ ] 289. Test with Windows Snap Layouts disabled
- [ ] 290. Detect when Snap Layouts flyout is active
- [ ] 291. Test Snap Groups behavior (grouped windows)
- [ ] 292. Test Win+Z Snap Layout shortcut conflict
- [ ] 293. Test edge/corner snap zones don't conflict
- [ ] 294. Add option to disable Windows Snap integration
- [ ] 295. Document Snap Layouts coexistence strategy

### 12.3 Windows Subsystems
- [ ] 296. Test with WSLg windows (Linux GUI apps)
- [ ] 297. Test WSL2 terminal windows
- [ ] 298. Test Windows Subsystem for Android (WSA) apps
- [ ] 299. Test Android app window chrome differences
- [ ] 300. Handle WSLg window class names
- [ ] 301. Document subsystem window behavior

### 12.4 Windows on ARM
- [ ] 302. Build ARM64 target configuration
- [ ] 303. Test on Windows 11 ARM (Snapdragon)
- [ ] 304. Test x64 emulation layer behavior
- [ ] 305. Test ARM64EC compatibility
- [ ] 306. Verify hook DLL works under emulation
- [ ] 307. Document ARM64 support status

### 12.5 Other Windows 11 Features
- [ ] 308. Test with Focus Assist / Do Not Disturb enabled
- [ ] 309. Test with Widgets panel open (Win+W)
- [ ] 310. Test with Chat/Teams integration active

---

## 13. Accessibility Testing (25 tasks)

### 13.1 Visual Accessibility
- [ ] 311. Test with High Contrast mode (all themes)
- [ ] 312. Test with Color Filters enabled
- [ ] 313. Test with Magnifier running
- [ ] 314. Test with Magnifier in full-screen mode
- [ ] 315. Test with Magnifier in lens mode
- [ ] 316. Test with Magnifier in docked mode
- [ ] 317. Test with Night Light / blue light filter
- [ ] 318. Verify UI is visible in all contrast modes
- [ ] 319. Document accessibility support status

### 13.2 Screen Reader Compatibility
- [ ] 320. Test with Windows Narrator
- [ ] 321. Test with NVDA screen reader
- [ ] 322. Test with JAWS screen reader
- [ ] 323. Add accessible names to UI elements
- [ ] 324. Test tray icon accessibility
- [ ] 325. Test virtual numpad accessibility
- [ ] 326. Verify hotkey actions announced

### 13.3 Input Accessibility
- [ ] 327. Test with Sticky Keys enabled
- [ ] 328. Test with Filter Keys enabled
- [ ] 329. Test with Toggle Keys enabled
- [ ] 330. Test with Mouse Keys enabled
- [ ] 331. Test with On-Screen Keyboard
- [ ] 332. Verify hotkey combinations work with accessibility features
- [ ] 333. Test with Eye Control enabled
- [ ] 334. Test with Voice Access (Win11)
- [ ] 335. Document keyboard accessibility

---

## 14. Special Window Types (35 tasks)

### 14.1 Elevated & Protected Windows
- [ ] 336. Test resizing UAC-elevated windows (admin apps)
- [ ] 337. Test when WinSplit runs as admin
- [ ] 338. Test when WinSplit runs as standard user
- [ ] 339. Handle UIPI (User Interface Privilege Isolation)
- [ ] 340. Skip or warn for protected processes
- [ ] 341. Test with Task Manager (always elevated)
- [ ] 342. Test with Registry Editor (regedit)
- [ ] 343. Test with Device Manager
- [ ] 344. Log access denied gracefully
- [ ] 345. Document elevation requirements

### 14.2 UWP & Store Apps
- [ ] 346. Test with UWP apps (Calculator, Photos, etc.)
- [ ] 347. Handle ApplicationFrameHost window class
- [ ] 348. Test with Microsoft Store app
- [ ] 349. Test with Windows Terminal (hybrid UWP)
- [ ] 350. Test with PWA apps (Edge-installed web apps)
- [ ] 351. Handle CoreWindow class
- [ ] 352. Handle Windows.UI.Core.CoreWindow
- [ ] 353. Document UWP limitations

### 14.3 Fullscreen & Immersive
- [ ] 354. Test with fullscreen exclusive games
- [ ] 355. Test with borderless fullscreen games
- [ ] 356. Test with DirectX exclusive fullscreen
- [ ] 357. Detect and skip fullscreen windows
- [ ] 358. Test Alt+Tab from fullscreen
- [ ] 359. Test with Windows Media Player fullscreen
- [ ] 360. Test with Netflix/streaming app fullscreen
- [ ] 361. Handle HWND_TOPMOST fullscreen

### 14.4 Gaming Overlays
- [ ] 362. Test with Xbox Game Bar active (Win+G)
- [ ] 363. Test with Steam overlay
- [ ] 364. Test with Discord overlay
- [ ] 365. Test with NVIDIA GeForce Experience overlay
- [ ] 366. Test with AMD Adrenalin overlay
- [ ] 367. Skip overlay windows (WS_EX_TOOLWINDOW detection)
- [ ] 368. Document gaming compatibility

### 14.5 Special Window Styles
- [ ] 369. Test borderless windows (no WS_BORDER)
- [ ] 370. Test tool windows (WS_EX_TOOLWINDOW)
- [ ] 371. Test layered windows (WS_EX_LAYERED)
- [ ] 372. Test click-through windows (WS_EX_TRANSPARENT)
- [ ] 373. Test windows with WS_EX_NOACTIVATE
- [ ] 374. Test non-resizable windows (no WS_THICKFRAME)
- [ ] 375. Test windows with min/max size constraints
- [ ] 376. Test owned windows (child dialogs)
- [ ] 377. Test popup windows
- [ ] 378. Handle cloaked windows (DWM_CLOAKED)

---

## 15. Hardware & Display Edge Cases (30 tasks)

### 15.1 HDR & Advanced Displays
- [ ] 379. Test with HDR display enabled
- [ ] 380. Test with Auto HDR (Windows 11)
- [ ] 381. Test with SDR content on HDR display
- [ ] 382. Test color profile changes
- [ ] 383. Test Variable Refresh Rate (VRR/G-Sync/FreeSync)
- [ ] 384. Document HDR compatibility

### 15.2 Laptop & Mobile Scenarios
- [ ] 385. Test laptop lid close with external monitor
- [ ] 386. Test laptop lid open with external monitor
- [ ] 387. Test docking station connect
- [ ] 388. Test docking station disconnect
- [ ] 389. Test USB-C display hotplug
- [ ] 390. Test Thunderbolt dock scenarios
- [ ] 391. Test tablet mode toggle (2-in-1 devices)
- [ ] 392. Test detachable keyboard scenarios
- [ ] 393. Handle WM_SETTINGCHANGE for tablet mode

### 15.3 Display Configuration Changes
- [ ] 394. Test resolution change while running
- [ ] 395. Test refresh rate change
- [ ] 396. Test color depth change
- [ ] 397. Test display orientation change (portrait/landscape)
- [ ] 398. Test extend vs duplicate display modes
- [ ] 399. Test projector mode (Win+P scenarios)
- [ ] 400. Test Miracast/wireless display
- [ ] 401. Handle WM_DISPLAYCHANGE robustly
- [ ] 402. Document display change handling

### 15.4 GPU Scenarios
- [ ] 403. Test with hybrid graphics (Intel+NVIDIA)
- [ ] 404. Test GPU switch during operation
- [ ] 405. Test with external GPU (eGPU)
- [ ] 406. Test GPU driver crash recovery
- [ ] 407. Test DWM restart recovery
- [ ] 408. Handle D3D device lost scenarios

---

## 16. Software Conflict Testing (25 tasks)

### 16.1 Other Window Managers
- [ ] 409. Test with PowerToys FancyZones
- [ ] 410. Test with DisplayFusion
- [ ] 411. Test with AquaSnap
- [ ] 412. Test with Divvy
- [ ] 413. Test with WindowGrid
- [ ] 414. Test with bug.n (tiling WM)
- [ ] 415. Test with Actual Window Manager
- [ ] 416. Detect and warn about conflicts
- [ ] 417. Document coexistence strategies

### 16.2 Automation & Scripting Tools
- [ ] 418. Test with AutoHotkey scripts running
- [ ] 419. Test with AutoIt scripts
- [ ] 420. Test with PowerShell window manipulation
- [ ] 421. Test with UIAutomation clients
- [ ] 422. Handle competing hotkey registrations
- [ ] 423. Handle RegisterHotKey failures gracefully

### 16.3 Remote Access Software
- [ ] 424. Test when TeamViewer is connected
- [ ] 425. Test when AnyDesk is connected
- [ ] 426. Test when Chrome Remote Desktop active
- [ ] 427. Test inside RDP session (incoming)
- [ ] 428. Test RDP client window (outgoing)
- [ ] 429. Test in Citrix virtual desktop
- [ ] 430. Test in VMware Horizon
- [ ] 431. Test in Azure Virtual Desktop
- [ ] 432. Document remote session limitations

### 16.4 Security Software
- [ ] 433. Test with various antivirus (Defender, Norton, etc.)
- [ ] 434. Handle hook injection blocks
- [ ] 435. Test with application whitelisting (AppLocker)
- [ ] 436. Test with Windows Sandbox
- [ ] 437. Test with Hyper-V windows

---

## 17. System State Testing (20 tasks)

### 17.1 Power State Transitions
- [ ] 438. Test sleep and wake cycle
- [ ] 439. Test hibernate and resume
- [ ] 440. Test Modern Standby (S0 low power)
- [ ] 441. Test Fast Startup
- [ ] 442. Recover state after resume
- [ ] 443. Re-register hotkeys after wake
- [ ] 444. Handle hook DLL state after resume

### 17.2 System Events
- [ ] 445. Test behavior during Windows Update install
- [ ] 446. Test User Account switch (fast user switching)
- [ ] 447. Test lock screen and unlock
- [ ] 448. Handle WM_QUERYENDSESSION properly
- [ ] 449. Handle WM_ENDSESSION properly
- [ ] 450. Test system restore point creation
- [ ] 451. Test with Memory Integrity (HVCI) enabled

### 17.3 Performance Modes
- [ ] 452. Test in Battery Saver mode
- [ ] 453. Test in Best Performance mode
- [ ] 454. Test in Best Power Efficiency mode
- [ ] 455. Test with efficiency mode (Windows 11)
- [ ] 456. Test under thermal throttling
- [ ] 457. Document performance impact

---

## 18. Input Method Testing (20 tasks)

### 18.1 Touch Input
- [ ] 458. Test drag'n'go with touch
- [ ] 459. Test virtual numpad with touch
- [ ] 460. Test multi-touch gestures
- [ ] 461. Test pinch-to-zoom interaction
- [ ] 462. Test touch-and-hold (right-click)
- [ ] 463. Handle WM_TOUCH messages
- [ ] 464. Handle WM_POINTER messages

### 18.2 Pen & Stylus
- [ ] 465. Test with Surface Pen
- [ ] 466. Test with Wacom stylus
- [ ] 467. Test pen button customization conflicts
- [ ] 468. Handle WM_POINTERDOWN from pen

### 18.3 Alternative Input
- [ ] 469. Test with gamepad (Xbox controller)
- [ ] 470. Test with external trackpad
- [ ] 471. Test with trackball
- [ ] 472. Test with vertical mouse
- [ ] 473. Test with high-DPI gaming mouse
- [ ] 474. Test with keyboard with macro keys
- [ ] 475. Handle HID device changes

---

## 19. Race Conditions & Timing (20 tasks)

### 19.1 Rapid Operations
- [ ] 476. Test rapid window create/destroy
- [ ] 477. Test multiple hotkeys within 100ms
- [ ] 478. Test resize during minimize animation
- [ ] 479. Test resize during maximize animation
- [ ] 480. Test move during DPI change
- [ ] 481. Test hook messages during shutdown

### 19.2 Concurrent Operations
- [ ] 482. Test multiple windows moving simultaneously
- [ ] 483. Test resize while another app also resizing
- [ ] 484. Test monitor disconnect during drag
- [ ] 485. Test DPI change during drag'n'go
- [ ] 486. Test resolution change during resize
- [ ] 487. Handle window destroyed mid-operation

### 19.3 Resource Exhaustion
- [ ] 488. Test with 100+ windows open
- [ ] 489. Test with 500+ windows open
- [ ] 490. Test under low memory conditions
- [ ] 491. Test under high CPU load (90%+)
- [ ] 492. Test GDI object limit approach
- [ ] 493. Test USER object limit approach
- [ ] 494. Test handle limit approach
- [ ] 495. Document resource limits

---

## 20. CI/CD Testing Environments (20 tasks)

### 20.1 GitHub Actions Windows Testing
- [ ] 496. Configure Windows Server 2022 runner
- [ ] 497. Configure Windows Server 2019 runner
- [ ] 498. Test build on both server versions
- [ ] 499. Run unit tests in CI
- [ ] 500. Run security tests in CI
- [ ] 501. Add test matrix for configurations
- [ ] 502. Cache test dependencies
- [ ] 503. Generate test reports

### 20.2 VM-Based Testing
- [ ] 504. Create Windows 11 QEMU/KVM VM script
- [ ] 505. Create Windows 10 QEMU/KVM VM script
- [ ] 506. Document Vagrant box setup
- [ ] 507. Create Packer template for test VMs
- [ ] 508. Add VM snapshot for clean state testing
- [ ] 509. Script automated test execution in VM

### 20.3 Self-Hosted Runners
- [ ] 510. Document self-hosted Windows 11 runner setup
- [ ] 511. Create runner setup script
- [ ] 512. Add desktop GUI test capability
- [ ] 513. Configure multi-monitor test environment
- [ ] 514. Add high-DPI test configuration
- [ ] 515. Add ARM64 test runner (if available)

---

## 21. Future Features (10 tasks)

### 21.1 Windows 11 Native Features

- [ ] 516. Integrate with Snap Layouts API
- [ ] 517. Support Snap Groups
- [ ] 518. Add rounded corner awareness
- [ ] 519. Support Windows 11 context menus
- [ ] 520. Add Mica/Acrylic backdrop option

### 21.2 Quality of Life

- [ ] 521. Add dark mode support
- [ ] 522. Add system theme following
- [ ] 523. Modernize UI design
- [ ] 524. Add keyboard-only operation mode
- [ ] 525. Add touch/tablet support

---

## Summary

| Category | Total | Complete | Pending |
|----------|-------|----------|---------|
| 1. Setup & Build | 20 | 7 | 13 |
| 2. Security Critical | 25 | 0 | 25 |
| 3. Security High | 25 | 0 | 25 |
| 4. Security Medium | 20 | 0 | 20 |
| 5. Windows 11 Compat | 30 | 20 | 10 |
| 6. Multi-Monitor | 20 | 5 | 15 |
| 7. Code Quality | 25 | 5 | 20 |
| 8. Testing | 35 | 23 | 12 |
| 9. Documentation | 20 | 10 | 10 |
| 10. CI/CD | 15 | 0 | 15 |
| 11. Release | 15 | 5 | 10 |
| 12. Win11 Edge Cases | 30 | 0 | 30 |
| 13. Accessibility | 25 | 0 | 25 |
| 14. Special Windows | 43 | 0 | 43 |
| 15. Hardware/Display | 30 | 0 | 30 |
| 16. Software Conflicts | 29 | 0 | 29 |
| 17. System State | 20 | 0 | 20 |
| 18. Input Methods | 18 | 0 | 18 |
| 19. Race Conditions | 20 | 0 | 20 |
| 20. CI/CD Environments | 20 | 0 | 20 |
| 21. Future Features | 10 | 0 | 10 |
| **TOTAL** | **515** | **75** | **440** |

**v10.1.0 Released:** 2026-01-25 - First public release with Windows 11 support

---

## Priority Matrix

### High Priority (Next Release)
- Security Critical (#21-45)
- Security High (#46-70)
- Win11 Edge Cases: Snap Layouts conflict (#288-295)
- Win11 Edge Cases: Virtual Desktops (#281-287)

### Medium Priority
- Special Windows: UAC elevated (#336-345)
- Special Windows: UWP apps (#346-353)
- Accessibility Testing (#311-335)
- Hardware/Display (#379-408)

### Lower Priority
- Software Conflicts (#409-437)
- Input Methods (#458-477)
- Future Features (#516-525)
- ARM64 support (#302-307)

---

*Last Updated: January 25, 2026*
