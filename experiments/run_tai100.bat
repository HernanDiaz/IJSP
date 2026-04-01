@echo off
title IJSP Experiments - tai100
echo Lanzando experimentos IJSP tai100...
echo Los resultados se guardan en run_tai100.log
echo NO cierres esta ventana mientras corran los experimentos.
echo.
cd /d "%~dp0"
"C:\Program Files\Git\bin\bash.exe" -c "cd '/c/Users/diazhernan/CLionProjects/IJSP/experiments' && bash run_tai100.sh >> run_tai100.log 2>&1"
echo.
echo *** Experimentos tai100 finalizados ***
pause
