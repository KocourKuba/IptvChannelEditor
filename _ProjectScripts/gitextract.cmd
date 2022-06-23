@echo off
rem "App"
rem "\\diskstation2/GitServer/App.git"
rem "516b4be5e61c3bf1ee0990688d6d055f01af9031"
rem "Include/MonitorDialog.h"
rem "D:\Works\OpenSource\GitHub\SourceIndexer\SourceIndexerGui\bin\Debug\516b4be5e61c3bf1ee0990688d6d055f01af9031\Include/MonitorDialog.h"

setlocal
set REPOS_ROOT=C:\Temp
rem "App"
if ()==(%1) exit /b

rem "\\diskstation2/GitServer/App.git"
if ()==(%2) exit /b

rem "516b4be5e61c3bf1ee0990688d6d055f01af9031"
if ()==(%3) exit /b

rem "Include/MonitorDialog.h"
if ()==(%4) exit /b

rem "D:\Works\OpenSource\GitHub\SourceIndexer\SourceIndexerGui\bin\Debug\516b4be5e61c3bf1ee0990688d6d055f01af9031\Source/stdafx.cpp"
if ()==(%5) exit /b

set GIT_WORKING_DIR="%REPOS_ROOT%\%~1.LocalRepo"
set GIT_ORIGIN_URL="%~2"
set GIT_REPO_OBJ_PATH="%~3:%~4"
set GIT_CONTENTS_OUTPUT="%~5"

if exist %GIT_WORKING_DIR% goto update

rem Create repository if not exist
md %GIT_WORKING_DIR%
git --git-dir=%GIT_WORKING_DIR% init
git --git-dir=%GIT_WORKING_DIR% fetch %GIT_ORIGIN_URL%
goto show

:update
for /f "usebackq delims=" %%i in (`git --no-pager --git-dir=%%GIT_WORKING_DIR%% show "%%GIT_REPO_OBJ_PATH%%"`) do ( goto show )
echo *Updating...*
git --git-dir=%GIT_WORKING_DIR% fetch %GIT_ORIGIN_URL%

:show
if not exist %~dp5 mkdir %~dp5

git --no-pager --git-dir=%GIT_WORKING_DIR% show %GIT_REPO_OBJ_PATH% > %GIT_CONTENTS_OUTPUT%

:cleanup
endlocal
