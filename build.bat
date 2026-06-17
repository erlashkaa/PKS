@echo off
chcp 65001 >nul
REM ============================================================
REM build.bat - build script for network simulator
REM ============================================================

setlocal enabledelayedexpansion
set "PATH=%PATH%;C:\msys64\mingw64\bin"
echo ============================================================
echo  Network Simulator Build Script
echo ============================================================
echo.

REM --- Collect all .cpp files ---
set SOURCES=src\Packet.cpp src\Node.cpp src\Host.cpp src\Router.cpp ^
            src\CSMACDMedium.cpp src\StatisticsCollector.cpp ^
            src\TrafficStrategies.cpp src\PoissonStrategy.cpp ^
            src\NetworkVisualizer.cpp src\NodeFactory.cpp ^
            src\SimulationFacade.cpp src\main.cpp

set INCLUDES=-Iinclude
set OUTPUT=network_simulator.exe
set FLAGS=-std=c++17 -Wall -Wextra -O2

REM ============================================================
REM Attempt 1: g++ (MinGW / MSYS2 / Chocolatey)
REM ============================================================
where g++ >nul 2>&1
if %ERRORLEVEL% == 0 (
    echo [BUILD] Found g++. Compiling...
    g++ %FLAGS% %INCLUDES% %SOURCES% -o %OUTPUT%
    if %ERRORLEVEL% == 0 (
        echo.
        echo [OK] Build successful! Running: %OUTPUT%
        echo.
        %OUTPUT%
    ) else (
        echo [ERROR] g++ compilation error.
    )
    goto :end
)

REM ============================================================
REM Attempt 2: CMake + Ninja
REM ============================================================
where cmake >nul 2>&1
if %ERRORLEVEL% == 0 (
    where ninja >nul 2>&1
    if %ERRORLEVEL% == 0 (
        echo [BUILD] Found cmake + ninja. Configuring...
        cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
        cmake --build build
        if %ERRORLEVEL% == 0 (
            echo.
            echo [OK] Build successful!
            build\%OUTPUT%
        )
        goto :end
    )
)

REM ============================================================
REM Attempt 3: Visual Studio / MSVC via vswhere
REM ============================================================
set VSWHERE="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if exist %VSWHERE% (
    for /f "tokens=*" %%i in ('%VSWHERE% -latest -find VC\Auxiliary\Build\vcvarsall.bat') do set VCVARS=%%i
    if not defined VCVARS (
        for /f "tokens=*" %%i in ('%VSWHERE% -latest -find VC\Auxiliary\Build\vcvars64.bat') do set VCVARS=%%i
        set VCVARS_ARG=
    ) else (
        set VCVARS_ARG=x64
    )
)

if defined VCVARS (
    echo [BUILD] Found MSVC: !VCVARS!
    call "!VCVARS!" !VCVARS_ARG!
    cl /std:c++17 /EHsc /O2 /Iinclude %SOURCES% /Fe:%OUTPUT%
    if %ERRORLEVEL% == 0 (
        echo.
        echo [OK] Build successful! Running...
        %OUTPUT%
    ) else (
        echo [ERROR] MSVC compilation error.
    )
    goto :end
)

REM ============================================================
REM Nothing found
REM ============================================================
echo.
echo [ERROR] Compiler not found!
echo.
echo Install one of:
echo   1. MinGW-w64 (g++):
echo      https://winlibs.com/
echo      Or via Chocolatey: choco install mingw
echo.
echo   2. Visual Studio 2019/2022 (Community - free):
echo      https://visualstudio.microsoft.com/
echo.
echo   3. CMake + Ninja:
echo      https://cmake.org/download/
echo      https://github.com/ninja-build/ninja/releases
echo.

:end
endlocal
pause
