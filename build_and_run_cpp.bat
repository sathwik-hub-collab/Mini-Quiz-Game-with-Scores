@echo off
title Sakura Quiz Platform - C++ Console Arena
color 0D
echo ===================================================================
echo     MINI QUIZ GAME - SAKURA EDITION (C++ Console Engine)
echo ===================================================================
echo.
echo Compiling quiz_game.cpp with MinGW G++...
"C:\Program Files (x86)\Dev-Cpp\MinGW64\bin\g++.exe" -std=c++11 quiz_game.cpp -o quiz_game.exe
if %errorlevel% neq 0 (
    echo [ERROR] Compilation failed.
    pause
    exit /b %errorlevel%
)
echo [OK] Compilation successful!
echo.
echo Starting Sakura Quiz Console Arena...
echo.
quiz_game.exe
pause

