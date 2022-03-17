C:

setlocal

if "%1"=="SECONDINSTALL"  goto second
set builddir=build
 goto notsecond
 :second
set qmakeparam="SECONDINSTALL=true"
set innoparam="/DSECONDINSTALL"
set builddir=buildbeta
 :notsecond

set QtTools=C:\Qt\Tools\mingw810_32\bin
set QtKit=C:\Qt\5.15.2\mingw81_32\bin
set QtOpenSSL="C:\Qt\Tools\OpenSSL\Win_x86\bin"
set QtLicenses="C:\Qt\Licenses"
set HamlibPath="C:\Projects\hamlib-w32-4.5"

if exist %QtKit% goto kitInstalled

  echo %QtKit% is not installed
  goto endit

 :kitInstalled

set PATH=%QtKit%;%QtTools%;%PATH%

REM go up from the batch file directory to the "root"

set MROOT=%~dp0..\..
cd %MROOT%

git pull origin master

if not exist %builddir% mkdir %builddir%
cd %builddir%

qmake.exe %qmakeparam% ..\mqt\mqt.pro  -spec win32-g++
IF %ERRORLEVEL% == 0 goto make
  echo qmake failed; please fix errors and rebuild
  goto reset
:make

mingw32-make -j8 release

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

copy %MROOT%\%builddir%\MqtAppStarter\release\MqtAppStarter.exe Bin
copy %MROOT%\%builddir%\MqtChat\release\MqtChat.exe Bin
copy %MROOT%\%builddir%\MqtCluster\release\MqtCluster.exe Bin
REM copy %MROOT%\%builddir%\MqtControl\release\MqtControl.exe Bin
REM copy %MROOT%\%builddir%\MqtKeyer\release\MqtKeyer.exe Bin
copy %MROOT%\%builddir%\MqtKSTClient\release\MqtKSTClient.exe Bin
copy %MROOT%\%builddir%\MqtLogger\release\MqtLogger.exe Bin
copy %MROOT%\%builddir%\MqtMonitor\release\MqtMonitor.exe Bin
copy %MROOT%\%builddir%\MqtQrzServer\release\MqtQrzServer.exe Bin
copy %MROOT%\%builddir%\MqtRigControl\release\MqtRigControl.exe Bin
copy %MROOT%\%builddir%\MqtRigRecorder\release\MqtRigRecorder.exe Bin
copy %MROOT%\%builddir%\MqtRigSync\release\MqtRigSync.exe Bin
copy %MROOT%\%builddir%\MqtRotator\release\MqtRotator.exe Bin
copy %MROOT%\%builddir%\MqtServer\release\MqtServer.exe Bin

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
call powershell.exe "& {Invoke-WebRequest https://www.rsgbcc.org/vhf/vhfcontests20.xml -Outfile vhfcontests20.xml}"
call powershell.exe "& {Invoke-WebRequest https://www.rsgbcc.org/vhf/vhfcontests21.xml -Outfile vhfcontests21.xml}"

call powershell.exe "& {Invoke-WebRequest http://www.rsgbcc.org/vhf/hfcontests20.xml -Outfile hfcontests20.xml}"
call powershell.exe "& {Invoke-WebRequest http://www.rsgbcc.org/vhf/hfcontests21.xml -Outfile hfcontests21.xml}"

call powershell.exe "& {Invoke-WebRequest http://microwave.rsgbcc.org/microcontests20.xml -Outfile microcontests20.xml}"
call powershell.exe "& {Invoke-WebRequest http://microwave.rsgbcc.org/microcontests21.xml -Outfile microcontests21.xml}"


cd ../Bin

windeployqt.exe MqtAppStarter.exe
windeployqt.exe MqtChat.exe
windeployqt.exe MqtCluster.exe
REM windeployqt.exe MqtControl.exe
REM windeployqt.exe MqtKeyer.exe
windeployqt.exe MqtKSTClient.exe
windeployqt.exe MqtLogger.exe
windeployqt.exe MqtMonitor.exe
windeployqt.exe MqtQrzServer.exe
windeployqt.exe MqtRigControl.exe
windeployqt.exe MqtRigRecorder.exe
windeployqt.exe MqtRigSync.exe
windeployqt.exe MqtRotator.exe
windeployqt.exe MqtServer.exe

REM bin\translations now exists... we can build our translations

@ECHO OFF
for %%i in (en_GB fr_FR) do (
  for %%j in (MqtAppStarter MqtChat MqtCluster MqtKSTClient MqtLogger MqtMonitor MqtQrzServer MqtRigControl MqtRigRecorder MqtRigSync MqtRotator MqtServer) do (
  lconvert -verbose -o translations\%%j_%%i.qm ^
  %MROOT%\%builddir%\MqtUtils\release\minos_%%i.qm ^
  %MROOT%\%builddir%\TinyXML\release\minos_%%i.qm ^
  %MROOT%\%builddir%\XMPPLib\release\minos_%%i.qm ^
  %MROOT%\%builddir%\MqtBase\release\minos_%%i.qm ^
  %MROOT%\%builddir%\KeyerBase\release\minos_%%i.qm ^
  %MROOT%\%builddir%\%%j\release\minos_%%i.qm
  )
)

@ECHO ON

REM translations finished
cd ../..
mkdir Installer

xcopy /E /F /Y %MROOT%\mqt\Installer .\Installer

C:\"Program Files (x86)\Inno Setup 6\ISCC.exe" %innoparam% Installer\Minos2Install.iss
IF %ERRORLEVEL% == 0 goto reset
  echo Inno Setup failed; please fix errors and rebuild

:reset

cd %MROOT%\mqt\Installer

:endit
