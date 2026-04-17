C:

setlocal

if "%1"=="SECONDINSTALL"  goto second
set builddir=build\install
 goto notsecond
 :second
set qmakeparam="SECONDINSTALL=true"
set innoparam="/DSECONDINSTALL"
set builddir=build\installBeta
 :notsecond

set QtTools=C:\Qt\Tools\mingw810_32\bin
set QtKit=C:\Qt\5.15.2\mingw81_32\bin
set QtOpenSSL="C:\Qt\Tools\OpenSSL\Win_x86\bin"
set QtLicenses="C:\Qt\Licenses"
set HamlibPath="C:\Projects\hamlib-w32-4.7.1"
set MMVARIPath="C:\Ham\MMVARI"

if exist %QtKit% goto kitInstalled

  echo %QtKit% is not installed
  goto endit

 :kitInstalled

set PATH=%QtKit%;%QtTools%;%PATH%

REM go up from the batch file directory to the "root"

set MROOT=%~dp0..\..
cd %MROOT%

git pull origin master

rmdir /Q /S %builddir%
if not exist %builddir% mkdir %builddir%
cd %builddir%

qmake.exe %qmakeparam% ..\..\mqt\mqt.pro  -spec win32-g++
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
mkdir Configuration\FunctionKeyMessages
mkdir Configuration\Omnirig_Minos_Ini
mkdir Configuration\VoiceKeyer
mkdir Configuration\VoiceKeyer\CommonParams
mkdir Lists
mkdir Logs
mkdir Bin
mkdir Docs

for %%j in (MqtAppStarter MqtChat MqtCluster MqtDataModes MqtKSTClient MqtLogger MqtMonitor MqtPcCwKeyer MqtQrzServer MqtRigControl MqtRigRecorder MqtRigSync MqtRotator MqtServer MqtWinkeyer) do (
copy %MROOT%\%builddir%\%%j\release\%%j.exe Bin
)

copy %MMVARIPath%\MMVARI.ocx Bin

copy %HamlibPath%\bin\*.dll Bin
copy %HamlibPath%\bin\*.exe Bin
copy C:\Windows\SysWOW64\msvcr100.dll Bin 

copy %QtOpenSSL%\*.DLL Bin
copy %QtLicenses%\LICENSE-OPENSSL

copy %MROOT%\mqt\Docs\*.* Docs

xcopy /F /Y %MROOT%\mqt\ControlFiles\Configuration .\Configuration
xcopy /F /Y %MROOT%\mqt\ControlFiles\Configuration\WindowsFiles .\Configuration
xcopy /F /Y %MROOT%\mqt\ControlFiles\Configuration\Cluster .\Configuration\Cluster
xcopy /F /Y %MROOT%\mqt\ControlFiles\Configuration\FunctionKeyMessages .\Configuration\FunctionKeyMessages
xcopy /F /Y %MROOT%\mqt\ControlFiles\Configuration\VoiceKeyer\CommonParams .\Configuration\VoiceKeyer\CommonParams

xcopy /F /Y %MROOT%\mqt\ControlFiles\Omnirig_Minos_Ini .\Configuration\Omnirig_Minos_Ini

cd Configuration

call powershell.exe "& {Invoke-WebRequest https://www.country-files.com/cty/cty.dat -Outfile cty.dat}"
call powershell.exe "& {Invoke-WebRequest https://www.rsgbcc.org/cgi-bin/vhfenter.pl?afsdownload=y -Outfile clublist.txt}"
call powershell.exe "& {Invoke-WebRequest https://www.rsgbcc.org/vhf/vhfcontests26.xml -Outfile vhfcontests26.xml}"
call powershell.exe "& {Invoke-WebRequest https://www.rsgbcc.org/vhf/vhfcontests25.xml -Outfile vhfcontests25.xml}"

call powershell.exe "& {Invoke-WebRequest http://www.rsgbcc.org/vhf/hfcontests26.xml -Outfile hfcontests26.xml}"
call powershell.exe "& {Invoke-WebRequest http://www.rsgbcc.org/vhf/hfcontests25.xml -Outfile hfcontests25.xml}"

call powershell.exe "& {Invoke-WebRequest http://microwave.rsgbcc.org/microcontests26.xml -Outfile microcontests26.xml}"
call powershell.exe "& {Invoke-WebRequest http://microwave.rsgbcc.org/microcontests25.xml -Outfile microcontests25.xml}"

call powershell.exe "& {Invoke-WebRequest http://bartg.rsgbcc.org/bartgcontests26.xml -Outfile bartgcontests26.xml}"
call powershell.exe "& {Invoke-WebRequest http://bartg.rsgbcc.org/bartgcontests25.xml -Outfile bartgcontests25.xml}"


cd ../Bin

for %%j in (MqtAppStarter MqtChat MqtCluster MqtDataModes MqtKSTClient MqtPcCwKeyer MqtQrzServer MqtRigControl MqtRigRecorder MqtRigSync MqtRotator MqtServer MqtWinkeyer) do (
windeployqt.exe %%j.exe
)
for %%j in ( MqtLogger MqtMonitor) do (
windeployqt.exe  --qmldir %MROOT%/mqt/MqtBase/QSOView  %%j.exe
)

REM bin\translations now exists... we can build our translations

@ECHO OFF
for %%i in (en_GB fr_FR) do (
  for %%j in (MqtAppStarter MqtChat MqtCluster MqtDataModes MqtKSTClient MqtLogger MqtMonitor MqtPcCwKeyer MqtQrzServer MqtRigControl MqtRigRecorder MqtRigSync MqtRotator MqtServer MqtWinkeyer) do (
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
