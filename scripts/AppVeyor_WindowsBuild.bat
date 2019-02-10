mkdir "%CMAKE_BUILD_DIR%"
cd "%CMAKE_BUILD_DIR%"
cmake -DAUTO_DEPLOY=0 %CMAKE_OPTIONS% ..
IF ERRORLEVEL 1 EXIT /b 1

IF "%SERVER_BUILD%"=="0" (
	MSBuild.exe BugfixedHL.sln /t:client /p:PlatformTarget=x86 /p:Configuration=RelWithDebInfo /m
	IF ERRORLEVEL 1 EXIT /b 1
	IF "%VGUI2_BUILD%"=="1" (
		..\scripts\AppVeyor_PackageClientVGUI2.bat
	) ELSE (
		..\scripts\AppVeyor_PackageClient.bat
	)
) ELSE (
	MSBuild.exe BugfixedHL.sln /t:hl /p:PlatformTarget=x86 /p:Configuration=RelWithDebInfo /m
	IF ERRORLEVEL 1 EXIT /b 1
	..\scripts\AppVeyor_PackageServer.bat
)
