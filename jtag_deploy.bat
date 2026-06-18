@echo off
chcp 65001 >nul
echo ============================================
echo   EES351 Zynq JTAG Deploy Script
echo   Make sure the board is powered ON and
echo   the JTAG cable is connected before running.
echo ============================================
echo.

REM --- 检查 xsct 是否在 PATH 中 ---
where xsct >nul 2>&1
if %errorlevel% neq 0 (
    echo [ERROR] xsct not found in PATH!
    echo Please run this from a Vitis command prompt, or add
    echo Vitis/2023.2/bin/xsct to your PATH.
    pause
    exit /b 1
)

echo [INFO] Running JTAG deploy with xsct...
echo.

xsct jtag_deploy.tcl

echo.
echo ============================================
echo   Done! The application is now running.
echo
echo   To see the SD card log:
echo   1. Let the program run for a while
echo      (STM32 sends data every 1 second)
echo   2. Stop the program in Vitis or power off
echo   3. Remove the SD card and read log.txt on PC
echo ============================================
echo.
pause
