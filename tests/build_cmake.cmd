@echo off
REM WinSplit Revolution Test Suite - CMake Build
REM Uses Google Test and Catch2 from GitHub

setlocal enabledelayedexpansion

echo.
echo ============================================================
echo  WinSplit Test Suite - CMake Build
echo ============================================================
echo.
echo This build method automatically downloads:
echo   - Google Test (industry-standard C++ testing)
echo   - Catch2 (modern C++ test framework)
echo.

REM Check for CMake
where cmake >nul 2>&1
if errorlevel 1 (
    echo ERROR: CMake not found!
    echo.
    echo Install CMake from: https://cmake.org/download/
    echo Or use: winget install Kitware.CMake
    echo.
    exit /b 1
)

REM Check for Ninja or use default generator
set GENERATOR=
where ninja >nul 2>&1
if not errorlevel 1 (
    set GENERATOR=-G Ninja
    echo Using Ninja generator
) else (
    echo Using default Visual Studio generator
)

REM Create build directory
if not exist "build" mkdir build
cd build

REM Configure
echo.
echo Configuring with CMake...
cmake .. %GENERATOR% -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 (
    echo CMake configuration failed!
    cd ..
    exit /b 1
)

REM Build
echo.
echo Building tests...
cmake --build . --config Release
if errorlevel 1 (
    echo Build failed!
    cd ..
    exit /b 1
)

cd ..

echo.
echo ============================================================
echo  Build Complete
echo ============================================================
echo.
echo Run tests with:
echo   cd build
echo   ctest --output-on-failure
echo.
echo Or run individual test executables:
echo   build\security_tests.exe
echo   build\functional_tests.exe
echo   build\stress_tests.exe
echo.

endlocal
