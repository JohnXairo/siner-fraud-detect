@echo off
REM ============================================================
REM  install-service.bat — Instala fraud-engine como servicio
REM  Ejecutar como Administrador
REM ============================================================
setlocal

set SERVICE_NAME=FraudEngine
set SERVICE_DISP=Sinergy Fraud Detection Engine
set SERVICE_DESC=Motor de deteccion de fraude bancario - Demo OTel
set EXE_PATH=%~dp0..\backend\build\Release\fraud-engine.exe

echo [*] Instalando servicio: %SERVICE_NAME%
echo [*] Ejecutable: %EXE_PATH%

if not exist "%EXE_PATH%" (
    echo [ERROR] No se encontro el ejecutable: %EXE_PATH%
    echo         Compilar primero con: cmake --build backend\build --config Release
    pause
    exit /b 1
)

sc query "%SERVICE_NAME%" >nul 2>&1
if %errorlevel% == 0 (
    echo [*] Servicio ya existe, deteniendolo...
    sc stop "%SERVICE_NAME%" >nul 2>&1
    sc delete "%SERVICE_NAME%" >nul 2>&1
    timeout /t 2 >nul
)

sc create "%SERVICE_NAME%" ^
    binPath= "\"%EXE_PATH%\"" ^
    DisplayName= "%SERVICE_DISP%" ^
    start= auto

sc description "%SERVICE_NAME%" "%SERVICE_DESC%"
sc start "%SERVICE_NAME%"

echo.
echo [OK] Servicio instalado e iniciado.
echo      Puerto: 8081
echo      Health: http://localhost:8081/health
echo.
pause
