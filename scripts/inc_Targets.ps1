if (-not $THIS_IS_BUILD_SCRIPT)
{
    Write-Host "Use BuildRelease.ps1 instead";
    exit 1;
}


$TARGET_CMAKE_FLAGS = '';	# CMake flags
$TARGET_BUILD = '';			# Name of MSVS project
$TARGET_BINARY_DIR = '';	# Where to put binaries relative to gamedir
$TARGET_GAME_FILES = @();	# Files to copy from $ROOT_DIR/<x> to gamedir/<y>
$TARGET_NEED_LIB_SUFFIX = $false;	# Add _i386 to the filename on Linux
$TARGET_HAS_LIB_SUFFIX = $false;	# Built file has _i386 in the name on Linux

$TARGET_COMMON_FILES = @(
    @('gamedir/delta.lst', 'delta.lst'),
    @('README.md', 'README.md')
);

if ($target -eq 'Client')
{
    $TARGET_CMAKE_FLAGS = '';
    $TARGET_BUILD = 'client';
    $TARGET_BINARY_DIR = 'cl_dlls';

    $TARGET_GAME_FILES += $TARGET_COMMON_FILES;
    $TARGET_GAME_FILES += @(
        ,@('gamedir/sprites', 'sprites')
    );
}
elseif ($target -eq 'ClientVGUI2')
{
    $TARGET_CMAKE_FLAGS = '-DUSE_VGUI2=1';
    $TARGET_BUILD = 'client';
    $TARGET_BINARY_DIR = 'cl_dlls';

    $TARGET_GAME_FILES += $TARGET_COMMON_FILES;
    $TARGET_GAME_FILES += @(
        ,@('gamedir/resource', 'resource')
        ,@('gamedir/sprites', 'sprites')
        ,@('gamedir/ui', 'ui')
    );
}
elseif ($target -eq 'Client4554')
{
    $TARGET_CMAKE_FLAGS = '-DUSE_VGUI2=1 -DVGUI2_BUILD_4554=1';
    $TARGET_BUILD = 'client';
    $TARGET_BINARY_DIR = 'cl_dlls';

    $TARGET_GAME_FILES += $TARGET_COMMON_FILES;
    $TARGET_GAME_FILES += @(
        ,@('gamedir/resource', 'resource')
        ,@('gamedir/sprites', 'sprites')
        ,@('gamedir/ui', 'ui')
    );
}
elseif ($target -eq 'Server')
{
    $TARGET_CMAKE_FLAGS = '';
    $TARGET_BUILD = 'hl';
    $TARGET_BINARY_DIR = 'dlls';

    $TARGET_GAME_FILES += $TARGET_COMMON_FILES;
}
elseif ($target -eq 'Amxx')
{
    $TARGET_CMAKE_FLAGS = '';
    $TARGET_BUILD = 'bugfixedapi_amxx';
    $TARGET_BINARY_DIR = 'addons/amxmodx/modules';
    $TARGET_NEED_LIB_SUFFIX = $true;
    $TARGET_HAS_LIB_SUFFIX = $true;

    $TARGET_GAME_FILES += @(
        ,@('gamedir/addons/amxmodx/scripting/bugfixedapi.inc', 'addons/amxmodx/scripting/bugfixedapi.inc')
    );
}
else
{
    Write-Error "Unknown target";
}

#--------------------------------------------------
# Target functions
#--------------------------------------------------
function Get-TargetCmakeFlags
{
    Write-Output "$TARGET_CMAKE_FLAGS";
}

function Get-BuildTarget
{
    Write-Output "$TARGET_BUILD";
}

function Get-TargetGameFiles
{
    Write-Output $TARGET_GAME_FILES;
}

function Get-TargetBinaryDir
{
    Write-Output $TARGET_BINARY_DIR;
}
