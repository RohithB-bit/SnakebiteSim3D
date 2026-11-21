@echo off
REM ============================================================
REM  build.bat - 3D Interactive Snakebite Emergency Simulator
REM  Requires: MSYS2 MinGW-w64 (C:\msys64\mingw64)
REM ============================================================

SET MINGW=C:\msys64\mingw64\bin
SET PATH=%MINGW%;%PATH%

echo [BUILD] Compiling SnakebiteSim3D...
g++ -O2 -std=c++17 -static-libgcc -static-libstdc++ main.cpp -o SnakebiteSim3D.exe -lfreeglut -lglu32 -lopengl32 -lwinmm -lgdi32

IF ERRORLEVEL 1 (
    echo [ERROR] Compilation failed!
    pause
    exit /b 1
)

echo [OK] Build successful!

REM Ensure runtime DLLs are next to exe
IF NOT EXIST libfreeglut.dll copy "%MINGW%\libfreeglut.dll" libfreeglut.dll >nul 2>&1
IF NOT EXIST libwinpthread-1.dll copy "%MINGW%\libwinpthread-1.dll" libwinpthread-1.dll >nul 2>&1
IF NOT EXIST libgcc_s_seh-1.dll copy "%MINGW%\libgcc_s_seh-1.dll" libgcc_s_seh-1.dll >nul 2>&1
IF NOT EXIST libstdc++-6.dll copy "%MINGW%\libstdc++-6.dll" libstdc++-6.dll >nul 2>&1

echo [RUN] Launching SnakebiteSim3D.exe...
echo       Controls: WASD Move, Mouse Drag Orbit, 1-4 Actions, ESC Quit
start "" SnakebiteSim3D.exe
