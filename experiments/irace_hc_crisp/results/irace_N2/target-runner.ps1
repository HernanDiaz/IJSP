# target-runner.ps1 for HC irace tuning — CRISP instances
# Calls target-runner.sh via WSL.
# Windows-side timeout = IRACE_PS_TIMEOUT env var (ms), default 1100000 (1100s).
# N3 HC on large instances needs up to 900s; 1100s gives a safety margin.

param([Parameter(ValueFromRemainingArguments=$true)] [string[]]$PassArgs)

# Derive the .sh path from this script's location
$ShScriptWin = Join-Path $PSScriptRoot "target-runner.sh"

# Convert Windows path to WSL path
$ShScript = (wsl wslpath -a ($ShScriptWin -replace '\\', '/')).Trim()

# Build argument string for wsl.exe
$QuotedArgs = $PassArgs | ForEach-Object {
    if ($_.Length -eq 0) { '""' }
    elseif ($_ -match '[ \t]') { '"' + $_ + '"' }
    else { $_ }
}

# Pass IRACE_TIMEOUT explicitly to WSL environment
$iraceTimeout = $env:IRACE_TIMEOUT
if (-not $iraceTimeout -or $iraceTimeout -le 0) { $iraceTimeout = '900' }
$WslArgStr = 'env IRACE_TIMEOUT=' + $iraceTimeout + ' bash "' + $ShScript + '" ' + ($QuotedArgs -join ' ')

$psi = New-Object System.Diagnostics.ProcessStartInfo
$psi.FileName = 'wsl'
$psi.Arguments = $WslArgStr
$psi.UseShellExecute = $false
$psi.CreateNoWindow = $true
$psi.RedirectStandardInput = $true
$psi.RedirectStandardOutput = $true
$psi.RedirectStandardError = $true

$proc = [System.Diagnostics.Process]::Start($psi)
$proc.StandardInput.Close()

$stdoutTask = $proc.StandardOutput.ReadToEndAsync()
$stderrTask = $proc.StandardError.ReadToEndAsync()

# Windows-side timeout: default 1100s (900s inner + 200s safety margin)
$psTimeout = [int]($env:IRACE_PS_TIMEOUT)
if ($psTimeout -le 0) { $psTimeout = 1100000 }

if (-not $proc.WaitForExit($psTimeout)) {
    & taskkill /F /T /PID $proc.Id 2>&1 | Out-Null
    [Console]::WriteLine("Inf")
    exit 0
}

$stdout = $stdoutTask.GetAwaiter().GetResult()
$stderrTask.GetAwaiter().GetResult() | Out-Null
[Console]::Write($stdout)

exit $proc.ExitCode
