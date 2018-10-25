if [ -z ${THIS_IS_MAKE_SCRIPT} ]; then
    echo "Error: you can't execute inc_MakeBase.sh directly"
    exit 1;
fi

if [ -n "$1" ] && [ "$1" -eq "$1" ] 2>/dev/null && [ "$1" -ge 1 ]; then
    THREAD_NUM=$1
fi

# Print settings
echo "Build settings:"
echo "SUFFIX = ${SUFFIX}"
echo "THREAD_NUM = ${THREAD_NUM}"
echo "TARGET_FILE = ${TARGET_FILE}"
echo "TARGET_FILE = ${BUILD_DIR}"
echo "BUILD_DIR = ${BUILD_DIR}"
echo "MAKE_TARGET = ${MAKE_TARGET}"
echo "CMAKE_OPTIONS = ${CMAKE_OPTIONS}"

# Foolproofness
if [ ! -f CMakeLists.txt -o ! -d cl_dll -a ! -d dlls ]; then
    echo "Error! Couldn't find 'CMakeLists.txt', 'cl_dll' or 'dlls'. You should be running the script form repo's root."
    exit 1;
fi;

# Find CMake
if [ -z "$CMAKE" ]; then
    CMAKE=$(which cmake)

    if [ -z "$CMAKE" ]; then
        echo "Error: cmake is not installed."
        echo "Set CMAKE envvar to the correct path or use your default package manager to install it."
        exit 1
    fi
fi

# Clean up
if [ -d "${BUILD_DIR}" ]; then
    rm -rf "${BUILD_DIR}";
fi

mkdir "${BUILD_DIR}";
cd "${BUILD_DIR}";

# Generate makefiles
echo "---------------- Generating Makefiles using CMake"
$CMAKE .. -DAUTO_DEPLOY=0 ${CMAKE_OPTIONS}

# Build library
echo "---------------- Making target ${MAKE_TARGET} with ${THREAD_NUM} thread(s)"
make -j${THREAD_NUM} "${MAKE_TARGET}"

if [ ! -f "$TARGET_FILE" ]; then
    echo "Error! ${TARGET_FILE} was not created by the compiler. Check error log, fix the errors and run the script again."
    exit 1;
fi

# Get game version
VERSION=$(cat "${APPVERSION_PATH}" | grep -i '#define APP_VERSION ' | sed -e 's/#define APP_VERSION \(.*\)/\1/i')
VERSION=${VERSION//'"'}
VERSION=${VERSION//+/-}

# Create temporary directory
TMP_DIR=$(mktemp -d)
GAMEDIR_NAME=BugfixedHL-${VERSION}-${SUFFIX}-linux
GAMEDIR="${TMP_DIR}/${GAMEDIR_NAME}"

echo "---------------- Creating ${GAMEDIR_NAME}.tar.gz"

mkdir -p "${GAMEDIR}"

# Copy files from gamedir to $GAMEDIR
cp -r ../gamedir/. "${GAMEDIR}"

# Copy other files
CopyFiles

# Create the archive
tar -zcvf "${GAMEDIR_NAME}.tar.gz" -C "${TMP_DIR}" "${GAMEDIR_NAME}"

if [ ! -f "${GAMEDIR_NAME}.tar.gz" ]; then
    echo "Error! ${GAMEDIR_NAME}.tar.gz. Check error log, fix the errors and run the script again."
    exit 1
fi

# Copy and clean-up
cp "${GAMEDIR_NAME}.tar.gz" ../
rm -rf "${TMP_DIR}"

