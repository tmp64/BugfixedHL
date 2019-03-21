#!/bin/bash

#
# Purpose: create a tar.gz file with server files and put in the repo's root directory
# Script assumes it is run from repo's root directory
# Usage: ./scripts/MakeAmxxRelease.sh < number of threads (default = 2) >
#

# Some settings
BUILD_DIR=build_auto_amxx
THREAD_NUM=2
APPVERSION_PATH="../bugfixed_api/appversion.h"
SUFFIX="server"
TARGET_FILE="bugfixedapi_amxx_i386.so"
MAKE_TARGET="bugfixedapi_amxx"
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

