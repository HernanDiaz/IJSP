@echo off
title IJSP Experiments
echo Lanzando experimentos IJSP...
echo Los resultados se guardan en run_experiments.log
echo NO cierres esta ventana mientras corran los experimentos.
echo.
cd /d "%~dp0"
"C:\Program Files\Git\bin\bash.exe" -c "cd '/c/Users/diazhernan/CLionProjects/IJSP/experiments' && bash run_experiments.sh >> run_experiments.log 2>&1"
echo.
echo *** Experimentos finalizados ***
pause
