SET APPVERSION_PATH=..\cl_dll\appversion.h
SET TARGET_FILE=Release\client.dll
SET SUFFIX=client-vgui2

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
COPY ..\README.md "%TMPDIR%\README.txt"

:: Copy target file
IF NOT EXIST "%TMPDIR%\cl_dlls" MKDIR "%TMPDIR%\cl_dlls"
COPY /y "%TARGET_FILE%" "%TMPDIR%\cl_dlls\client.dll"

::
:: Create ZIP
::
SET ZIPNAME=..\BugfixedHL-%version%-%SUFFIX%-windows.zip
echo ZIPNAME = %ZIPNAME%
DEL "%ZIPNAME%"
"C:\Program Files\7-Zip\7z.exe" a "%ZIPNAME%" %TMPDIR%
