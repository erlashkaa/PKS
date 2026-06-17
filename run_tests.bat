@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion
set "PATH=%PATH%;C:\msys64\mingw64\bin"

echo ============================================================
echo   Build and Run Network Scenarios (Ninja + CMake)
echo ============================================================

where cmake >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] cmake not found in PATH or in C:\msys64\mingw64\bin
    pause
    exit /b 1
)

echo [1/3] Configuring CMake...
cmake -S . -B build -G Ninja -DCMAKE_MAKE_PROGRAM=C:/msys64/mingw64/bin/ninja.exe -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
copy build\compile_commands.json . >nul
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] CMake configuration failed!
    pause
    exit /b 1
)

echo [2/3] Building scenarios...
cmake --build build
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Build failed!
    pause
    exit /b 1
)

echo [3/3] Running tests...
echo ============================================================
echo   Running core unit tests...
echo ============================================================
set "FAILED_TESTS=0"

build\tests\unit_tests.exe
if %ERRORLEVEL% NEQ 0 set /a FAILED_TESTS+=1

echo ============================================================
echo   Running integration scenarios...
echo ============================================================

build\tests\scenario1_simple_transfer.exe --gtest_brief=1
if %ERRORLEVEL% NEQ 0 set /a FAILED_TESTS+=1

build\tests\scenario2_collision.exe --gtest_brief=1
if %ERRORLEVEL% NEQ 0 set /a FAILED_TESTS+=1

build\tests\scenario3_routing.exe --gtest_brief=1
if %ERRORLEVEL% NEQ 0 set /a FAILED_TESTS+=1

build\tests\scenario4_heavy_load.exe --gtest_brief=1
if %ERRORLEVEL% NEQ 0 set /a FAILED_TESTS+=1

build\tests\scenario5_mixed_traffic.exe --gtest_brief=1
if %ERRORLEVEL% NEQ 0 set /a FAILED_TESTS+=1

build\tests\scenario6_network_failure.exe --gtest_brief=1
if %ERRORLEVEL% NEQ 0 set /a FAILED_TESTS+=1

echo ============================================================
if %FAILED_TESTS% NEQ 0 (
    echo [ERROR] Some scenarios failed! (Total: %FAILED_TESTS%)
) else (
    echo [OK] All scenarios passed successfully!
)

pause
