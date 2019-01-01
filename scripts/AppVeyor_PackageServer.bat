SET APPVERSION_PATH=..\dlls\appversion.h
SET TARGET_FILE=RelWithDebInfo\hl.dll
SET TARGET_FILE_PDB=RelWithDebInfo\hl.pdb
SET SUFFIX=server

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

:: Remove VGUI2 UI folder
RMDIR /s /q %TMPDIR%\ui

:: Copy target file
IF NOT EXIST "%TMPDIR%\dlls" MKDIR "%TMPDIR%\dlls"
COPY /y "%TARGET_FILE%" "%TMPDIR%\dlls\hl.dll"
COPY /y "%TARGET_FILE_PDB%" "%TMPDIR%\dlls\hl.pdb"

::
:: Create ZIP
::
SET ZIPNAME=..\BugfixedHL-%version%-%SUFFIX%-windows.zip
echo ZIPNAME = %ZIPNAME%
DEL "%ZIPNAME%"
"C:\Program Files\7-Zip\7z.exe" a "%ZIPNAME%" %TMPDIR%
