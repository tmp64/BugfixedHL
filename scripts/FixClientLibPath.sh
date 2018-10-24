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

# Fix vgui.so
ORIGINAL=$(ldd ${LIB} | grep vgui.so | awk -F" " '{print $1}')
if [ ! -z "$ORIGINAL" ]; then
    echo "[${LIB}] Fixing vgui.so path"
    REPLACEMENT="vgui.so"
    $PATCHELF --replace-needed ${ORIGINAL} ${REPLACEMENT} ${LIB}
fi

# Fix libtier0.so
ORIGINAL=$(ldd ${LIB} | grep libtier0.so | awk -F" " '{print $1}')
if [ ! -z "$ORIGINAL" ]; then
    echo "[${LIB}] Fixing libtier0.so path"
    REPLACEMENT="libtier0.so"
    $PATCHELF --replace-needed ${ORIGINAL} ${REPLACEMENT} ${LIB}
fi

# Fix libvstdlib.so
ORIGINAL=$(ldd ${LIB} | grep libvstdlib.so | awk -F" " '{print $1}')
if [ ! -z "$ORIGINAL" ]; then
    echo "[${LIB}] Fixing libvstdlib.so path"
    REPLACEMENT="libvstdlib.so"
    $PATCHELF --replace-needed ${ORIGINAL} ${REPLACEMENT} ${LIB}
fi
