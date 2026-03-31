@echo off
title Estado Experimentos IJSP

:loop
cls
echo ============================================
echo  ESTADO EXPERIMENTOS IJSP - %date% %time%
echo ============================================
echo.

powershell -NoProfile -Command ^
  "$results = 'C:\Users\diazhernan\CLionProjects\IJSP\experiments\results'; " ^
  "$activos = (Get-Process FuzzyFW -ErrorAction SilentlyContinue | Measure-Object).Count; " ^
  "Write-Host ('Procesos FuzzyFW activos: ' + $activos); " ^
  "Write-Host ''; " ^
  "$total = 0; $maxTotal = 0; " ^
  "foreach ($nb in @('n1','n2','n3','nh','next')) { " ^
  "  $header = $false; " ^
  "  foreach ($comp in @('EV','LEX1','LEX2','YX')) { " ^
  "    $dir = Join-Path $results ('{0}_{1}' -f $nb, $comp); " ^
  "    if (-not (Test-Path $dir)) { " ^
  "      if (-not $header) { Write-Host ('{0}:' -f $nb.ToUpper()); $header = $true; } " ^
  "      Write-Host ('  {0}_{1}:  (sin directorio)' -f $nb, $comp); " ^
  "      $maxTotal += 82; " ^
  "      continue; " ^
  "    } " ^
  "    $done = (Get-ChildItem $dir -Filter '*.csv' -ErrorAction SilentlyContinue | " ^
  "      Where-Object { $_.Name -match '_\d{14}\.csv$' } | " ^
  "      Select-Object -ExpandProperty BaseName | " ^
  "      ForEach-Object { $_ -replace '_\d{14}$','' } | " ^
  "      Sort-Object -Unique | Measure-Object).Count; " ^
  "    $total += $done; $maxTotal += 82; " ^
  "    if (-not $header) { Write-Host ('{0}:' -f $nb.ToUpper()); $header = $true; } " ^
  "    $bar = '[' + ('#' * [math]::Floor($done * 20 / 82)) + (' ' * (20 - [math]::Floor($done * 20 / 82))) + ']'; " ^
  "    $pct = [math]::Round($done * 100 / 82, 0); " ^
  "    $status = if ($done -eq 82) { ' COMPLETO' } elseif ($done -eq 0) { ' pendiente' } else { '' }; " ^
  "    Write-Host ('  {0}_{1}: {2} {3,3}%% ({4}/82){5}' -f $nb, $comp, $bar, $pct, $done, $status); " ^
  "  } " ^
  "  Write-Host ''; " ^
  "} " ^
  "$pct = [math]::Round($total * 100 / $maxTotal, 1); " ^
  "$globalBar = '[' + ('#' * [math]::Floor($total * 40 / $maxTotal)) + (' ' * (40 - [math]::Floor($total * 40 / 82*4*5))) + ']'; " ^
  "Write-Host ('TOTAL: ' + $total + ' / ' + $maxTotal + ' (' + $pct + '%%)'); " ^
  "Write-Host ''; " ^
  "$logFile = 'C:\Users\diazhernan\CLionProjects\IJSP\experiments\run_experiments.log'; " ^
  "if (Test-Path $logFile) { " ^
  "  Write-Host '--- Ultimas lineas del log ---'; " ^
  "  Get-Content $logFile -Tail 6; " ^
  "}"

echo.
echo Actualizando en 30 segundos... (Ctrl+C para salir)
timeout /t 30 /nobreak >nul
goto loop
