$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$results = Join-Path $root 'results'
New-Item -ItemType Directory -Force -Path $results | Out-Null
$status = 0
$checks = @()
try {
    $manifest = Get-Content -LiteralPath (Join-Path $root 'bundle-manifest.json') -Raw | ConvertFrom-Json
    foreach ($entry in $manifest.files.PSObject.Properties) {
        $path = Join-Path $root $entry.Name
        if (-not (Test-Path -LiteralPath $path -PathType Leaf)) { throw "Missing file: $($entry.Name)" }
        $actual = (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant()
        if ($actual -ne $entry.Value) { throw "File hash differs: $($entry.Name)" }
    }
    $checks += [pscustomobject]@{check='file-integrity';passed=$true;files=$manifest.files.PSObject.Properties.Count}
    $comparison = Join-Path $results 'comparison.csv'
    if (-not (Test-Path -LiteralPath $comparison)) {
        Copy-Item -LiteralPath (Join-Path $root 'comparison-template.csv') -Destination $comparison
    }
    $os = Get-CimInstance Win32_OperatingSystem
    @{version=$os.Version;caption=$os.Caption;architecture=$os.OSArchitecture;utc=(Get-Date).ToUniversalTime().ToString('o')} |
        ConvertTo-Json | Set-Content -LiteralPath (Join-Path $results 'environment.json') -Encoding UTF8
    $screens = @('welcome','main','lp/input-mode','transportation/input-mode','assignment/input',
        'equations/input','games/input','integer/input','networks/input','planning/input','queuing/input')
    $index = 0
    foreach ($screen in $screens) {
        $stdout = Join-Path $results "startup-$index.stdout.txt"
        $stderr = Join-Path $results "startup-$index.stderr.txt"
        $process = Start-Process -FilePath (Join-Path $root 'app/Eidolon.exe') -ArgumentList @('--smoke-test','--screen',$screen) `
            -WorkingDirectory $root -RedirectStandardOutput $stdout -RedirectStandardError $stderr -PassThru
        $finished = $process.WaitForExit(30000)
        if (-not $finished) { $process.Kill(); $process.WaitForExit(); throw "Startup timeout: $screen" }
        $process.Refresh()
        $code = $process.ExitCode
        $checks += [pscustomobject]@{check='screen-startup';screen=$screen;exit_code=$code;passed=($code -eq 0)}
        if ($code -ne 0) { throw "Startup failed for $screen with exit code $code" }
        $index++
    }
    Write-Host 'Integrity and 11 screen startups passed. Continue with testing.md for numerical and TORA checks.'
} catch {
    $status = 1
    $checks += [pscustomobject]@{check='error';passed=$false;message=$_.Exception.Message}
    Write-Host $_.Exception.Message
} finally {
    $checks | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $results 'startup-checks.json') -Encoding UTF8
}
exit $status
