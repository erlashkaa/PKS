. "$PSScriptRoot\setup-env.ps1"

Set-Location $PSScriptRoot

if (Test-Path build\CMakeCache.txt) {
    $cache = Get-Content build\CMakeCache.txt -Raw
    if ($cache -match "CMAKE_GENERATOR:INTERNAL=NMake") {
        Write-Host "Removing old NMake build cache..." -ForegroundColor Yellow
        Remove-Item -Recurse -Force build
    }
}

cmake -S . -B build
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

cmake --build build
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host ""
Write-Host "Build OK. Run:" -ForegroundColor Green
Write-Host "  .\build\network_simulator.exe" -ForegroundColor Cyan
