mkdir "%CMAKE_BUILD_DIR%"
cd "%CMAKE_BUILD_DIR%"
cmake -DAUTO_DEPLOY=0 %CMAKE_OPTIONS% ..
IF ERRORLEVEL 1 EXIT /b 1

if "%BUILD_TYPE%"=="0" (

	MSBuild.exe BugfixedHL.sln /t:client /p:PlatformTarget=x86 /p:Configuration=RelWithDebInfo /m
	IF ERRORLEVEL 1 EXIT /b 1
	..\scripts\AppVeyor_PackageClient.bat
	
) ELSE IF "%BUILD_TYPE%"=="1" (

	MSBuild.exe BugfixedHL.sln /t:client /p:PlatformTarget=x86 /p:Configuration=RelWithDebInfo /m
	IF ERRORLEVEL 1 EXIT /b 1
	..\scripts\AppVeyor_PackageClientVGUI2.bat
	
) ELSE IF "%BUILD_TYPE%"=="2" (

	MSBuild.exe BugfixedHL.sln /t:hl /p:PlatformTarget=x86 /p:Configuration=RelWithDebInfo /m
	IF ERRORLEVEL 1 EXIT /b 1
	..\scripts\AppVeyor_PackageServer.bat
	
) ELSE IF "%BUILD_TYPE%"=="3" (

	MSBuild.exe BugfixedHL.sln /t:bugfixedhl_amxx /p:PlatformTarget=x86 /p:Configuration=RelWithDebInfo /m
	IF ERRORLEVEL 1 EXIT /b 1
	..\scripts\AppVeyor_PackageAmxxModule.bat
	
)
