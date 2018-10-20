#!/bin/bash

# Usage: FixClientLibPath.sh [path to client.so]

LIB=$1

if [ -z "$LIB" ]; then
    echo "Usage: FixClientLibPath.sh [path to client.so]"
    echo "Purpose: replaces original path to vgui.so (e.g. ../lib/public/vgui.so) with generic 'vgui.so'.";
    echo "Requires patchelf to be installed."
    exit 1
fi

if [ -z "$PATCHELF" ]; then
    PATCHELF=$(which patchelf)

    if [ -z "$PATCHELF" ]; then
        echo "Error: 'patchelf' utility is not installed."
        echo "Set PATCHELF envvar to the correct path or"
        echo "use you default package manager to install it."
        exit 1
    fi
fi

if [ ! -f "$LIB" ]; then
    echo "${LIB}: no such file";
    exit 1
fi

echo "[${LIB}] Fixing vgui.so path"

# Get original vgui.so path
ORIGINAL=$(ldd ${LIB} | grep vgui.so | awk -F" " '{print $1}')
REPLACEMENT="vgui.so"
$PATCHELF --replace-needed ${ORIGINAL} ${REPLACEMENT} ${LIB}
