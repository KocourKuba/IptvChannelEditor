@echo off
set ProgFiles=%ProgramFiles%
if /i "%PROCESSOR_ARCHITECTURE%" == "AMD64" set ProgFiles=%ProgramFiles(x86)%

set SYMSRV_APP="%ProgFiles%\Windows Kits\10\Debuggers\x64\symstore.exe"
set SYMSTORE=\\DISKSTATION2\SymbolStore\

rem Get app version
dll\GetVersion.exe "%~dp0\Release Unicode\IPTVChannelEditor.exe" \\StringFileInfo\\040904b0\\ProductVersion >AppVer.tmp
set /P BUILD=<AppVer.tmp
del AppVer.tmp >nul 2>&1
set outfile=%~dp0\package\update.xml
echo %BUILD%

call :update_source
call :upload_pdb

echo prepare package folder...
set pkg=package\%BUILD%
md "%~dp0\%pkg%" >nul 2>&1
copy "%~dp0\Release Unicode\IPTVChannelEditor.exe"		"%pkg%" >nul
copy "%~dp0\Release Unicode\IPTVChannelEditorRUS.dll"	"%pkg%" >nul
copy "%~dp0\Release Unicode\Updater.exe"				"%pkg%" >nul
copy "%~dp0\dll\7za.dll"								"%pkg%" >nul
copy "%~dp0\BugTrap\bin\BugTrapU.dll"					"%pkg%" >nul
copy "%~dp0\BugTrap\pkg\dbghelp.dll"					"%pkg%" >nul
copy "%~dp0\Changelog.md"								"%pkg%" >nul
copy "%~dp0\Changelog.md" "%~dp0\package\Changelog.md" >nul
copy "%~dp0\Changelog.md" "%~dp0\package\Changelog.md.%BUILD%" >nul

pushd "package\%BUILD%"
mklink /D dune_plugin "%~dp0\dune_plugin" >nul 2>&1
mklink /D playlists "%~dp0\playlists" >nul 2>&1

echo build update package...

7z a -xr!*.bin dune_plugin.7z dune_plugin >nul
7z a -xr!*.bin playlists.7z playlists >nul

call :header > %outfile%

echo ^<package^> >>%outfile%
call :add_node IPTVChannelEditor.exe		>>%outfile%
call :add_node IPTVChannelEditorRUS.dll		>>%outfile%
call :add_node Updater.exe					>>%outfile%
call :add_node 7za.dll						>>%outfile%
call :add_node BugTrapU.dll					>>%outfile%
call :add_node dbghelp.dll					>>%outfile%
call :add_node Changelog.md					>>%outfile%
call :add_node dune_plugin.7z				>>%outfile%
call :add_node playlists.7z	true			>>%outfile%
echo ^</package^> >>%outfile%
copy /Y "%outfile%" "%outfile%.%BUILD%" >nul

echo build standard archive...
IPTVChannelEditor.exe /MakeAll /NoEmbed /NoCustom .

echo IPTVChannelEditor.exe		>packing.lst
echo IPTVChannelEditorRUS.dll	>>packing.lst
echo Updater.exe				>>packing.lst
echo 7za.dll					>>packing.lst
echo BugTrapU.dll				>>packing.lst
echo dbghelp.dll				>>packing.lst
echo Changelog.md				>>packing.lst
echo %~dp0\dune_plugin			>>packing.lst
echo %~dp0\playlists			>>packing.lst
echo dune_plugin_*.zip			>>packing.lst

7z a -xr!*.bin "%~dp0\package\dune_channel_editor_universal.7z" @packing.lst >nul
copy /Y "%~dp0\package\dune_channel_editor_universal.7z" "%~dp0\package\dune_channel_editor_universal.7z.%BUILD%" >nul
del packing.lst >nul 2>&1
del dune_plugin_*.zip >nul 2>&1
rd dune_plugin /q
rd playlists /q
popd

echo done!
goto :EOF

:update_source

call %~dp0_ProjectScripts\SrcSrvNew.cmd %~dp0 "%~dp0Release Unicode"
call %~dp0_ProjectScripts\SrcSrvNew.cmd %~dp0BugTrap "%~dp0BugTrap\bin"
goto :EOF

:upload_pdb

echo Upload PDB to symbol server

set src_dir=%SOURCEDIR%%1
set dest=%~dp0\ArchivePDB\32

mkdir "%dest%" >nul 2>&1

echo IPTVChannelEditor
copy "%~dp0\Release Unicode\IPTVChannelEditor.exe" "%dest%" >nul
copy "%~dp0\Release Unicode\IPTVChannelEditor.pdb" "%dest%" >nul

echo Updater
copy "%~dp0\Release Unicode\Updater.exe" "%dest%" >nul
copy "%~dp0\Release Unicode\Updater.pdb" "%dest%" >nul

pushd "%dest%"
dir /b /O:N *.exe *.dll *.pdb > upload.lst

%SYMSRV_APP% add /o /t "IPTVChannelEditor" /v "%BUILD% (x32)" /d Transaction.log /3 /f "@upload.lst" /s %SYMSTORE% /compress
del /q *.exe *.dll *.pdb *.lst >nul 2>&1
popd

echo done!
goto :EOF

:header
echo ^<?xml version="1.0" encoding="UTF-8"?^>
echo ^<update_info version="%BUILD%" /^>
goto :EOF

:add_node 
%~dp0\dll\FileCrc.exe -d %1 > crc.tmp
Set /P CRC32=<crc.tmp
del crc.tmp >nul 2>&1
if [%2]==[] (
echo   ^<file name="%1" hash="%CRC32%"^>^</file^>
) else (
echo   ^<file name="%1" hash="%CRC32%" opt="%2"^>^</file^>
)
goto :EOF
