$extra = @(
    "C:\Program Files\CMake\bin",
    "C:\ProgramData\mingw64\mingw64\bin",
    "$env:LOCALAPPDATA\Microsoft\WinGet\Packages\Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe"
) | Where-Object { Test-Path $_ }

$env:Path = ($extra -join ";") + ";" + $env:Path

$script:CmakeExe = "C:\Program Files\CMake\bin\cmake.exe"
if (-not (Test-Path $script:CmakeExe)) {
    Write-Host "ERROR: CMake not found. Install: winget install Kitware.CMake" -ForegroundColor Red
    return
}

function cmake {
    param([Parameter(ValueFromRemainingArguments = $true)][string[]]$Args)

    $useNinja = $false
    if ($Args -contains "-S" -and ($Args -contains "-B" -or ($Args -match "^-B"))) {
        if ($Args -notcontains "-G" -and $Args -notmatch "^--preset") {
            $useNinja = $true
        }
    }

    if ($useNinja) {
        & $script:CmakeExe @Args -G Ninja
    } else {
        & $script:CmakeExe @Args
    }
}

Write-Host "OK: cmake $(cmake --version | Select-Object -First 1)" -ForegroundColor Green
Write-Host "Use: cmake -S . -B build  (Ninja will be selected automatically)" -ForegroundColor Cyan
