@echo off
:: WinSplit Revolution Build Script
:: Requires: Visual Studio 2022+ with C++ workload, Windows SDK
:: Note (January 2026): Verify latest VS version online before installing

:: IMPORTANT: This script avoids "enabledelayedexpansion" because the project
:: path contains "!" which conflicts with delayed expansion syntax.

echo ============================================
echo  WinSplit Revolution Build Script
echo ============================================
echo.

:: Check if running from Developer Command Prompt, if not auto-detect VS
where cl >nul 2>&1
if %errorlevel% neq 0 (
    echo Looking for Visual Studio...
    call :FindVisualStudio
    if errorlevel 1 exit /b 1
)

:: Verify cl is available
where cl >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: Failed to initialize Visual Studio environment.
    echo Make sure "Desktop development with C++" workload is installed.
    exit /b 1
)
echo.

:: Configuration
set CONFIG=Release
set PLATFORM=x64
set VERSION=10.2.0

echo Configuration: %CONFIG%
echo Platform: %PLATFORM%
echo Version: %VERSION%
echo.

:: Navigate to upstream directory
cd /d "%~dp0upstream"
if %errorlevel% neq 0 (
    echo ERROR: Cannot find upstream directory
    exit /b 1
)

:: Step 1: Build wxWidgets (if not already built)
echo [1/4] Checking wxWidgets...
if exist "wxWidgets\lib\vc_x64_lib\wxmsw32u_core.lib" (
    echo       wxWidgets already built, skipping.
) else (
    echo       Building wxWidgets - this takes a while...
    cd wxWidgets\build\msw
    nmake -f makefile.vc BUILD=release RUNTIME_LIBS=static TARGET_CPU=X64
    if %errorlevel% neq 0 (
        echo ERROR: wxWidgets build failed!
        exit /b 1
    )
    cd /d "%~dp0upstream"
    echo       wxWidgets built successfully.
)
echo.

:: Step 2: Build WinSplit Revolution
echo [2/4] Building WinSplit Revolution...
msbuild "Winsplit Revolution.sln" /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /m
if %errorlevel% neq 0 (
    echo ERROR: Build failed!
    exit /b 1
)
echo       Build successful.
echo.

:: Step 3: Copy to bin directory
echo [3/4] Copying files to bin directory...
if not exist "bin" mkdir "bin"
copy /y "x64\%CONFIG%\Winsplit.exe" "bin\" >nul
copy /y "x64\%CONFIG%\winsplithook.dll" "bin\" >nul
echo       Files copied.
echo.

:: Step 4: Create portable ZIP
echo [4/4] Creating portable ZIP...
cd /d "%~dp0"
if exist "dist" rmdir /s /q "dist"
mkdir "dist\WinSplit Revolution"

:: Copy executables
copy /y "upstream\bin\Winsplit.exe" "dist\WinSplit Revolution\" >nul
copy /y "upstream\bin\winsplithook.dll" "dist\WinSplit Revolution\" >nul

:: Copy resources
xcopy /e /i /q "upstream\bin\images" "dist\WinSplit Revolution\images\" >nul
xcopy /e /i /q "upstream\bin\languages" "dist\WinSplit Revolution\languages\" >nul

:: Copy documentation
copy /y "upstream\README.md" "dist\WinSplit Revolution\" >nul
copy /y "upstream\LICENSE" "dist\WinSplit Revolution\" >nul
copy /y "upstream\CHANGELOG.md" "dist\WinSplit Revolution\" >nul

:: Create portable mode marker
echo This file enables portable mode. Settings will be stored in this directory. > "dist\WinSplit Revolution\portable.txt"

:: Create ZIP using PowerShell
set ZIPNAME=WinSplit-Revolution-v%VERSION%-portable-x64.zip
powershell -Command "Compress-Archive -Path 'dist\WinSplit Revolution' -DestinationPath 'dist\%ZIPNAME%' -Force"

echo       Created: dist\%ZIPNAME%
echo.

echo ============================================
echo  Build Complete!
echo ============================================
echo.
echo Output files:
echo   - upstream\x64\%CONFIG%\Winsplit.exe
echo   - upstream\x64\%CONFIG%\winsplithook.dll
echo   - dist\%ZIPNAME%
echo.
echo To create installer, install NSIS and run:
echo   cd upstream\nsis
echo   makensis Winsplit-SetupScript.nsi
echo.

exit /b 0

:: ============================================
:: Subroutine: Find Visual Studio
:: ============================================
:FindVisualStudio
setlocal

:: Try vswhere first
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if exist "%VSWHERE%" (
    for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -property installationPath`) do (
        if exist "%%i\VC\Auxiliary\Build\vcvars64.bat" (
            echo Found Visual Studio at: %%i
            endlocal
            call "%%i\VC\Auxiliary\Build\vcvars64.bat" >nul
            exit /b 0
        )
    )
)

:: Fallback: Try known VS paths directly (newest first)
if exist "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" (
    echo Found VS 18 Community
    endlocal
    call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" >nul
    exit /b 0
)
if exist "C:\Program Files\Microsoft Visual Studio\18\Professional\VC\Auxiliary\Build\vcvars64.bat" (
    echo Found VS 18 Professional
    endlocal
    call "C:\Program Files\Microsoft Visual Studio\18\Professional\VC\Auxiliary\Build\vcvars64.bat" >nul
    exit /b 0
)
if exist "C:\Program Files\Microsoft Visual Studio\18\Enterprise\VC\Auxiliary\Build\vcvars64.bat" (
    echo Found VS 18 Enterprise
    endlocal
    call "C:\Program Files\Microsoft Visual Studio\18\Enterprise\VC\Auxiliary\Build\vcvars64.bat" >nul
    exit /b 0
)
if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
    echo Found VS2022 Community
    endlocal
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul
    exit /b 0
)
if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat" (
    echo Found VS2022 Professional
    endlocal
    call "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat" >nul
    exit /b 0
)
if exist "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat" (
    echo Found VS2022 Enterprise
    endlocal
    call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat" >nul
    exit /b 0
)

echo ERROR: Visual Studio not found!
echo.
echo Please install Visual Studio with "Desktop development with C++" workload.
echo Or run from a Developer Command Prompt.
endlocal
exit /b 1
