#!/bin/false FIXME


# BuildRelease.ps1 -Target [target] -Out [path] -MakeZip -CleanUp


Param (
    [string]$Target = "Help",
    [string]$Out = "__UNSPECIFIED",
    [switch]$MakeZip = $false,
    [switch]$Updater = $false,
    [switch]$CleanUp = $false
)

$ErrorActionPreference = "Stop"
$THIS_IS_BUILD_SCRIPT = $true;   # For checking in includes

# Check params
if (($Target -ne "Help") -and ($Target -ne "Client") -and ($Target -ne "ClientVGUI2") -and ($Target -ne "Client4554") -and ($Target -ne "Server") -and ($Target -ne "Amxx"))
{
    Write-Host "Target is invalid";
    Write-Host "Type 'BuildRelease.ps1 -Target Help' for help";
    exit 1;
}

if ($Target -eq "Help")
{
    Write-Host "BuildRelease.ps1 -Target [target] -Out [path] -MakeZip -CleanUp";
    Write-Host "`t-Target`t`ttarget that needs to be built:`n`t`t`t`t`tHelp, Client, ClientVGUI2, Client4554, Server, Amxx";
    Write-Host "`t-Out`t`tdirectory in which build and game files will be located.`n`t`t`t`t`tDefault: <repo>/autobuild/<version>-<target>";
    Write-Host "`t-MakeZip`twill make a ZIP file with binaries and game files (ready to be unpacked to 'Half-Life/valve')";
    Write-Host "`t-Updater`tenable update checker";
    Write-Host "`t-CleanUp`twill remove build files when build is finished successfully";
    exit 0;
}


# Find Git
$GIT = '';

if (Test-Path -Path Env:GIT_PATH)
{
    $GIT = $env:GIT_PATH;
}
elseif (Get-Command "git" -ErrorAction SilentlyContinue)
{
    $GIT = (Get-Command git).Definition;
}
else
{
    Write-Host "Git executable was not found.";
    Write-Host "Set environment variable GIT_PATH to path to git.";
    exit 1;
}

Write-Host "Found git at $GIT";


# Find CMake
$CMAKE = '';

if (Test-Path -Path Env:CMAKE_PATH)
{
    $CMAKE = $env:CMAKE_PATH;
}
elseif (Get-Command "cmake" -ErrorAction SilentlyContinue)
{
    $CMAKE = (Get-Command cmake).Definition;
}
else
{
    Write-Host "CMake executable was not found.";
    Write-Host "Set environment variable CMAKE_PATH to path to cmake.";
    exit 1;
}

Write-Host "Found cmake at $CMAKE";


# Find repo directory
$ROOT_DIR = & $GIT rev-parse --show-toplevel;
Write-Host "Repo root dir: $ROOT_DIR";


# Get repo version
$VERSION = (& $GIT describe --long --tags --dirty --always).Substring(1).Replace('-', '.').Replace(".g", "-").Replace(".dirty", "-m");
Write-Host "Repo version: $VERSION";

# Update $Out
$IS_DEFAULT_OUT_DIR = $false;
if ($Out -eq "__UNSPECIFIED")
{
    $Out = "$ROOT_DIR/autobuild/$VERSION-$Target";
    $IS_DEFAULT_OUT_DIR = $true;
}
New-Item -ItemType Directory -Force -Path $Out | Out-Null;
Remove-Item "$Out/*" -Force -Recurse;
Write-Host "Out directory: $Out";

# Detect OS
$IS_WINDOWS = $false;
if ([System.Environment]::OSVersion.Platform -eq "Win32NT")
{
    $IS_WINDOWS = $true;
    Write-Host "Detected Windows OS";
}
else
{
    Write-Host "Detected Linux OS";
}
Write-Host;


# Load platform script
if ($IS_WINDOWS)
{
    . $ROOT_DIR/scripts/inc_Windows.ps1;
}
else
{
    . $ROOT_DIR/scripts/inc_Linux.ps1;
}

if (-not $?)
{
    Write-Host "Platform script failed.";
    exit 1;
}
Write-Host;


# Load target script
. $ROOT_DIR\scripts\inc_Targets.ps1;


# Run CMake
$CMAKE_BUILD_DIR = "$Out/build";
$PLATFORM_ARGS = Get-PlatformCmakeArgs;
$TARGET_FLAGS = Get-TargetCmakeFlags;
New-Item -ItemType Directory -Force -Path $CMAKE_BUILD_DIR | Out-Null;

$CMAKE_UPDATER_FLAG = '';
if ($Updater)
{
    $CMAKE_UPDATER_FLAG = '-DUSE_UPDATER=1'
}

$CMAKE_ARGS = "-B `"${CMAKE_BUILD_DIR}`" -S `"${ROOT_DIR}`" -DAUTO_DEPLOY=0 ${CMAKE_UPDATER_FLAG} ${PLATFORM_ARGS} ${TARGET_FLAGS}";
Write-Host "Running cmake ${CMAKE_ARGS}";
$CMAKE_CMD = "& `"$CMAKE`" $CMAKE_ARGS";
Invoke-Expression $CMAKE_CMD;

# Build
Invoke-PlatformBuild -BuildDir $CMAKE_BUILD_DIR;

# Copy to gamedir
Write-Host "Copying game files";

$OUT_GAME_DIR = "$Out/gamedir";
New-Item -ItemType Directory -Force -Path $OUT_GAME_DIR | Out-Null;

foreach ($i in Get-TargetGameFiles)
{
    $from = "${ROOT_DIR}/" + $i[0];
    $to = "${OUT_GAME_DIR}/" + $i[1];
    if ((Get-Item $from) -is [System.IO.FileInfo])
    {
        New-Item -ItemType File -Path $to -Force | Out-Null; # Workaround to create directory structure
    }
    Copy-Item $from -Destination $to -Recurse -Force | Out-Null;
    Write-Host "${from} -> ${to}"
}


Write-Host "Copying game binaries";
Copy-Binaries -BuildDir $CMAKE_BUILD_DIR -GameDir $OUT_GAME_DIR;

# Create .zip
if ($MakeZip)
{
    $ZIP_NAME = "BugfixedHL-${VERSION}-${Target}";
    if ($IS_WINDOWS)
        { $ZIP_NAME = "$ZIP_NAME-windows"; }
    else 
        { $ZIP_NAME = "$ZIP_NAME-linux"; }
    $ZIP_NAME = "$ZIP_NAME.zip";

    if ($IS_DEFAULT_OUT_DIR)
    {
        $ZIP_NAME = "../$ZIP_NAME";
    }

    Compress-Archive -Path $OUT_GAME_DIR -DestinationPath "${Out}/${ZIP_NAME}" -Force
}


# Clean up
if ($CleanUp)
{
    if ($MakeZip)
    {
        Remove-Item "$OUT_GAME_DIR" -Force -Recurse;
    }

    Remove-Item "$CMAKE_BUILD_DIR" -Force -Recurse;

    if ((Get-ChildItem "$Out" | Measure-Object).Count -eq 0)
    {
        Remove-Item "$Out";
    }

    Write-Host "Cleaned up";
}


Write-Host;
Write-Host;
Write-Host "All done!";
Write-Host;
Write-Host;
