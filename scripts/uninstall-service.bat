@echo off
REM ============================================================
REM  uninstall-service.bat — Desinstala fraud-engine
REM  Ejecutar como Administrador
REM ============================================================
setlocal

set SERVICE_NAME=FraudEngine

echo [*] Deteniendo servicio: %SERVICE_NAME%
sc stop "%SERVICE_NAME%" >nul 2>&1
timeout /t 2 >nul

echo [*] Eliminando servicio...
sc delete "%SERVICE_NAME%"

echo.
echo [OK] Servicio eliminado.
pause
