$ErrorActionPreference = "Stop"
$DIR = Split-Path -Parent $MyInvocation.MyCommand.Path
$RSCRIPT = "C:\Program Files\R\R-4.5.3\bin\Rscript.exe"
Set-Location $DIR
& $RSCRIPT -e "library(irace); irace_main(scenario = readScenario('scenario.txt'))" 2>&1 | Tee-Object -FilePath "irace_run.out"
Write-Host "irace abcp finished. Results in irace.log (Rdata)"
