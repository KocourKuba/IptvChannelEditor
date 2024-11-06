@echo off
setlocal

set SYMSRV_APP="%ProgramFiles(x86)%\Windows Kits\10\Debuggers\x64\symstore.exe"
set SYMSTORE=\\DISKSTATION2\SymbolStore\
set DEV_ENV="%ProgramW6432%\Microsoft Visual Studio\2022\Professional\Common7\IDE\devenv.com"

rem ********************************************************
:build_project

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

echo "%BUILD_PATH%\%BUILD_NAME%.exe"

echo @%DEV_ENV% %BUILD_NAME%.sln /Rebuild "%BUILD_TYPE%|%BUILD_PLATFORM%" /Project %BUILD_NAME%.vcxproj /OUT build.log >build.bat
echo @if NOT exist "%BUILD_PATH%\%BUILD_NAME%.exe" pause >>build.bat
echo @exit >>build.bat
start /wait build.bat
del build.bat

if exist "%BUILD_PATH%" goto getversion

set ERROR=Error %BUILD_NAME%.exe is not compiled
endlocal & set ERROR=%ERROR%
echo %ERROR%
goto :EOF

:getversion
rem Get app version
bin\GetVersion.exe "%BUILD_PATH%\%BUILD_NAME%.exe" \\StringFileInfo\\040904b0\\ProductVersion >AppVer.tmp
set /P BUILD=<AppVer.tmp
del AppVer.tmp >nul 2>&1
FOR /f "tokens=1,2 delims=." %%a IN ("%BUILD%") do set MAJOR=%%a&set MINOR=%%b

set outfile=%ROOT%package\update.xml
echo %BUILD%

call :update_source
call :upload_pdb

set pkg=%ROOT%package
set build_pkg=%pkg%\%BUILD%

echo prepare package folder...
md "%pkg%" >nul 2>&1
md "%build_pkg%" >nul 2>&1

copy "%BUILD_PATH%\%BUILD_NAME%.exe"					"%build_pkg%" >nul
copy "%BUILD_PATH%\%BUILD_NAME%RUS.dll"					"%build_pkg%" >nul
copy "%BUILD_PATH%\Updater.exe"							"%build_pkg%" >nul
copy "%ROOT%%DLL_PATH%\7z.dll"							"%build_pkg%" >nul
copy "%ROOT%BugTrap\bin\%BUGTRAP%.dll"					"%build_pkg%" >nul
copy "%ROOT%BugTrap\pkg\dbghelp.dll"					"%build_pkg%" >nul
copy "%ROOT%\defaults_%MAJOR%.%MINOR%.json"				"%build_pkg%" >nul
copy "%ROOT%dune_plugin\changelog.md"					"%build_pkg%" >nul

copy "%ROOT%dune_plugin\changelog.md" 					"%pkg%\changelog.md" >nul
copy "%ROOT%dune_plugin\changelog.md" 					"%pkg%\changelog.md.%BUILD%" >nul

echo build dune_plugin package...
del %build_pkg%\dune_plugin.pkg >nul 2>&1
pushd dune_plugin
7z a %build_pkg%\dune_plugin.pkg >nul
popd

echo build picons package...
del %build_pkg%\picons.pkg >nul 2>&1
7z a %build_pkg%\picons.pkg icons\>nul

echo build channels list package...
del %build_pkg%\ChannelsLists.pkg >nul 2>&1
pushd ChannelsLists
7z a -xr!*.bin -xr!.gitignore -xr!custom %build_pkg%\ChannelsLists.pkg >nul
popd

pushd "%build_pkg%"

call :header > %outfile%

echo ^<package^> >>%outfile%
call :add_node %BUILD_NAME%.exe					>>%outfile%
call :add_node %BUILD_NAME%RUS.dll				>>%outfile%
call :add_node Updater.exe						>>%outfile%
call :add_node 7z.dll							>>%outfile%
call :add_node %BUGTRAP%.dll					>>%outfile%
call :add_node dbghelp.dll						>>%outfile%
call :add_node changelog.md						>>%outfile%
call :add_node defaults_%MAJOR%.%MINOR%.json	>>%outfile%
call :add_node dune_plugin.pkg					>>%outfile%
call :add_node picons.pkg						>>%outfile%
call :add_node ChannelsLists.pkg				>>%outfile%
echo ^</package^> >>%outfile%
copy /Y "%outfile%" "%outfile%.%BUILD%" >nul

echo %BUILD_NAME%.exe				>packing.lst
echo %BUILD_NAME%RUS.dll			>>packing.lst
echo Updater.exe					>>packing.lst
echo 7z.dll 						>>packing.lst
echo %BUGTRAP%.dll					>>packing.lst
echo dbghelp.dll					>>packing.lst
echo changelog.md					>>packing.lst
echo defaults_%MAJOR%.%MINOR%.json	>>packing.lst
echo dune_plugin.pkg				>>packing.lst
echo picons.pkg						>>packing.lst
echo ChannelsLists.pkg				>>packing.lst

echo "remove %ROOT%package\dune_channel_editor_universal.7z" >nul
del "%ROOT%package\dune_channel_editor_universal.7z" >nul

7z a -xr!*.bin -xr!custom "%ROOT%package\dune_channel_editor_universal.7z" @packing.lst >nul
copy /Y "%ROOT%package\dune_channel_editor_universal.7z" "%ROOT%package\dune_channel_editor_universal.7z.%BUILD%" >nul
del packing.lst >nul 2>&1
del dune_plugin_*.zip >nul 2>&1
popd

echo done!
endlocal & set BUILD=%BUILD%
goto :EOF

:update_source

call %ROOT%_ProjectScripts\SrcSrvNew.cmd %ROOT% "%BUILD_PATH%"
call %ROOT%_ProjectScripts\SrcSrvNew.cmd %ROOT%BugTrap "%ROOT%BugTrap\bin"
goto :EOF

:upload_pdb

echo Upload PDB to symbol server

set dest=%ROOT%\ArchivePDB\32

mkdir "%dest%" >nul 2>&1

echo IPTVChannelEditor
copy "%BUILD_PATH%\%BUILD_NAME%.exe" "%dest%" >nul
copy "%BUILD_PATH%\%BUILD_NAME%.pdb" "%dest%" >nul

echo Updater
copy "%BUILD_PATH%\Updater.exe" "%dest%" >nul
copy "%BUILD_PATH%\Updater.pdb" "%dest%" >nul

pushd "%dest%"
dir /b /O:N *.exe *.dll *.pdb > upload.lst

%SYMSRV_APP% add /o /t "%BUILD_NAME%" /v "%BUILD% (%BUILD_PLATFORM%)" /d Transaction.log /3 /f "@upload.lst" /s %SYMSTORE% /compress
del /q *.exe *.dll *.pdb *.lst >nul 2>&1
popd

git.exe push --force --tags  -- "origin" master:master

echo done!
goto :EOF

:header
echo ^<?xml version="1.0" encoding="UTF-8"?^>
echo ^<update_info version="%BUILD%" /^>
goto :EOF

:add_node 
%ROOT%bin\FileCrc.exe -d %1 > crc.tmp
Set /P CRC32=<crc.tmp
del crc.tmp >nul 2>&1
if [%2]==[] (
echo   ^<file name="%1" hash="%CRC32%"^>^</file^>
) else (
echo   ^<file name="%1" hash="%CRC32%" opt="%2"^>^</file^>
)
goto :EOF
