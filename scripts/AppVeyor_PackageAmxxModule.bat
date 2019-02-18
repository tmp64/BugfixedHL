SET APPVERSION_PATH=..\bugfixedapi_amxx\appversion.h
SET TARGET_FILE=RelWithDebInfo\bugfixedapi_amxx.dll
SET TARGET_FILE_PDB=RelWithDebInfo\bugfixedapi_amxx.pdb
SET SUFFIX=amxx

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
SET TMPDIR=%TMPFILE%.tmp2\bugfixedapi_amxx
MKDIR "%TMPFILE%.tmp2"
MKDIR %TMPDIR%
COPY ..\README.md "%TMPDIR%\README.txt"
COPY ..\bugfixedapi_amxx\bugfixedapi.inc "%TMPDIR%\bugfixedapi.inc"

COPY /y "%TARGET_FILE%" "%TMPDIR%\bugfixedapi_amxx.dll"
COPY /y "%TARGET_FILE_PDB%" "%TMPDIR%\bugfixedapi_amxx.pdb"

::
:: Create ZIP
::
SET ZIPNAME=..\BugfixedHL-%version%-%SUFFIX%-windows.zip
echo ZIPNAME = %ZIPNAME%
DEL "%ZIPNAME%"
"C:\Program Files\7-Zip\7z.exe" a "%ZIPNAME%" %TMPDIR%
