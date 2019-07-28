@ECHO OFF
::
:: Pre-build auto-versioning script
::
:: Usage: GenerateAppVersion.bat [git repo dir] [file to generate] [major] [minor] [patch] [tag] [metadata]
::

SET repodir=%~1
SET srcfile=%~2
SET arg_major=%~3
SET arg_minor=%~4
SET arg_patch=%~5
SET arg_tag=%~6
SET arg_metadata=%~7

SET old_version=""
SET new_version="0.0.0"
SET new_specialbuild=""
SET git_version="v0.0-0"
SET git_date="?"

::
:: Check for git.exe presence
::
git.exe 2>NUL >NUL
SET errlvl=%ERRORLEVEL%

::
:: Read old appversion.h, if present
::
IF EXIST "%srcfile%" (
	FOR /F "usebackq tokens=1,2,3" %%i IN ("%srcfile%") DO (
		IF %%i==#define (
			IF %%j==APP_VERSION SET old_version=%%k
		)
	)
)
SET old_version=%old_version:~1,-1%

::
:: Bail out if git.exe not found
::
IF NOT "%errlvl%" == "1" (
	ECHO Can not locate git.exe. Auto-versioning step will not be performed.

	:: If we don't have appversion.h, we need to create it
	IF "%old_version%" == "" GOTO COMPARE
	EXIT /B 0
)

::
:: Generate temp file name
::
:GETTEMPNAME
:: Use current path, current time and random number to create unique file name
SET TMPFILE=git-%CD:~-15%-%RANDOM%-%TIME:~-5%-%RANDOM%
:: Remove bad characters
SET TMPFILE=%TMPFILE:\=%
SET TMPFILE=%TMPFILE:.=%
SET TMPFILE=%TMPFILE:,=%
SET TMPFILE=%TMPFILE: =%
:: Will put in a temporary directory
SET TMPFILE=%TMP%.\%TMPFILE%
IF EXIST "%TMPFILE%" GOTO :GETTEMPNAME

::
:: Get information from GIT repository
::
SET errlvl=0

:: e.g. v1.3-0-gcc5b7c1-dirty
git.exe -C "%repodir%." describe --long --tags --dirty --always > "%TMPFILE%.tmp1"
IF NOT "%ERRORLEVEL%" == "0" THEN SET errlvl=1

:: e.g. 2019-07-23 15:28:44 +0700
git.exe -C "%repodir%." log -1 --format^=%%ci >> "%TMPFILE%.tmp2"
IF NOT "%ERRORLEVEL%" == "0" THEN SET errlvl=1

:: e.g. cc5b7c1
git.exe -C "%repodir%." rev-parse --short HEAD > "%TMPFILE%.tmp3"
IF NOT "%ERRORLEVEL%" == "0" THEN SET errlvl=1

IF NOT "%errlvl%" == "0" (
	ECHO git.exe done with errors [%ERRORLEVEL%].
	ECHO Check if you have correct GIT repository at '%repodir%'.
	ECHO Auto-versioning step will not be performed.

	DEL /F /Q "%TMPFILE%.tmp1" 2>NUL
	DEL /F /Q "%TMPFILE%.tmp2" 2>NUL

	:: If we don't have appversion.h, we need to create it
	IF "%old_version%" == "" GOTO COMPARE
	EXIT /B 0
)

::
:: Read version and commit date from temp files
::
SET /P git_version=<"%TMPFILE%.tmp1"
SET /P git_date=<"%TMPFILE%.tmp2"
SET /P git_commit_hash=<"%TMPFILE%.tmp3"

DEL /F /Q "%TMPFILE%.tmp1" 2>NUL
DEL /F /Q "%TMPFILE%.tmp2" 2>NUL
DEL /F /Q "%TMPFILE%.tmp3" 2>NUL

:: Check if tree is dirty
SET git_dirty_tag=
ECHO %git_version%|find "-dirty" >nul
IF NOT ERRORLEVEL 1 (SET git_dirty_tag=+m)

:: Prepend variables
IF NOT [%arg_tag%] == [] (
	SET arg_tag=-%arg_tag%
)

IF NOT [%arg_metadata%] == [] (
	SET arg_metadata=+%arg_metadata%
)

SET new_version=%arg_major%.%arg_minor%.%arg_patch%%arg_tag%+%git_commit_hash%%arg_metadata%%git_dirty_tag%

::
:: Check if version has changed
::
:COMPARE
IF NOT "%new_version%"=="%old_version%" GOTO UPDATE
EXIT /B 0

::
:: Update appversion.h
::
:UPDATE
ECHO Updating %srcfile%, old version "%old_version%", new version "%new_version%".

ECHO #ifndef __APPVERSION_H__>"%srcfile%"
ECHO #define __APPVERSION_H__>>"%srcfile%"
ECHO.>>"%srcfile%"
ECHO // >>"%srcfile%"
ECHO // This file is generated automatically.>>"%srcfile%"
ECHO // Don't edit it.>>"%srcfile%"
ECHO // >>"%srcfile%"
ECHO.>>"%srcfile%"
ECHO // Version defines>>"%srcfile%"

ECHO #define APP_VERSION "%new_version%">>"%srcfile%"
ECHO #define APP_VERSION_C %arg_major%,%arg_minor%,%arg_patch%,^0>>"%srcfile%"

ECHO.>>"%srcfile%"
ECHO #define APP_VERSION_DATE "%git_date%">>"%srcfile%"

ECHO.>>"%srcfile%"
IF NOT "%new_specialbuild%" == "" (
	ECHO #define APP_VERSION_FLAGS VS_FF_SPECIALBUILD>>"%srcfile%"
	ECHO #define APP_VERSION_SPECIALBUILD "%new_specialbuild%">>"%srcfile%"
) ELSE (
	ECHO #define APP_VERSION_FLAGS 0x0L>>"%srcfile%"
)

ECHO.>>"%srcfile%"
ECHO #endif //__APPVERSION_H__>>"%srcfile%"

EXIT /B 0
