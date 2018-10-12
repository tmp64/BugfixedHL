@ECHO OFF
::-------------------------------------------------------------
:: Auto-releasing script
:: Creates a ZIP archive with the latest client Windows release of the SDK
::-------------------------------------------------------------

::
:: Build the client project (and its dependencies)
::
SET MS_BUILD="C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\MSBuild\15.0\Bin\MSBuild.exe"
%MS_BUILD% multiplayer.sln /t:client /p:PlatformTarget=x86 /p:Configuration=Release

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
findstr /L /C:"#define APP_VERSION " cl_dll\appversion.h > %TMPFILE%.tmp1
SET /P version=<"%TMPFILE%.tmp1"

:: Strip away #define and stuff
SET version=%version:~21%
SET version=%version:"=%

::
:: Copy files to a directory
::
SET TMPDIR=%TMPFILE%.tmp2\valve
MKDIR "%TMPFILE%.tmp2"
MKDIR %TMPDIR%
xcopy /s gamedir %TMPDIR%
IF NOT EXIST "%TMPDIR%\cl_dlls" MKDIR "%TMPDIR%\cl_dlls"
COPY /y cl_dll\msvc\Release\client.dll "%TMPDIR%\cl_dlls\client.dll"
COPY README.md "%TMPDIR%\README.txt"

::
:: Create ZIP
::
DEL BugfixedHL-%version%.zip
"C:\Program Files\7-Zip\7z.exe" a BugfixedHL-%version%.zip %TMPDIR%

::
:: Clean up
::
DEL %TMPFILE%.tmp1
RD /S /Q "%TMPFILE%.tmp2"
