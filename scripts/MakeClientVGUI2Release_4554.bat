@ECHO OFF
::-------------------------------------------------------------
:: Auto-releasing script
:: Creates a ZIP archive with the latest client Windows release of the SDK
::-------------------------------------------------------------

::
:: Options
::
SET BUILD_DIR=build_auto_client_vgui2_4554
SET APPVERSION_PATH=..\cl_dll\appversion.h
SET SUFFIX=client-vgui2-4554
SET TARGET_FILE=RelWithDebInfo\client.dll
SET TARGET_FILE_PDB=RelWithDebInfo\client.pdb
SET MAKE_TARGET=client
SET MS_BUILD="C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\MSBuild\15.0\Bin\MSBuild.exe"

::
:: Clean up
::
IF EXIST "%BUILD_DIR%" RMDIR /S /Q "%BUILD_DIR%"

::
:: Generate VS files using CMake
::
echo ---------------- Generating Visual Studio projects using CMake
mkdir "%BUILD_DIR%"
cd "%BUILD_DIR%"
cmake -G"Visual Studio 15 2017" -T"v141_xp" -DAUTO_DEPLOY=0 -DUSE_VGUI2=1 -DVGUI2_BUILD_4554=1 ..

IF NOT EXIST "BugfixedHL.sln" (
	echo Error: failed to create BugfixedHL.sln
	cd ..
	EXIT /B 1
)

::
:: Build
::
echo ---------------- Building target %MAKE_TARGET%
%MS_BUILD% BugfixedHL.sln /t:%MAKE_TARGET% /p:PlatformTarget=x86 /p:Configuration=RelWithDebInfo

IF NOT EXIST "%TARGET_FILE%" (
	echo Error! %TARGET_FILE% was not created by the compiler. Check error log, fix the errors and run the script again.
	cd ..
	EXIT /B 1
)

::
:: Generate temp file name
::
:GETTEMPNAME
:: Use current path, current time and random number to create unique file name
SET TMPFILE=version-%CD:~-15%-%RANDOM%-%TIME:~-5%-%RANDOM%
:: Remove bad characters
SET TMPFILE=%TMPFILE:\=%
SET TMPFILE=%TMPFILE:.=%
SET TMPFILE=%TMPFILE:,=%
SET TMPFILE=%TMPFILE: =%
:: Will put in a temporary directory
SET TMPFILE=%TMP%\%TMPFILE%
IF EXIST "%TMPFILE%" GOTO :GETTEMPNAME

::
:: Get version
::
findstr /L /C:"#define APP_VERSION " %APPVERSION_PATH% > %TMPFILE%.tmp1
SET /P version=<"%TMPFILE%.tmp1"

:: Strip away #define and stuff
SET version=%version:~21%
SET version=%version:"=%
SET version=%version:+=-%

::
:: Copy files to a directory
::
SET TMPDIR=%TMPFILE%.tmp2\valve
MKDIR "%TMPFILE%.tmp2"
MKDIR %TMPDIR%
xcopy /s ..\gamedir %TMPDIR%

:: Copy target file
IF NOT EXIST "%TMPDIR%\cl_dlls" MKDIR "%TMPDIR%\cl_dlls"
COPY /y "%TARGET_FILE%" "%TMPDIR%\cl_dlls\client.dll"
COPY /y "%TARGET_FILE_PDB%" "%TMPDIR%\cl_dlls\client.pdb"
COPY ..\README.md "%TMPDIR%\README.txt"

::
:: Create ZIP
::
SET ZIPNAME=..\BugfixedHL-%version%-%SUFFIX%-windows.zip
echo ZIPNAME = %ZIPNAME%
DEL "%ZIPNAME%"
"C:\Program Files\7-Zip\7z.exe" a "%ZIPNAME%" %TMPDIR%

::
:: Clean up
::
DEL %TMPFILE%.tmp1
RD /S /Q "%TMPFILE%.tmp2"

:: Go back to root dir
cd ..
