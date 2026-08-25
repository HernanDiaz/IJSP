# "Caja negra": registra cada 20 s el estado de Windows en un CSV para diagnosticar
# POST-MORTEM el proximo cuelgue (el fichero sobrevive al reinicio duro).
# Campos clave:
#   free_GB      -> si cae a ~0 antes del cuelgue: fue MEMORIA
#   mhz          -> si baja de 2401: THROTTLING (indicio termico/alimentacion)
#   load_pct     -> carga de CPU
#   disk_queue   -> si se dispara: saturacion de E/S
#   workers      -> procesos FuzzyFW vivos
# Uso: powershell -ExecutionPolicy Bypass -File final\blackbox.ps1
$log = "$PSScriptRoot\blackbox.csv"
if (-not (Test-Path $log)) {
  "ts,free_GB,used_GB,workers,worker_RAM_GB,mhz,load_pct,disk_queue,top_proc,top_GB" | Out-File $log -Encoding utf8
}
while ($true) {
  try {
    $os = Get-CimInstance Win32_OperatingSystem
    $tot = $os.TotalVisibleMemorySize/1MB; $free = $os.FreePhysicalMemory/1MB
    $cpu = Get-CimInstance Win32_Processor
    $f = Get-Process FuzzyFW -ErrorAction SilentlyContinue
    $n = if ($f) { $f.Count } else { 0 }
    $wram = if ($f) { [math]::Round(($f | Measure-Object WorkingSet64 -Sum).Sum/1GB,2) } else { 0 }
    $top = Get-Process | Where-Object { $_.ProcessName -ne 'FuzzyFW' } |
           Sort-Object WorkingSet64 -Descending | Select-Object -First 1
    $dq = 0
    try { $dq = [math]::Round((Get-Counter '\PhysicalDisk(_Total)\Current Disk Queue Length' -ErrorAction Stop).CounterSamples[0].CookedValue,1) } catch {}
    "{0},{1},{2},{3},{4},{5},{6},{7},{8},{9}" -f (Get-Date -Format 'MM-dd_HH:mm:ss'),
      [math]::Round($free,2), [math]::Round($tot-$free,2), $n, $wram,
      $cpu.CurrentClockSpeed, $cpu.LoadPercentage, $dq,
      $top.ProcessName, [math]::Round($top.WorkingSet64/1GB,2) |
      Out-File $log -Append -Encoding utf8
  } catch { }
  Start-Sleep -Seconds 20
}
