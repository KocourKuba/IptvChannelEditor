@echo off
setlocal
set PATH=%PATH%;"C:\Program Files\Git\usr\bin\"

for /f "delims=" %%a in ('git log --oneline ^| find "" /v /c') do @set BUILD=%%a
for /F "tokens=2-4 delims= " %%a in ('git log -1 --pretty^=format:%%cD') do (
sed 's/\$WCREV\$/%BUILD%/g;s/\$WCDATE=%%B %%d, %%Y\$/%%a %%b %%c/g;s/\$WCYEAR=%%Y\$/%%c/g' .\VerGIT.in.h >.\VerGIT.h
)

endlocal
