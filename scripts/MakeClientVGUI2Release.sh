#!/bin/bash

#
# Purpose: create a tar.gz file with cleint files and put in the repo's root directory
# Script assumes it is run from repo's root directory
# Usage: ./scripts/MakeServerVGUI2Release.sh < number of threads (default = 2) >
#

# Some settings
BUILD_DIR=build_auto_client_vgui2
THREAD_NUM=2
APPVERSION_PATH="../cl_dll/appversion.h"
SUFFIX="client-vgui2"
TARGET_FILE="client.so"
MAKE_TARGET="client"
CMAKE_OPTIONS="-DUSE_VGUI2=1"

if [ ! -f "./scripts/inc_MakeBase.sh" ]; then
    echo "Error: unable to find ./scripts/inc_MakeBase.sh. You need to execute this script from repo's root dir."
    exit 1;
fi

function CopyFiles()
{
    mkdir "${GAMEDIR}/cl_dlls"
    cp "${TARGET_FILE}" "${GAMEDIR}/cl_dlls"
}

THIS_IS_MAKE_SCRIPT=1
. ./scripts/inc_MakeBase.sh

