C:

setlocal

set QtTools=C:\Qt\Tools\mingw530_32\bin
set QtKit="C:\Qt\5.10.1\mingw53_32\bin"

if exist %QtKit% goto kitInstalled

  echo %QtKit% is not installed
  goto endit

 :kitInstalled

set PATH=%QtKit%;%QtTools%;%PATH%

REM go up from the batch file directory to the "root"

set MROOT=%~dp0..\..
cd %MROOT%

git pull origin

if not exist build mkdir build
cd build

qmake.exe ..\mqt\mqt.pro

mingw32-make release

cd \
if not exist temp mkdir temp
cd temp

if exist mqtInstaller rmdir /Q /S mqtInstaller
mkdir mqtInstaller
cd mqtInstaller
mkdir installFiles
cd installFiles

mkdir Configuration
mkdir Lists
mkdir Logs
mkdir Bin

copy %MROOT%\build\MqtAppStarter\release\MqtAppStarter.exe Bin
copy %MROOT%\build\MqtChat\release\MqtChat.exe Bin
REM copy %MROOT%\build\MqtControl\release\MqtControl.exe Bin
REM copy %MROOT%\build\MqtKeyer\release\MqtKeyer.exe Bin
copy %MROOT%\build\MqtLogger\release\MqtLogger.exe Bin
copy %MROOT%\build\MqtMonitor\release\MqtMonitor.exe Bin
copy %MROOT%\build\MqtRigControl\release\MqtRigControl.exe Bin
copy %MROOT%\build\MqtRotator\release\MqtRotator.exe Bin
copy %MROOT%\build\MqtServer\release\MqtServer.exe Bin

copy C:\Projects\hamlib-w32-3.1\bin\*.dll Bin

xcopy /F /Y %MROOT%\mqt\ControlFiles\Configuration .\Configuration
xcopy /F /Y %MROOT%\mqt\ControlFiles\Configuration\WindowsFiles .\Configuration

cd Configuration

\bin\wget\wget http://www.country-files.com/cty/cty.dat -O cty.dat
\bin\wget\wget http://www.rsgbcc.org/vhf/vhfcontests16.xml -O vhfcontests16.xml
\bin\wget\wget http://www.rsgbcc.org/vhf/vhfcontests17.xml -O vhfcontests17.xml
\bin\wget\wget http://www.rsgbcc.org/vhf/vhfcontests18.xml -O vhfcontests18.xml

cd ../Bin

windeployqt.exe MqtAppStarter.exe
windeployqt.exe MqtChat.exe
REM windeployqt.exe MqtControl.exe
REM windeployqt.exe MqtKeyer.exe
windeployqt.exe MqtLogger.exe
windeployqt.exe MqtMonitor.exe
windeployqt.exe MqtRigControl.exe
windeployqt.exe MqtRotator.exe
windeployqt.exe MqtServer.exe

cd ../..
mkdir Installer

xcopy /E /F /Y %MROOT%\mqt\Installer .\Installer

C:\"Program Files (x86)\Inno Setup 5\ISCC.exe" Installer\Minos2Install.iss


cd %MROOT%\mqt\Installer

:endit
