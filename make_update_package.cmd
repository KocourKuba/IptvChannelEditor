@echo off
dll\GetVersion.exe "%~dp0\Release Unicode\IPTVChannelEditor.exe" \\StringFileInfo\\040904b0\\ProductVersion >AppVer.tmp
set /P BUILD=<AppVer.tmp
del AppVer.tmp >nul 2>&1
set outfile=%~dp0\package\update.xml
echo %BUILD%

echo prepare package folder...
set pkg=package\%BUILD%
md "%~dp0\%pkg%" >nul 2>&1
copy "%~dp0\Release Unicode\IPTVChannelEditor.exe" "%pkg%" >nul
copy "%~dp0\Release Unicode\IPTVChannelEditorRUS.dll" "%pkg%" >nul
copy "%~dp0\Release Unicode\Updater.exe" "%pkg%" >nul
copy "%~dp0\dll\7za.dll" "%pkg%" >nul
copy "%~dp0\Changelog.md" "%pkg%" >nul
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
call :add_node Changelog.md					>>%outfile%
call :add_node dune_plugin.7z				>>%outfile%
call :add_node playlists.7z	true			>>%outfile%
echo ^</package^> >>%outfile%
copy /Y "%outfile%" "%outfile%.%BUILD%" >nul

echo build standard archive...
IPTVChannelEditor.exe /MakeAll .
7z a -xr!*.bin "%~dp0\package\dune_channel_editor_universal.7z" IPTVChannelEditor.exe IPTVChannelEditorRUS.dll Updater.exe 7za.dll ChangeLog.md %~dp0\dune_plugin %~dp0\playlists dune_plugin_*.zip >nul
copy /Y "%~dp0\package\dune_channel_editor_universal.7z" "%~dp0\package\dune_channel_editor_universal.7z.%BUILD%" >nul
del dune_plugin_*.zip >nul 2>&1
rd dune_plugin /q
rd playlists /q
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
