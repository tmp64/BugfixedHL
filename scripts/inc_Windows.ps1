if (-not $THIS_IS_BUILD_SCRIPT)
{
    Write-Host "Use BuildRelease.ps1 instead";
    exit 1;
}

if (-not (Get-Module -Listavailable -Name VSSetup))
{
    Write-Host "PowerShell module VSSetup was not found."
    Write-Host "Visit https://github.com/Microsoft/vssetup.powershell for more details."
    exit 1;
}

$PLATFORM_MSVS_PATH = '';
$PLATFORM_ENABLE_WINXP = $false;

# Try to find MSVS 2017 with Windows XP support
$PLATFORM_VS = Get-VSSetupInstance -All | Select-VSSetupInstance -Require 'Microsoft.VisualStudio.Component.WinXP' -Latest
if ($PLATFORM_VS)
{
    $PLATFORM_MSVS_PATH = $PLATFORM_VS.InstallationPath;
    $PLATFORM_ENABLE_WINXP = $true;
}
else
{
    # Try to find any Visual Stduio with C++
    $PLATFORM_VS = Get-VSSetupInstance -All | Select-VSSetupInstance -Require 'Microsoft.VisualStudio.Component.VC.CoreIde' -Latest
    if (-not $PLATFORM_VS)
    {
        Write-Host "Visual Studio was not found.";
        exit 1;
    }
}

$PLATFORM_MSVS_PATH = $PLATFORM_VS.InstallationPath;
Write-Host "Found" ${PLATFORM_VS}.DisplayName ${PLATFORM_VS}.InstallationVersion "at '${PLATFORM_MSVS_PATH}'";
if ($PLATFORM_ENABLE_WINXP)
{
    Write-Host "Windows XP support enabled."
}

# Set CMake generator value
$PLATFORM_CMAKE_GENERATOR = '';
if ($PLATFORM_VS.InstallationVersion.Major -eq 15)
{
    $PLATFORM_CMAKE_GENERATOR = 'Visual Studio 15 2017';
}
elseif ($PLATFORM_VS.InstallationVersion.Major -eq 16)
{
    $PLATFORM_CMAKE_GENERATOR = 'Visual Studio 16 2019';
}
else
{
    Write-Host "Unsupported Visual Studio" $PLATFORM_VS.InstallationVersion".";
    Write-Host "only 2017 (15) and 2019 (16) are supported";
}

function Get-MSBuildPath
{
    $p = '';
    if ($PLATFORM_VS.InstallationVersion.Major -eq 15)
    {
        $p = "MSBuild\15.0\Bin\MSBuild.exe"
    }
    elseif ($PLATFORM_VS.InstallationVersion.Major -eq 16)
    {
        $p = "MSBuild\Current\Bin\MSBuild.exe"
    }
    else
    {
        Write-Error "Unsupported Visual Studio";
    }
    Write-Output "${PLATFORM_MSVS_PATH}/${p}";
}

#--------------------------------------------------
# Platform functions
#--------------------------------------------------
function Get-PlatformCmakeArgs
{
    $a = "-G `"${PLATFORM_CMAKE_GENERATOR}`" -A `"Win32`"";
    if ($PLATFORM_ENABLE_WINXP)
    {
        $a = "$a -T `"v141_xp`"";
    }
    Write-Output $a;
}

function Invoke-PlatformBuild
{
    Param (
        [Parameter (Mandatory=$true)][string]$BuildDir
    )

    Push-Location
    Set-Location $BuildDir

    $target = Get-BuildTarget;
    $MSBUILD = Get-MSBuildPath;
    & $MSBUILD BugfixedHL.sln -nologo /t:${target} /p:PlatformTarget=x86 /p:Configuration=RelWithDebInfo /m

    if ($LastExitCode -ne 0)
    {
        Pop-Location;
        Write-Host "Build failed.";
        exit 1;
    }

    Pop-Location
}

function Copy-Binaries
{
    Param (
        [Parameter (Mandatory=$true)][string]$BuildDir,
        [Parameter (Mandatory=$true)][string]$GameDir
    )

    $buildtarg = Get-BuildTarget;
    $bin_dir = Get-TargetBinaryDir;
    $to = "${GameDir}/${bin_dir}";

    function CopyWtihLog
    {
        Param (
            [Parameter (Mandatory=$true)][string]$From,
            [Parameter (Mandatory=$true)][string]$To
        )
        New-Item -ItemType File -Path $To -Force | Out-Null;
        Copy-Item "$From" -Destination "$To" -Force | Out-Null;
        Write-Host "${From} -> ${To}"
    }

    CopyWtihLog -From "${BuildDir}/RelWithDebInfo/${buildtarg}.dll" -To "$to/${buildtarg}.dll"
    CopyWtihLog -From "${BuildDir}/RelWithDebInfo/${buildtarg}.pdb" -To "$to/${buildtarg}.pdb"
}
