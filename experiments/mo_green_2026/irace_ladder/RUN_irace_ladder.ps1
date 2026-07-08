# RUN_irace_ladder.ps1 — launch irace (Windows R) for the LADDER arm.
# Run from PowerShell:  powershell -ExecutionPolicy Bypass -File RUN_irace_ladder.ps1
$ErrorActionPreference = "Stop"
$DIR = Split-Path -Parent $MyInvocation.MyCommand.Path
$RSCRIPT = "C:\Program Files\R\R-4.5.3\bin\Rscript.exe"
Set-Location $DIR
# irace reads scenario.txt (targetRunner = target-runner.cmd) from the cwd.
& $RSCRIPT -e "library(irace); irace_main(scenario = readScenario('scenario.txt'))" `
    2>&1 | Tee-Object -FilePath "irace_run.out"
Write-Host "irace finished. Best config in irace.Rdata / irace.log"
