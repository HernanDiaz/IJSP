# target-runner.ps1
# Calls target-runner.sh via WSL.
# Windows-side timeout = IRACE_PS_TIMEOUT env var (ms), default 1100000 (1100s).
# Set IRACE_PS_TIMEOUT=2700000 for N1/N3 (2700s = 2500s bash + 200s margin).

param([Parameter(ValueFromRemainingArguments=$true)] [string[]]$PassArgs)

# Derive the .sh path from this script's location (same directory as .ps1)
$ShScriptWin = Join-Path $PSScriptRoot "target-runner.sh"

# Convert Windows path to WSL path using wslpath (handles 8.3 short names correctly)
$ShScript = (wsl wslpath -a ($ShScriptWin -replace '\\', '/')).Trim()

# Build argument string for wsl.exe, quoting each arg properly
$QuotedArgs = $PassArgs | ForEach-Object {
    if ($_.Length -eq 0) { '""' }
    elseif ($_ -match '[ \t]') { '"' + $_ + '"' }
    else { $_ }
}
# Pass IRACE_TIMEOUT explicitly to WSL environment (env vars are NOT inherited by WSL from PS)
$iraceTimeout = $env:IRACE_TIMEOUT
if (-not $iraceTimeout -or $iraceTimeout -le 0) { $iraceTimeout = '900' }
$WslArgStr = 'env IRACE_TIMEOUT=' + $iraceTimeout + ' bash "' + $ShScript + '" ' + ($QuotedArgs -join ' ')

$psi = New-Object System.Diagnostics.ProcessStartInfo
$psi.FileName = 'wsl'
$psi.Arguments = $WslArgStr
$psi.UseShellExecute = $false
$psi.CreateNoWindow = $true
$psi.RedirectStandardInput = $true    # Prevent WSL stdin blocking
$psi.RedirectStandardOutput = $true   # Capture stdout to pass through
$psi.RedirectStandardError = $true    # Capture stderr to prevent bash job-control messages polluting stdout

$proc = [System.Diagnostics.Process]::Start($psi)
$proc.StandardInput.Close()           # EOF on stdin immediately

# Read both streams asynchronously to prevent buffer deadlock
$stdoutTask = $proc.StandardOutput.ReadToEndAsync()
$stderrTask = $proc.StandardError.ReadToEndAsync()   # Consumed but discarded

# Windows-side timeout: read from env var, default 1100s
$psTimeout = [int]($env:IRACE_PS_TIMEOUT)
if ($psTimeout -le 0) { $psTimeout = 1100000 }

if (-not $proc.WaitForExit($psTimeout)) {
    # Timeout: kill the wsl process tree (wsl + bash + FuzzyFW)
    & taskkill /F /T /PID $proc.Id 2>&1 | Out-Null
    [Console]::WriteLine("Inf")
    exit 0   # exit 0 so irace treats Inf as a valid (bad) performance value
}

# Write captured stdout to pipeline stdout (irace reads this as the objective value)
$stdout = $stdoutTask.GetAwaiter().GetResult()
$stderrTask.GetAwaiter().GetResult() | Out-Null   # Discard stderr (bash job-control noise)
[Console]::Write($stdout)

exit $proc.ExitCode
