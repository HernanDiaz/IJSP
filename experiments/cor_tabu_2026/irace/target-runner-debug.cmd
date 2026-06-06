@echo off
REM Debug version of target-runner.cmd
SET "SCRIPT=%~f0"
SET "SCRIPT=%SCRIPT:\=/%"
SET "SCRIPT=%SCRIPT:C:=/mnt/c%"
SET "SCRIPT=%SCRIPT:-debug.cmd=.sh%"

echo DEBUG: Script=%SCRIPT% >> C:\Temp\irace_debug.log 2>&1
echo DEBUG: Args=%* >> C:\Temp\irace_debug.log 2>&1
echo DEBUG: IRACE_NEIGHBOURHOOD=%IRACE_NEIGHBOURHOOD% >> C:\Temp\irace_debug.log 2>&1
echo DEBUG: PATH=%PATH:~0,100% >> C:\Temp\irace_debug.log 2>&1

wsl bash "%SCRIPT%" %* < nul >> C:\Temp\irace_debug.log 2>&1
SET EXITCODE=%ERRORLEVEL%
echo DEBUG: WSL exit code=%EXITCODE% >> C:\Temp\irace_debug.log 2>&1

type C:\Temp\irace_debug.log
EXIT /B %EXITCODE%
