@echo off
cd /d "%~dp0"
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0scripts\run-checks.ps1"
set "check_result=%ERRORLEVEL%"
echo.
echo Results are in the results folder. Exit code: %check_result%
pause
exit /b %check_result%
