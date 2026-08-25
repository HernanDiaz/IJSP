# VIGILANTE de la Fase 2C: registra estado cada 30 s y, si la memoria libre baja de
# un umbral critico, DETIENE la tanda (wsl --terminate) para evitar un cuelgue duro.
# El runner es idempotente a nivel de trozo, asi que parar cuesta minutos, no horas.
# Uso:  powershell -ExecutionPolicy Bypass -File final\guard.ps1
$log   = "$PSScriptRoot\guard.csv"
$UMBRAL = 1.5   # GB libres: por debajo de esto se corta
if (-not (Test-Path $log)) { "ts,free_GB,workers,worker_RAM_GB,max_worker_MB,accion" | Out-File $log -Encoding utf8 }
while ($true) {
  try {
    $os = Get-CimInstance Win32_OperatingSystem
    $free = [math]::Round($os.FreePhysicalMemory/1MB,2)
    $p = Get-Process FuzzyFW -ErrorAction SilentlyContinue
    $n = if ($p) { $p.Count } else { 0 }
    $sum = if ($p) { [math]::Round(($p | Measure-Object WorkingSet64 -Sum).Sum/1GB,2) } else { 0 }
    $mx  = if ($p) { [math]::Round(($p | Measure-Object WorkingSet64 -Maximum).Maximum/1MB) } else { 0 }
    $acc = "ok"
    if ($free -lt $UMBRAL -and $n -gt 0) {
      $acc = "CORTE_POR_MEMORIA"
      wsl --terminate Ubuntu 2>$null
    }
    "{0},{1},{2},{3},{4},{5}" -f (Get-Date -Format 'MM-dd_HH:mm:ss'), $free, $n, $sum, $mx, $acc |
      Out-File $log -Append -Encoding utf8
    if ($acc -ne "ok") { break }
  } catch { }
  Start-Sleep -Seconds 30
}
