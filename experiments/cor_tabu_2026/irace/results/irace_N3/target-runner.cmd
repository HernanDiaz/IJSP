@echo off
REM Windows wrapper: calls target-runner.ps1 which enforces 310s timeout via taskkill /T
SET "DIR=%~dp0"
powershell.exe -NoProfile -NonInteractive -ExecutionPolicy Bypass -File "%DIR%target-runner.ps1" %*
EXIT /B %ERRORLEVEL%
