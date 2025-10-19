@echo off
setlocal

rem ********************************************************

for /f "delims=" %%a in ('git log --oneline ^| find "" /v /c') do @set BUILD=%%a

echo %BUILD%

set outfile=%ROOT%package\picons.xml

set pkg=%ROOT%package

echo build picons package...
del picons.pkg >nul 2>&1
7z a picons.pkg icons\>nul

call :header > %outfile%

echo ^<package^> >>%outfile%
call :add_node picons.pkg >>%outfile%
echo ^</package^> >>%outfile%

copy /Y picons.pkg %pkg% >nul
del picons.pkg >nul 2>&1

echo done!
endlocal & set BUILD=%BUILD%
goto :EOF

:header
echo ^<?xml version="1.0" encoding="UTF-8"?^>
echo ^<update_info" /^>
goto :EOF

:add_node
%ROOT%bin\FileCrc.exe -d %1 > crc.tmp
Set /P CRC32=<crc.tmp
del crc.tmp >nul 2>&1
if [%2]==[] (
echo   ^<file name="%1" hash="%CRC32%" build="%BUILD%"^>^</file^>
) else (
echo   ^<file name="%1" hash="%CRC32%" build="%BUILD%" opt="%2"^>^</file^>
)
goto :EOF
