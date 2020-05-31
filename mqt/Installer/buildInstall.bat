C:

setlocal

set QtTools=C:\Qt\Tools\mingw730_32\bin
set QtKit="C:\Qt\5.12.6\mingw73_32\bin"
set QtOpenSSL="C:\Qt\Tools\OpenSSL\Win_x86\bin"
set QtLicenses="C:\Qt\Tools\Licenses"
set HamlibPath="C:\Projects\hamlib-w32-3.3"

if exist %QtKit% goto kitInstalled

  echo %QtKit% is not installed
  goto endit

 :kitInstalled

set PATH=%QtKit%;%QtTools%;%PATH%

REM go up from the batch file directory to the "root"

set MROOT=%~dp0..\..
cd %MROOT%

git pull origin master

if not exist build mkdir build
cd build

qmake.exe ..\mqt\mqt.pro  -spec win32-g++
IF %ERRORLEVEL% == 0 goto make
  echo qmake failed; please fix errors and rebuild
  goto reset
:make

mingw32-make release

IF %ERRORLEVEL% == 0 goto installer
  echo mingw32-make failed; please fix errors and rebuild
  goto reset
:installer

cd \
if not exist temp mkdir temp
cd temp

if exist mqtInstaller rmdir /Q /S mqtInstaller
mkdir mqtInstaller
cd mqtInstaller
mkdir installFiles
cd installFiles

mkdir Configuration
mkdir Configuration\Cluster
mkdir Lists
mkdir Logs
mkdir Bin
mkdir Docs
mkdir Help

copy %MROOT%\build\MqtAppStarter\release\MqtAppStarter.exe Bin
copy %MROOT%\build\MqtChat\release\MqtChat.exe Bin
copy %MROOT%\build\MqtCluster\release\MqtCluster.exe Bin
REM copy %MROOT%\build\MqtControl\release\MqtControl.exe Bin
REM copy %MROOT%\build\MqtKeyer\release\MqtKeyer.exe Bin
copy %MROOT%\build\MqtKSTClient\release\MqtKSTClient.exe Bin
copy %MROOT%\build\MqtLogger\release\MqtLogger.exe Bin
copy %MROOT%\build\MqtMonitor\release\MqtMonitor.exe Bin
copy %MROOT%\build\MqtRigControl\release\MqtRigControl.exe Bin
copy %MROOT%\build\MqtRotator\release\MqtRotator.exe Bin
copy %MROOT%\build\MqtServer\release\MqtServer.exe Bin

copy %HamlibPath%\bin\*.dll Bin
copy %HamlibPath%\bin\*.exe Bin
copy C:\Windows\SysWOW64\msvcr100.dll Bin 

copy %QtOpenSSL%\*.DLL Bin
copy %QtLicenses%\LICENSE-OPENSSL

copy %MROOT%\mqt\Docs\*.* Docs
copy %MROOT%\mqt\Help\*.* Help

xcopy /F /Y %MROOT%\mqt\ControlFiles\Configuration .\Configuration
xcopy /F /Y %MROOT%\mqt\ControlFiles\Configuration\WindowsFiles .\Configuration
xcopy /F /Y %MROOT%\mqt\ControlFiles\Configuration\Cluster .\Configuration\Cluster

cd Configuration

call powershell.exe "& {Invoke-WebRequest https://www.country-files.com/cty/cty.dat -Outfile cty.dat}"
call powershell.exe "& {Invoke-WebRequest https://www.rsgbcc.org/vhf/vhfcontests19.xml -Outfile vhfcontests19.xml}"
call powershell.exe "& {Invoke-WebRequest https://www.rsgbcc.org/vhf/vhfcontests20.xml -Outfile vhfcontests20.xml}"


cd ../Bin

windeployqt.exe MqtAppStarter.exe
windeployqt.exe MqtChat.exe
windeployqt.exe MqtCluster.exe
REM windeployqt.exe MqtControl.exe
REM windeployqt.exe MqtKeyer.exe
windeployqt.exe MqtKSTClient.exe
windeployqt.exe MqtLogger.exe
windeployqt.exe MqtMonitor.exe
windeployqt.exe MqtRigControl.exe
windeployqt.exe MqtRotator.exe
windeployqt.exe MqtServer.exe

REM bin\translations now exists... we can build our translations

@ECHO OFF
for %%i in (en_GB fr_FR) do (
  for %%j in (MqtAppStarter MqtChat MqtCluster MqtKSTClient MqtLogger MqtMonitor MqtRigControl MqtRotator MqtServer) do (
  lconvert -verbose -o translations\%%j_%%i.qm ^
  %MROOT%\build\MqtUtils\release\minos_%%i.qm ^
  %MROOT%\build\TinyXML\release\minos_%%i.qm ^
  %MROOT%\build\XMPPLib\release\minos_%%i.qm ^
  %MROOT%\build\MqtBase\release\minos_%%i.qm ^
  %MROOT%\build\%%j\release\minos_%%i.qm
  )
)

@ECHO ON

REM translations finished
cd ../..
mkdir Installer

xcopy /E /F /Y %MROOT%\mqt\Installer .\Installer

C:\"Program Files (x86)\Inno Setup 6\ISCC.exe" Installer\Minos2Install.iss
IF %ERRORLEVEL% == 0 goto reset
  echo Inno Setup failed; please fix errors and rebuild

:reset

cd %MROOT%\mqt\Installer

:endit
