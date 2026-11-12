$ErrorActionPreference = 'Stop'

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$c10Path = Join-Path $repoRoot 'firmware/experimental/c10b-course/Core/Src/main.c'
$c50Path = Join-Path $repoRoot 'firmware/experimental/c50c-midterm/BALANCE/balance_task.c'
$showPath = Join-Path $repoRoot 'firmware/experimental/c50c-midterm/BALANCE/show_task.c'

$latin1 = [Text.Encoding]::GetEncoding(1252)
$c10 = $latin1.GetString([IO.File]::ReadAllBytes($c10Path))
$c50 = $latin1.GetString([IO.File]::ReadAllBytes($c50Path))
$show = $latin1.GetString([IO.File]::ReadAllBytes($showPath))

$checks = @(
    @{ Name = 'C10B display uses transmitted front distance'; Pass = $c10.Contains('OLED_ShowFloat16(32,0,front_mm/1000.0f,1,2);') },
    @{ Name = 'C10B scans each sector in 100 ms'; Pass = $c10.Contains('#define RANGER_SCAN_TICKS 10u') -and $c10.Contains('scan_tick>=RANGER_SCAN_TICKS') },
    @{ Name = 'C10B uses immediate-near filtering'; Pass = $c10.Contains('mm < scan_dist_mm[sec]') },
    @{ Name = 'C10B clears immediately on no echo'; Pass = $c10.Contains('mm >= RANGER_NO_ECHO_MM') },
    @{ Name = 'C10B stops early enough for scan latency'; Pass = $c10.Contains('#define RANGER_STOP_MM 600u') -and $c10.Contains('#define RANGER_CLEAR_MM 650u') },
    @{ Name = 'C50C rejects invalid startup distance'; Pass = $c50.Contains('front_valid ? (float)c10b_recv.distance * 0.001f : 0.0f') },
    @{ Name = 'C50C waits for stable frames before motion'; Pass = $c50 -match '#define\s+MID_READY_FRAMES\s+10\b' -and $c50.Contains('mid.ready_frames') },
    @{ Name = 'C50C stop threshold includes sensor latency'; Pass = $c50 -match '#define\s+MID_STOP_MM\s+600\b' -and $c50 -match '#define\s+MID_CLEAR_MM\s+650\b' },
    @{ Name = 'C50C odometry excludes pause and turn'; Pass = $c50.Contains('if(mid.state == MID_STATE_OUT || mid.state == MID_STATE_BACK)') },
    @{ Name = 'C50C total distance is displayed to 1 mm'; Pass = $show.Contains('OLED_ShowFloat16(32,16,Midterm_Distance(),1,3);') },
    @{ Name = 'C50C average speed is displayed to 1 mm/s'; Pass = $show.Contains('OLED_ShowFloat16(32,32,Midterm_AvgSpeed(),1,3);') }
)

$failed = @($checks | Where-Object { -not $_.Pass })
if ($failed.Count -gt 0) {
    $failed | ForEach-Object { Write-Host "FAILED: $($_.Name)" }
    exit 1
}

Write-Host "Midterm invariants passed: $($checks.Count) checks."
