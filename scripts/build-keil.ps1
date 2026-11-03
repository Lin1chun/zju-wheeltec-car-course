param(
    [Parameter(Mandatory = $true)][string]$Project,
    [Parameter(Mandatory = $true)][string]$Target
)

$uv4 = $env:UV4
if (-not $uv4) {
    $uv4 = "C:\Keil_v5\UV4\UV4.exe"
}

if (-not (Test-Path -LiteralPath $uv4)) {
    throw "Keil UV4.exe not found. Set `$env:UV4 to UV4.exe."
}

$projectPath = (Resolve-Path -LiteralPath $Project).Path
$projectDir = Split-Path -Parent $projectPath
$logPath = Join-Path $projectDir ("build-{0}.log" -f $Target)

& $uv4 -b $projectPath -t $Target -j0 -o $logPath

if (Test-Path -LiteralPath $logPath) {
    Get-Content -LiteralPath $logPath -Encoding Default | Select-Object -Last 20
}

exit $LASTEXITCODE
