if (-not $THIS_IS_BUILD_SCRIPT)
{
    Write-Host "Use BuildRelease.ps1 instead";
    exit 1;
}

# Find pathelf
$PATCHELF = '';

if (Test-Path -Path Env:PATCHELF_PATH)
{
    $PATCHELF = $env:PATCHELF_PATH;
}
elseif (Get-Command "patchelf" -ErrorAction SilentlyContinue)
{
    $PATCHELF = (Get-Command patchelf).Definition;
}
else
{
    Write-Host "Patchelf executable was not found.";
    Write-Host "Install patchelf or set environment variable PATCHELF_PATH to the path to the executable.";
    exit 1;
}

Write-Host "Found patchelf at $PATCHELF";

# Find ninja
$NINJA = '';

if (Test-Path -Path Env:NINJA_PATH)
{
    $NINJA = $env:NINJA_PATH;
}
elseif (Get-Command "ninja" -ErrorAction SilentlyContinue)
{
    $NINJA = (Get-Command ninja).Definition;
}
else
{
    Write-Host "Ninja executable was not found.";
    Write-Host "Install ninja-build or set environment variable NINJA_PATH to the path to the executable.";
    exit 1;
}

Write-Host "Found ninja at $NINJA";

#--------------------------------------------------
# Platform functions
#--------------------------------------------------
function Get-PlatformCmakeArgs
{
    Write-Output "-G`"Ninja`" -DCMAKE_TOOLCHAIN_FILE=`"$ROOT_DIR/cmake/Linux32Toolchain.cmake`"";
}

function Invoke-PlatformBuild
{
    Param (
        [Parameter (Mandatory=$true)][string]$BuildDir
    )

    Push-Location;
    Set-Location $BuildDir;

    Write-Host $BuildDir;

    $target = Get-BuildTarget;
    & $NINJA $target;

    if ($LastExitCode -ne 0)
    {
        Pop-Location;
        Write-Host "Build failed.";
        exit 1;
    }

    Pop-Location;
}

function Copy-Binaries
{
    Param (
        [Parameter (Mandatory=$true)][string]$BuildDir,
        [Parameter (Mandatory=$true)][string]$GameDir
    )

    $buildtarg = Get-BuildTarget;
    $bin_dir = Get-TargetBinaryDir;

    $From = "${BuildDir}/${buildtarg}";

    if ($TARGET_HAS_LIB_SUFFIX)
    {
        $From = "${From}_i386";
    }

    $From = "${From}.so";
    $To = "${GameDir}/${bin_dir}/${buildtarg}";

    if ($TARGET_NEED_LIB_SUFFIX)
    {
        $To = "${To}_i386";
    }

    $To = "${To}.so";

    New-Item -ItemType File -Path $To -Force | Out-Null;
    Copy-Item "$From" -Destination "$To" -Force | Out-Null;
    Write-Host "${From} -> ${To}"
}
