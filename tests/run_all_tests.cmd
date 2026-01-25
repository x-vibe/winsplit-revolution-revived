@echo off
REM WinSplit Revolution Test Suite - Test Runner
REM Runs all tests and generates a report

setlocal enabledelayedexpansion

echo.
echo ============================================================
echo  WinSplit Revolution Test Suite
echo ============================================================
echo  Date: %date% %time%
echo ============================================================
echo.

REM Check if binaries exist
if not exist "bin\test_hook_message_spoofing.exe" (
    echo ERROR: Test binaries not found!
    echo Please run build.cmd first.
    exit /b 1
)

REM Check if WinSplit is running
tasklist /FI "IMAGENAME eq WinSplit.exe" 2>NUL | find /I /N "WinSplit.exe">NUL
if errorlevel 1 (
    echo WARNING: WinSplit.exe is not running!
    echo Some tests require WinSplit to be active.
    echo.
    set /p CONT="Continue anyway? [y/N] "
    if /i not "!CONT!"=="y" exit /b 0
    echo.
)

set PASSED=0
set FAILED=0
set SKIPPED=0

echo.
echo ============================================================
echo  SECURITY TESTS
echo ============================================================

REM Test 1: Hook Message Spoofing
echo.
echo [TEST] Hook Message Spoofing (CRITICAL)
echo --------------------------------------------------------
bin\test_hook_message_spoofing.exe
if errorlevel 1 (
    set /a FAILED+=1
    echo RESULT: FAILED
) else (
    set /a PASSED+=1
    echo RESULT: PASSED
)

REM Test 2: DLL Hijacking
echo.
echo [TEST] DLL Hijacking (HIGH)
echo --------------------------------------------------------
bin\test_dll_hijacking.exe
if errorlevel 1 (
    set /a FAILED+=1
    echo RESULT: FAILED
) else (
    set /a PASSED+=1
    echo RESULT: PASSED
)

REM Test 3: XML Injection
echo.
echo [TEST] XML Injection (MEDIUM)
echo --------------------------------------------------------
bin\test_xml_injection.exe
if errorlevel 1 (
    set /a FAILED+=1
    echo RESULT: FAILED
) else (
    set /a PASSED+=1
    echo RESULT: PASSED
)

echo.
echo ============================================================
echo  FUNCTIONAL TESTS
echo ============================================================

REM Test 4: Window Positioning
echo.
echo [TEST] Window Positioning
echo --------------------------------------------------------
bin\test_window_positioning.exe
if errorlevel 1 (
    set /a FAILED+=1
    echo RESULT: FAILED
) else (
    set /a PASSED+=1
    echo RESULT: PASSED
)

REM Test 5: Multi-Monitor
echo.
echo [TEST] Multi-Monitor Support
echo --------------------------------------------------------
bin\test_multimonitor.exe
if errorlevel 1 (
    set /a FAILED+=1
    echo RESULT: FAILED
) else (
    set /a PASSED+=1
    echo RESULT: PASSED
)

REM Test 6: DPI Awareness
echo.
echo [TEST] DPI Awareness
echo --------------------------------------------------------
bin\test_dpi_awareness.exe
if errorlevel 1 (
    set /a FAILED+=1
    echo RESULT: FAILED
) else (
    set /a PASSED+=1
    echo RESULT: PASSED
)

echo.
echo ============================================================
echo  TEST SUMMARY
echo ============================================================
echo.
echo  Passed:  %PASSED%
echo  Failed:  %FAILED%
echo  Skipped: %SKIPPED%
echo.

set /a TOTAL=%PASSED%+%FAILED%+%SKIPPED%
echo  Total:   %TOTAL% tests
echo.

if %FAILED% EQU 0 (
    echo  *** ALL TESTS PASSED ***
    echo.
    exit /b 0
) else (
    echo  *** SOME TESTS FAILED ***
    echo.
    exit /b 1
)

endlocal
