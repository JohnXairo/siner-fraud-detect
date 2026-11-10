@echo off
REM ============================================================
REM  build-release.bat — Compila fraud-engine.exe en Release
REM  Requiere: Visual Studio 2022 Build Tools, CMake, vcpkg en C:\vcpkg
REM ============================================================
setlocal

set VCPKG_ROOT=C:\vcpkg
set BACKEND_DIR=%~dp0..\backend

echo [*] Configurando CMake...
cmake -B "%BACKEND_DIR%\build" -S "%BACKEND_DIR%" ^
    -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" ^
    -DVCPKG_TARGET_TRIPLET=x64-windows ^
    -DCMAKE_BUILD_TYPE=Release

if %errorlevel% neq 0 (
    echo [ERROR] Fallo la configuracion de CMake.
    pause & exit /b 1
)

echo [*] Compilando...
cmake --build "%BACKEND_DIR%\build" --config Release

if %errorlevel% neq 0 (
    echo [ERROR] Fallo la compilacion.
    pause & exit /b 1
)

echo.
echo [OK] Compilado: %BACKEND_DIR%\build\Release\fraud-engine.exe
echo      Para probar: fraud-engine.exe --console
echo.
pause
