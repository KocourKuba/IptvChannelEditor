@echo OFF
setlocal
set ProgFiles=%ProgramFiles%
if /i "%PROCESSOR_ARCHITECTURE%" == "AMD64" set ProgFiles=%ProgramFiles(x86)%

echo Build localized %2.slp ...
copy "%1%2.slp" "%1_%2.slp" >nul 2>&1
"%ProgFiles%\Sisulizer 4\slmake.exe" build -lang:ru; "%1_%2.slp" -q -w >nul 2>&1

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
del "%1.slp" >nul 2>&1
del "%1.~slp" >nul 2>&1
del "%1.sds" >nul 2>&1
goto :EOF
