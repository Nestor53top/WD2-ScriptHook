@echo off
echo === WD2 ScriptHook Build Script ===
echo.

if not exist build mkdir build
cd build

echo [1/3] Configuring CMake...
cmake .. -G "Visual Studio 17 2022" -A x64
if errorlevel 1 (
    echo CMake configure failed!
    pause
    exit /b 1
)

echo [2/3] Building...
cmake --build . --config Release
if errorlevel 1 (
    echo Build failed!
    pause
    exit /b 1
)

echo [3/3] Copying output...
if not exist "..\dist" mkdir "..\dist"
copy /Y "bin\Release\core.dll" "..\dist\core.dll"
copy /Y "bin\Release\core.pdb" "..\dist\core.pdb" 2>nul

echo.
echo === Build complete ===
echo Output: dist\core.dll
echo.
pause
