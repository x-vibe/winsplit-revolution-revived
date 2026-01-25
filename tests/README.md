# WinSplit Revolution - Test Suite & Security Validation Plan

## Overview

Comprehensive test suite using industry-standard frameworks to validate functionality, security, and Windows 11 compatibility.

---

## Frameworks Used

| Framework | Purpose | Source |
|-----------|---------|--------|
| **Google Test** | Security & stress tests | [github.com/google/googletest](https://github.com/google/googletest) |
| **Catch2** | Functional tests | [github.com/catchorg/Catch2](https://github.com/catchorg/Catch2) |
| **Dr. Memory** | Memory leak detection | [github.com/DynamoRIO/drmemory](https://github.com/DynamoRIO/drmemory) |
| **PE-bear** | PE analysis | [github.com/hasherezade/pe-bear](https://github.com/hasherezade/pe-bear) |
| **Dependencies** | DLL analysis | [github.com/lucasg/Dependencies](https://github.com/lucasg/Dependencies) |

---

## Quick Start

### Option 1: CMake Build (Recommended)
```cmd
cd tests
build_cmake.cmd
cd build
ctest --output-on-failure
```

### Option 2: Manual Build (VS Developer Prompt)
```cmd
cd tests
build.cmd
run_all_tests.cmd
```

### Option 3: Full Pentest Suite
```cmd
cd tests\pentest
setup_pentest_tools.cmd
run_pentest.cmd
```

---

## Test Suite Structure

```
tests/
├── README.md                    # Test documentation
├── build.cmd                    # Build all test tools
├── run_all_tests.cmd            # Execute full test suite
├── tests.vcxproj                # Visual Studio project
│
├── functional/                  # Functionality tests
│   ├── test_window_positioning.cpp
│   ├── test_hotkey_registration.cpp
│   ├── test_multimonitor.cpp
│   ├── test_dpi_awareness.cpp
│   └── test_drag_n_go.cpp
│
├── security/                    # Security tests
│   ├── test_hook_message_spoofing.cpp
│   ├── test_dll_hijacking.cpp
│   ├── test_xml_injection.cpp
│   └── test_process_access.cpp
│
├── stress/                      # Stress/stability tests
│   ├── test_rapid_hotkeys.cpp
│   └── test_memory_leaks.cpp
│
└── tools/                       # Test utilities
    ├── mock_window.cpp          # Create test windows
    ├── message_spoofer.cpp      # Security test tool
    └── test_harness.h           # Common test framework
```

---

## Security Findings Summary

### Critical Issues Found
| Issue | Location | Risk |
|-------|----------|------|
| Plain HTTP Updates | `update_thread.cpp:36` | MITM attack possible |
| Hook Message Spoofing | `frame_hook.cpp` | Any process can trigger actions |

### High Priority Issues
| Issue | Location | Risk |
|-------|----------|------|
| DLL Hijacking | `frame_hook.cpp:44-46` | Malicious DLL injection |
| PROCESS_ALL_ACCESS | `functions_special.cpp:78` | Excessive permissions |

### Medium Priority Issues
| Issue | Location | Risk |
|-------|----------|------|
| XML No Bounds Check | `settingsmanager.cpp:717` | Integer overflow |
| Negative Array Index | `frame_hook.cpp:202-205` | Potential crash |

---

## Phase 1: Test Utilities (`tests/tools/`)

### 1.1 Test Harness (`test_harness.h`)
Common testing framework with assertions and reporting.

### 1.2 Mock Window Tool (`mock_window.cpp`)
Creates test windows with specific properties.

### 1.3 Message Spoofer (`message_spoofer.cpp`)
**SECURITY TEST** - Sends spoofed messages to validate IPC security.

---

## Phase 2: Security Tests (`tests/security/`)

### 2.1 Hook Message Spoofing Test (CRITICAL)
**File:** `test_hook_message_spoofing.cpp`

**Tests:**
- Send spoofed WSM_STARTMOVING/STOPMOVING messages
- Send extreme wheel values (negative, INT_MAX)
- Verify WinSplit handles gracefully (no crash, no action)

### 2.2 DLL Hijacking Test (HIGH)
**File:** `test_dll_hijacking.cpp`

**Tests:**
- Verify DLL loads from application directory only
- Test with fake DLL in PATH

### 2.3 XML Injection Test (MEDIUM)
**File:** `test_xml_injection.cpp`

**Tests:**
- XXE injection attempts
- Integer overflow values in settings
- Malformed XML handling

### 2.4 Process Access Test
**File:** `test_process_access.cpp`

**Tests:**
- Verify minimal required permissions
- Test with protected processes

---

## Phase 3: Functional Tests (`tests/functional/`)

### 3.1 Window Positioning Tests
| Test | Pass Criteria |
|------|---------------|
| Basic snap (Ctrl+Alt+Numpad) | Window at exact grid position |
| DWM frame compensation | Visible bounds match target |
| Cycle through positions | All layouts work correctly |

### 3.2 Multi-Monitor Tests
| Test | Pass Criteria |
|------|---------------|
| Cross-monitor move | Window moves to correct monitor |
| Frame compensation on move | No offset on destination |
| Different resolutions | Proportional sizing works |

### 3.3 DPI Awareness Tests
| Test | Pass Criteria |
|------|---------------|
| High DPI (150%, 200%) | Pixel-perfect positioning |
| Mixed DPI monitors | Correct scaling on move |

### 3.4 Drag'n'Go Tests
| Test | Pass Criteria |
|------|---------------|
| Preview accuracy | Preview rect == final rect |
| Wheel scroll | Correct option selected |

---

## Phase 4: Stress Tests (`tests/stress/`)

### 4.1 Rapid Hotkey Test
- Fire 1000 hotkeys rapidly
- Verify no crashes or memory leaks

### 4.2 Memory Leak Test
- Extended runtime monitoring
- Check for handle leaks

---

## Implementation Priority

| Priority | Component | Description |
|----------|-----------|-------------|
| P0 | test_harness.h | Common test framework |
| P0 | mock_window.cpp | Window creation utility |
| P0 | test_hook_message_spoofing.cpp | Critical security test |
| P1 | test_window_positioning.cpp | Core functionality |
| P1 | test_dll_hijacking.cpp | Security validation |
| P1 | test_xml_injection.cpp | Settings security |
| P2 | test_multimonitor.cpp | Win11 functionality |
| P2 | test_dpi_awareness.cpp | Win11 compatibility |
| P3 | test_rapid_hotkeys.cpp | Stability |

---

## Files to Create

| File | Purpose |
|------|---------|
| `tests/README.md` | Test documentation |
| `tests/build.cmd` | Build script |
| `tests/run_all_tests.cmd` | Test runner |
| `tests/tools/test_harness.h` | Test framework |
| `tests/tools/mock_window.cpp` | Window utility |
| `tests/tools/message_spoofer.cpp` | Security tool |
| `tests/security/test_hook_message_spoofing.cpp` | IPC security test |
| `tests/security/test_dll_hijacking.cpp` | DLL security test |
| `tests/security/test_xml_injection.cpp` | XML security test |
| `tests/functional/test_window_positioning.cpp` | Position tests |
| `tests/functional/test_multimonitor.cpp` | Multi-monitor tests |
| `tests/functional/test_dpi_awareness.cpp` | DPI tests |

---

## Test Execution

### On Windows (target environment)
```cmd
cd tests
build.cmd
run_all_tests.cmd > results.log
```

### Expected Output
```
=== WinSplit Revolution Test Suite ===
[PASS] Security: Hook message spoofing rejected
[PASS] Security: DLL loads from app directory only
[PASS] Security: XML injection handled safely
[PASS] Functional: Window positioning accurate
[PASS] Functional: Multi-monitor move works
[PASS] Functional: DPI awareness correct
[PASS] Stress: Rapid hotkeys stable
=== All tests passed ===
```

---

## Verification Checklist

Before release:
- [ ] All security tests pass
- [ ] All functional tests pass
- [ ] No memory leaks in stress test
- [ ] Works on Windows 10 (regression)
- [ ] Works on Windows 11 (target)
- [ ] Works with high DPI
- [ ] Works with multiple monitors
