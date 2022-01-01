@echo off
setlocal
set PATH=%PATH%;"C:\Program Files\Git\usr\bin\"
pushd "%~dp0"
findstr "#define BUILD" VerGIT.h >tmp
set /p str=<tmp
del tmp
set t=%str%
for /f "tokens=3*" %%a in ("%t%") do (
   set curbuild=%%a
)

for /f "delims=" %%a in ('git log --oneline ^| find "" /v /c') do @set BUILD=%%a
if %curbuild%==%BUILD% goto :EOF

for /F "tokens=2-4 delims= " %%a in ('git log -1 --pretty^=format:%%cD') do (
	echo set BUILD to %BUILD%
	sed 's/\$WCREV\$/%BUILD%/g;s/\$WCDATE=%%B %%d, %%Y\$/%%a %%b %%c/g;s/\$WCYEAR=%%Y\$/%%c/g' .\VerGIT.in.h >.\VerGIT.h
)
endlocal
popd
