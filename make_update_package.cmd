@echo off

dll\GetVersion.exe "%~dp0\Release Unicode\IPTVChannelEditor.exe" \\StringFileInfo\\040904b0\\ProductVersion >AppVer.tmp
Set /P BUILD=<AppVer.tmp
del AppVer.tmp >nul 2>&1
set outfile=%~dp0\package\update.xml

echo prepare package folder...
md "package\%BUILD%" >nul 2>&1
pushd "package\%BUILD%"
copy "%~dp0\Release Unicode\IPTVChannelEditor.exe" . >nul
copy "%~dp0\Release Unicode\IPTVChannelEditorRUS.dll" . >nul
copy "%~dp0\Release Unicode\Updater.exe" . >nul
copy "%~dp0\dll\7za.dll" . >nul
copy "%~dp0\Changelog.md" . >nul
mklink /D dune_plugin "%~dp0\dune_plugin" >nul 2>&1
mklink /D playlists "%~dp0\playlists" >nul 2>&1

echo build update package...

7z a -xr!*.bin dune_plugin_%BUILD%.7z dune_plugin >nul
7z a -xr!*.bin playlists_%BUILD%.7z playlists >nul

call :header > %outfile%

call :crc_node app IPTVChannelEditor.exe	>>%outfile%
call :crc_node app IPTVChannelEditorRUS.dll	>>%outfile%
call :crc_node app Updater.exe				>>%outfile%
call :crc_node app 7za.dll					>>%outfile%

call :simple_node file Changelog.md					>>%outfile%
call :simple_node plugin dune_plugin_%BUILD%.7z		>>%outfile%
call :simple_node playlists playlists_%BUILD%.7z	>>%outfile%

call :footer >>%outfile%

echo build standard archive...
IPTVChannelEditor.exe /MakeAll .
7z a -xr!*.bin "%~dp0\package\dune_channel_editor_universal.7z" IPTVChannelEditor.exe IPTVChannelEditorRUS.dll Updater.exe 7za.dll ChangeLog.md %~dp0\dune_plugin %~dp0\playlists dune_plugin_*.zip >nul
del dune_plugin_*.zip >nul
rd dune_plugin /q
rd playlists /q
popd

echo done!
goto :EOF

:header
echo ^<?xml version="1.0" encoding="UTF-8"?^>
echo ^<update_info version="%BUILD%" /^>
echo ^<package^>
goto :EOF

:footer
echo ^</package^>
goto :EOF

:simple_node 
echo   ^<%1 name="%2" /^>
goto :EOF

:crc_node 
FileCrc.exe %2 > crc.tmp
Set /P CRC32=<crc.tmp
del crc.tmp >nul 2>&1
echo   ^<%1 name="%2" crc="%CRC32%" /^>
goto :EOF
