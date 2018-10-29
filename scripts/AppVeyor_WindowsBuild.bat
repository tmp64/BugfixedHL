mkdir "%CMAKE_BUILD_DIR%"
cd "%CMAKE_BUILD_DIR%"
cmake -DAUTO_DEPLOY=0 %CMAKE_OPTIONS% ..
IF %errorlevel% neq 0 EXIT /b %errorlevel%

IF "%SERVER_BUILD%"=="0" (
	MSBuild.exe BugfixedHL.sln /t:client /p:PlatformTarget=x86 /p:Configuration=Release /m
	IF %errorlevel% neq 0 EXIT /b %errorlevel%
	IF "%VGUI2_BUILD%"=="1" (
		..\scripts\AppVeyor_PackageClientVGUI2.bat
	) ELSE (
		..\scripts\AppVeyor_PackageClient.bat
	)
) ELSE (
	MSBuild.exe BugfixedHL.sln /t:hl /p:PlatformTarget=x86 /p:Configuration=Release /m
	IF %errorlevel% neq 0 EXIT /b %errorlevel%
	..\scripts\AppVeyor_PackageServer.bat
)
