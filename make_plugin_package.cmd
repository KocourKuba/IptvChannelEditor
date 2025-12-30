@echo off
setlocal

set ROOT=%~dp0
set BUILD_TYPE=Release Unicode
set BUILD_PLATFORM=x64
if "%BUILD_PLATFORM%" == "x64" (
	set BUGTRAP=BugTrapU-x64
	set DLL_PATH=dll64
) else (
	set BUGTRAP=BugTrapU
	set DLL_PATH=dll
)

del build.log >nul 2>&1

call UpdateVer.cmd

set BUILD_NAME=IPTVChannelEditor
set BUILD_PATH=%ROOT%%BUILD_PLATFORM%\%BUILD_TYPE%

:getversion
rem Get app version
bin\GetVersion.exe "%BUILD_PATH%\%BUILD_NAME%.exe" \\StringFileInfo\\040904b0\\ProductVersion >AppVer.tmp
set /P FULL_VER=<AppVer.tmp
del AppVer.tmp >nul 2>&1
FOR /f "tokens=1,2,3 delims=." %%a IN ("%FULL_VER%") do set MAJOR=%%a&set MINOR=%%b&set BUILD=%%c

echo %FULL_VER%

echo build dune_plugin package...
pushd dune_plugin
7z a ..\dune_plugin.pkg >nul
popd

echo ^<?xml version="1.0" encoding="UTF-8"?^> >update.xml
echo ^<update_info version="%FULL_VER%" /^> >>update.xml
echo ^<package^> >>update.xml
call :add_node dune_plugin.pkg
echo ^</package^> >>update.xml
goto :EOF

:add_node
%ROOT%bin\FileCrc.exe -d %1 > crc.tmp
Set /P CRC32=<crc.tmp
del crc.tmp >nul 2>&1
if [%2]==[] (
echo   ^<file name="%1" hash="%CRC32%" build="%BUILD%"^>^</file^> >>update.xml
) else (
echo   ^<file name="%1" hash="%CRC32%" build="%BUILD%" opt="%2"^>^</file^> >>update.xml
)
goto :EOF
