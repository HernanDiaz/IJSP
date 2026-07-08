@echo off
REM Windows wrapper for irace (Windows R): forwards all args to the WSL
REM bash target-runner, which runs the full ladder pipeline and prints -HV.
wsl bash /mnt/c/Users/diazhernan/CLionProjects/Fable/IJSP/experiments/mo_green_2026/irace_ladder/target-runner.sh %*
EXIT /B %ERRORLEVEL%
