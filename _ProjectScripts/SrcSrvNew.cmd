@echo off
setlocal
set ProgFiles=%ProgramFiles%
if /i "%PROCESSOR_ARCHITECTURE%" == "AMD64" set ProgFiles=%ProgramFiles(x86)%

set SRC_SERVER_FILES="%ProgFiles%\Windows Kits\10\Debuggers\x64\srcsrv"

if not exist %SRC_SERVER_FILES%\pdbstr.exe goto preq

echo Prepare files for Source Server
echo Source path:  %1
echo Symbols path: %2
echo Srcsrv path:  %SRC_SERVER_FILES%

rem echo %~dp0SourceIndexer.exe index -p %2 -s %1 -t %SRC_SERVER_FILES% -i %~dp0 -b GIT:gitextract.cmd
%~dp0\SourceIndexer.exe --pdb *.pdb --sourceRoot %1 --workingDir %2 --backend GitHub
echo Done
goto:EOF

:preq
echo Debugging Tools for Windows not installed
goto:EOF
