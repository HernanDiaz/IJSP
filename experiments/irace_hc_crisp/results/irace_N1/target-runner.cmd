@echo off
REM Windows wrapper: calls target-runner.ps1 which calls target-runner.sh via WSL
SET "DIR=%~dp0"
powershell.exe -NoProfile -NonInteractive -ExecutionPolicy Bypass -File "%DIR%target-runner.ps1" %*
EXIT /B %ERRORLEVEL%
