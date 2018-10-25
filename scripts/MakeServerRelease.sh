#!/bin/bash

#
# Purpose: create a tar.gz file with server files and put in the repo's root directory
# Script assumes it is run from repo's root directory
# Usage: ./scripts/MakeServerRelease.sh < number of threads (default = 2) >
#

# Some settings
BUILD_DIR=build_auto_server
THREAD_NUM=2
APPVERSION_PATH="../dlls/appversion.h"
SUFFIX="server"
TARGET_FILE="hl.so"
MAKE_TARGET="hl"
CMAKE_OPTIONS=""

if [ ! -f "./scripts/inc_MakeBase.sh" ]; then
    echo "Error: unable to find ./scripts/inc_MakeBase.sh. You need to execute this script from repo's root dir."
    exit 1;
fi

function CopyFiles()
{
    mkdir "${GAMEDIR}/dlls"
    cp "${TARGET_FILE}" "${GAMEDIR}/dlls"

    # Remove VGUI2 UI directory
    rm -rf "${GAMEDIR}/ui"
}

THIS_IS_MAKE_SCRIPT=1
. ./scripts/inc_MakeBase.sh

