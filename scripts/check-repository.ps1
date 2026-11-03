$ErrorActionPreference = 'Stop'

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$errors = [System.Collections.Generic.List[string]]::new()

$requiredPaths = @(
    'README.md',
    'AGENTS.md',
    'CONTRIBUTING.md',
    'CHANGELOG.md',
    'docs/README.md',
    'docs/course-map.md',
    'docs/toolchain.md',
    'docs/protocol.md',
    'firmware/original/c10b',
    'firmware/original/c50c',
    'firmware/experimental/c10b-course',
    'firmware/experimental/c50c-midterm'
)

foreach ($relativePath in $requiredPaths) {
    if (-not (Test-Path -LiteralPath (Join-Path $repoRoot $relativePath))) {
        $errors.Add("Missing required path: $relativePath")
    }
}

$trackedPaths = @(& git -C $repoRoot -c core.quotepath=false ls-files -c -o --exclude-standard | Sort-Object -Unique)
foreach ($path in $trackedPaths) {
    if ($path -match '[^\x00-\x7F]') {
        $errors.Add("Non-ASCII tracked path: $path")
    }
}

$forbiddenRx = '(?i)(^|/)(OBJ|Objects|Listings|DebugConfig)/|\.(hex|bin|axf|map|uvoptx|uvguix|log)$'
foreach ($path in $trackedPaths) {
    if ($path -match $forbiddenRx) {
        $errors.Add("Generated or user file is tracked: $path")
    }
}

$textExtensions = @('.md', '.txt', '.c', '.h', '.ps1', '.yml', '.yaml')
foreach ($path in $trackedPaths) {
    if ($textExtensions -notcontains [IO.Path]::GetExtension($path).ToLowerInvariant()) {
        continue
    }

    $fullPath = Join-Path $repoRoot $path
    $content = Get-Content -LiteralPath $fullPath -Raw -Encoding utf8
    if ($content -match '(?m)^(<<<<<<<|=======|>>>>>>>)') {
        $errors.Add("Merge conflict marker: $path")
    }
}

$linkRx = [regex]'\[[^\]]*\]\(([^)]+)\)'
foreach ($path in $trackedPaths | Where-Object { $_.EndsWith('.md', [StringComparison]::OrdinalIgnoreCase) }) {
    $fullPath = Join-Path $repoRoot $path
    $content = Get-Content -LiteralPath $fullPath -Raw -Encoding utf8
    $docDirectory = Split-Path -Parent $fullPath

    foreach ($match in $linkRx.Matches($content)) {
        $target = $match.Groups[1].Value.Trim().Trim('<', '>')
        if ($target -match '^(https?://|mailto:|#)' -or $target.StartsWith('codex://')) {
            continue
        }

        $target = ($target -split '#', 2)[0]
        if ([string]::IsNullOrWhiteSpace($target)) {
            continue
        }

        if ($target.StartsWith('/')) {
            $resolved = Join-Path $repoRoot $target.TrimStart('/')
        }
        else {
            $resolved = Join-Path $docDirectory $target
        }

        if (-not (Test-Path -LiteralPath $resolved)) {
            $errors.Add("Broken Markdown link: $path -> $target")
        }
    }
}

if ($errors.Count -gt 0) {
    $errors | ForEach-Object { Write-Error $_ }
    exit 1
}

Write-Host "Repository check passed: $($trackedPaths.Count) tracked files."
