@echo off
REM WinSplit Revolution Test Suite - Build Script
REM Requires Visual Studio or Build Tools for C++

setlocal enabledelayedexpansion

echo.
echo ============================================
echo  WinSplit Revolution Test Suite - Builder
echo ============================================
echo.

REM Check for Visual Studio environment
if "%VCINSTALLDIR%"=="" (
    echo Looking for Visual Studio...

    REM Use vswhere to find latest VS installation
    set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
    if exist "!VSWHERE!" (
        for /f "usebackq tokens=*" %%i in (`"!VSWHERE!" -latest -products * -property installationPath`) do (
            if exist "%%i\VC\Auxiliary\Build\vcvars64.bat" (
                call "%%i\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
                echo Found Visual Studio at: %%i
            )
        )
    )

    REM Fallback: Try known VS paths directly (newest first)
    if "%VCINSTALLDIR%"=="" (
        if exist "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" (
            call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
            echo Found VS 18 Community
        ) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
            call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
            echo Found VS2022 Community
        ) else (
            echo ERROR: Visual Studio not found!
            echo Please run from a Developer Command Prompt
            echo or install Visual Studio with C++ workload.
            exit /b 1
        )
    )
)

REM Create output directory
if not exist "bin" mkdir bin

echo.
echo Building test utilities...
echo --------------------------

REM Build mock_window.exe
echo [1/7] Building mock_window.exe...
cl /nologo /EHsc /W3 /O2 /Fe:bin\mock_window.exe tools\mock_window.cpp ^
    user32.lib kernel32.lib /link /SUBSYSTEM:CONSOLE
if errorlevel 1 (
    echo FAILED: mock_window.exe
    set ERRORS=1
) else (
    echo OK: mock_window.exe
)

REM Build message_spoofer.exe
echo [2/7] Building message_spoofer.exe...
cl /nologo /EHsc /W3 /O2 /Fe:bin\message_spoofer.exe tools\message_spoofer.cpp ^
    user32.lib kernel32.lib /link /SUBSYSTEM:CONSOLE
if errorlevel 1 (
    echo FAILED: message_spoofer.exe
    set ERRORS=1
) else (
    echo OK: message_spoofer.exe
)

echo.
echo Building security tests...
echo --------------------------

REM Build test_hook_message_spoofing.exe
echo [3/7] Building test_hook_message_spoofing.exe...
cl /nologo /EHsc /W3 /O2 /I. /Fe:bin\test_hook_message_spoofing.exe ^
    security\test_hook_message_spoofing.cpp ^
    user32.lib kernel32.lib /link /SUBSYSTEM:CONSOLE
if errorlevel 1 (
    echo FAILED: test_hook_message_spoofing.exe
    set ERRORS=1
) else (
    echo OK: test_hook_message_spoofing.exe
)

REM Build test_dll_hijacking.exe
echo [4/7] Building test_dll_hijacking.exe...
cl /nologo /EHsc /W3 /O2 /I. /Fe:bin\test_dll_hijacking.exe ^
    security\test_dll_hijacking.cpp ^
    user32.lib kernel32.lib shlwapi.lib psapi.lib /link /SUBSYSTEM:CONSOLE
if errorlevel 1 (
    echo FAILED: test_dll_hijacking.exe
    set ERRORS=1
) else (
    echo OK: test_dll_hijacking.exe
)

REM Build test_xml_injection.exe
echo [5/7] Building test_xml_injection.exe...
cl /nologo /EHsc /W3 /O2 /I. /Fe:bin\test_xml_injection.exe ^
    security\test_xml_injection.cpp ^
    user32.lib kernel32.lib shell32.lib /link /SUBSYSTEM:CONSOLE
if errorlevel 1 (
    echo FAILED: test_xml_injection.exe
    set ERRORS=1
) else (
    echo OK: test_xml_injection.exe
)

echo.
echo Building functional tests...
echo ----------------------------

REM Build test_window_positioning.exe
echo [6/7] Building test_window_positioning.exe...
cl /nologo /EHsc /W3 /O2 /I. /Fe:bin\test_window_positioning.exe ^
    functional\test_window_positioning.cpp ^
    user32.lib kernel32.lib dwmapi.lib /link /SUBSYSTEM:CONSOLE
if errorlevel 1 (
    echo FAILED: test_window_positioning.exe
    set ERRORS=1
) else (
    echo OK: test_window_positioning.exe
)

REM Build test_multimonitor.exe
echo [6b/7] Building test_multimonitor.exe...
cl /nologo /EHsc /W3 /O2 /I. /Fe:bin\test_multimonitor.exe ^
    functional\test_multimonitor.cpp ^
    user32.lib kernel32.lib dwmapi.lib /link /SUBSYSTEM:CONSOLE
if errorlevel 1 (
    echo FAILED: test_multimonitor.exe
    set ERRORS=1
) else (
    echo OK: test_multimonitor.exe
)

REM Build test_dpi_awareness.exe
echo [7/7] Building test_dpi_awareness.exe...
cl /nologo /EHsc /W3 /O2 /I. /Fe:bin\test_dpi_awareness.exe ^
    functional\test_dpi_awareness.cpp ^
    user32.lib kernel32.lib dwmapi.lib shcore.lib /link /SUBSYSTEM:CONSOLE
if errorlevel 1 (
    echo FAILED: test_dpi_awareness.exe
    set ERRORS=1
) else (
    echo OK: test_dpi_awareness.exe
)

REM Cleanup object files
del *.obj 2>nul

echo.
echo ============================================
if defined ERRORS (
    echo Build completed with ERRORS
    exit /b 1
) else (
    echo Build completed successfully!
    echo.
    echo Executables are in: %CD%\bin\
    echo.
    echo Run tests with: run_all_tests.cmd
)
echo ============================================
echo.

endlocal
