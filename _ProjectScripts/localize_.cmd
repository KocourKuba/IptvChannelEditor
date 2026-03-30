@echo OFF
setlocal
set LocalizeApp="%ProgramFiles%\Soluling\solumake.exe"

echo Build localized %2.ntp ...
copy "%1%2.ntp" "%1_%2.ntp" >nul 2>&1
%LocalizeApp% scan "%1_%2.ntp" -q -w >nul 2>&1
%LocalizeApp% build -lang:ru; -log "%1_%2.ntp" -q -w >solumake.log

if ERRORLEVEL 2 (
echo Builded with errors
call :cleanup %1_%2
endlocal
exit /B 1
)

if ERRORLEVEL 1 (
echo Builded with warnings
)

call :cleanup %1_%2
endlocal
exit /B 0

:cleanup
del "%1.ntp" >nul 2>&1
del "%1.~ntp" >nul 2>&1
del "%1.nds" >nul 2>&1
goto :EOF
