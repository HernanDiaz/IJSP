# target-runner.ps1
# Calls target-runner.sh via WSL with a 310-second Windows-side timeout.
# If the process exceeds 310s, taskkill /F /T kills the entire WSL process tree.

param([Parameter(ValueFromRemainingArguments=$true)] [string[]]$PassArgs)

# Derive the .sh path from this script's location (same directory as .ps1)
$ShScript = Join-Path $PSScriptRoot "target-runner.sh"

# Convert Windows path to WSL path: C:\foo\bar -> /mnt/c/foo/bar
$ShScript = $ShScript -replace '\\', '/'
if ($ShScript -match '^([A-Za-z]):(.*)') {
    $ShScript = '/mnt/' + $Matches[1].ToLower() + $Matches[2]
}

# Build argument string for wsl.exe, quoting each arg properly
$QuotedArgs = $PassArgs | ForEach-Object {
    if ($_.Length -eq 0) { '""' }
    elseif ($_ -match '[ \t]') { '"' + $_ + '"' }
    else { $_ }
}
$WslArgStr = 'bash "' + $ShScript + '" ' + ($QuotedArgs -join ' ')

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

if (-not $proc.WaitForExit(1100000)) {
    # 1100-second timeout: kill the wsl process tree (wsl + bash + FuzzyFW)
    & taskkill /F /T /PID $proc.Id 2>&1 | Out-Null
    [Console]::WriteLine("Inf")
    exit 0   # exit 0 so irace treats Inf as a valid (bad) performance value
}

# Write captured stdout to pipeline stdout (irace reads this as the objective value)
$stdout = $stdoutTask.GetAwaiter().GetResult()
$stderrTask.GetAwaiter().GetResult() | Out-Null   # Discard stderr (bash job-control noise)
[Console]::Write($stdout)

exit $proc.ExitCode
