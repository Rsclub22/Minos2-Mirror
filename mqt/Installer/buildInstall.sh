#!/bin/bash

#cd ~/Minos2
SCRIPT=$(readlink -f $0)
DIR=$(echo ${SCRIPT%/*}/../..)
cd $DIR

if [[ "$1" == "SECONDINSTALL" ]]; then
    BUILDDIR="buildBeta"
    QMAKEPARAM="SECONDINSTALL=true"
    RUNTIME="runtime-beta"
    INTERMEDIATE="minos-runtime-beta"
else
    BUILDDIR="build"
    RUNTIME="runtime"
    INTERMEDIATE="minos-runtime"
fi

git pull origin

if [ ! -d ./$BUILDDIR ]; then
   mkdir $BUILDDIR
fi

cd $BUILDDIR

qmake $QMAKEPARAM ../mqt/mqt.pro

retVal=$?
if [ $retVal -ne 0 ]; then
    echo "qmake failed; please fix errors and rebuild"
    exit $retVal
fi

make -j2
retVal=$?
if [ $retVal -ne 0 ]; then
    echo "make failed; please fix errors and rebuild"
    exit $retVal
fi

  for j in TinyXML XMPPLib KeyerBase MqtBase MqtUtils MqtAppStarter MqtChat MqtCluster MqtDataModes MqtKeyer MqtKeyerProxy MqtKSTClient MqtLogger MqtMonitor MqtQrzServer MqtRigControl MqtRigSync MqtRigRecorder MqtRotator MqtServer
  do
    lrelease -verbose ../mqt/${j}/${j}.pro
    if [ ! -d ${j}/translations ]; then
        mkdir ${j}/translations
    fi
    mv ../mqt/${j}/translations/*.qm ${j}/translations
  done

cd ../..

if [ -d ./$INTERMEDIATE ]; then
  rm -rf ./$INTERMEDIATE
fi

mkdir ./$INTERMEDIATE

cd ./$INTERMEDIATE

mkdir Configuration
mkdir Lists
mkdir Logs
mkdir Bin
mkdir Bin/translations
mkdir Docs
mkdir Help

for i in en_GB fr_FR
do
  for j in MqtAppStarter MqtChat MqtCluster MqtDataModes MqtKeyer MqtKeyerProxy MqtKSTClient MqtLogger MqtMonitor MqtQrzServer MqtRigControl MqtRigSync MqtRigRecorder MqtRotator MqtServer
  do
    lconvert -verbose -o Bin/translations/${j}_${i}.qm \
    $DIR/$BUILDDIR/MqtUtils/translations/minos_${i}.qm \
    $DIR/$BUILDDIR/TinyXML/translations/minos_${i}.qm \
    $DIR/$BUILDDIR/XMPPLib/translations/minos_${i}.qm \
    $DIR/$BUILDDIR/MqtBase/translations/minos_${i}.qm \
    $DIR/$BUILDDIR/KeyerBase/translations/minos_${i}.qm \
    $DIR/$BUILDDIR/${j}/translations/minos_${i}.qm
  done
done
cp /usr/share/qt5/translations/qt*.qm Bin/translations

echo $OSTYPE

if [[ "$OSTYPE" == "darwin"* ]] ; then 		#MacOS

	cp -r $DIR/$BUILDDIR/MqtAppStarter/MqtAppStarter.app Bin
	cp -r $DIR/$BUILDDIR/MqtChat/MqtChat.app Bin
	cp -r $DIR/$BUILDDIR/MqtCluster/MqtCluster.app Bin
	cp -r $DIR/$BUILDDIR/MqtControl/MqtControl.app Bin
	cp -r $DIR/$BUILDDIR/MqtDataModes/MqtDataModes.app Bin
	cp -r $DIR/$BUILDDIR/MqtKeyer/MqtKeyer.app Bin
	cp -r $DIR/$BUILDDIR/MqtKeyerProxy/MqtKeyerProxy.app Bin
	cp -r $DIR/$BUILDDIR/MqtKSTClient/MqtKSTClient.app Bin
	cp -r $DIR/$BUILDDIR/MqtLogger/MqtLogger.app Bin
	cp -r $DIR/$BUILDDIR/MqtMonitor/MqtMonitor.app Bin
	cp -r $DIR/$BUILDDIR/MqtQrzServer/MqtQrzServer.app Bin
	cp -r $DIR/$BUILDDIR/MqtRigControl/MqtRigControl.app Bin
	cp -r $DIR/$BUILDDIR/MqtRigSync/MqtRigSync.app Bin
	cp -r $DIR/$BUILDDIR/MqtRigRecorder/MqtRigRecorder.app Bin
	cp -r $DIR/$BUILDDIR/MqtRotator/MqtRotator.app Bin
	cp -r $DIR/$BUILDDIR/MqtServer/MqtServer.app Bin

else
	cp $DIR/$BUILDDIR/MqtAppStarter/MqtAppStarter Bin
	cp $DIR/$BUILDDIR/MqtChat/MqtChat Bin
	cp $DIR/$BUILDDIR/MqtCluster/MqtCluster Bin
	cp $DIR/$BUILDDIR/MqtControl/MqtControl Bin
	cp $DIR/$BUILDDIR/MqtDataModes/MqtDataModes Bin
	cp $DIR/$BUILDDIR/MqtKeyer/MqtKeyer Bin
	cp $DIR/$BUILDDIR/MqtKeyerProxy/MqtKeyerProxy Bin
	cp $DIR/$BUILDDIR/MqtKSTClient/MqtKSTClient Bin
	cp $DIR/$BUILDDIR/MqtLogger/MqtLogger Bin
	cp $DIR/$BUILDDIR/MqtMonitor/MqtMonitor Bin
	cp $DIR/$BUILDDIR/MqtQrzServer/MqtQrzServer Bin
	cp $DIR/$BUILDDIR/MqtRigControl/MqtRigControl Bin
	cp $DIR/$BUILDDIR/MqtRigSync/MqtRigSync Bin
	cp $DIR/$BUILDDIR/MqtRigRecorder/MqtRigRecorder Bin
	cp $DIR/$BUILDDIR/MqtRotator/MqtRotator Bin
	cp $DIR/$BUILDDIR/MqtServer/MqtServer Bin
fi

cp -r $DIR/mqt/ControlFiles/Configuration/* ./Configuration
cp -r $DIR/mqt/ControlFiles/Configuration/LinuxFiles/* ./Configuration
rm -rf ./Configuration/WindowsFiles
rm -rf ./Configuration/LinuxFiles
cp $DIR/mqt/Installer/Minos.sh . 
cp $DIR/mqt/Installer/runAppStarter.sh .
cp $DIR/mqt/Docs/*.* ./Docs
cp $DIR/mqt/Help/*.* ./Help
cp $DIR/mqt/*.ico .

cd Configuration

wget https://www.country-files.com/cty/cty.dat -O cty.dat
wget https://www.rsgbcc.org/cgi-bin/vhfenter.pl?afsdownload=y -O clublist.txt
wget https://www.rsgbcc.org/vhf/vhfcontests23.xml -O vhfcontests23.xml
wget https://www.rsgbcc.org/vhf/vhfcontests24.xml -O vhfcontests24.xml

wget https://www.rsgbcc.org/vhf/hfcontests23.xml -O hfcontests23.xml
wget https://www.rsgbcc.org/vhf/hfcontests24.xml -O hfcontests24.xml

wget https://microwave.rsgbcc.org/microcontests23.xml -O microcontests23.xml
wget https://microwave.rsgbcc.org/microcontests24.xml -O microcontests24.xml

cd ..

read -n 1 -p "Do you want to copy the build to ~/$RUNTIME (press y/n)? " ans;

case $ans in
    y|Y)
        ;;
    n|N|*)
        exit;;
esac

if [ ! -d .~/$RUNTIME ]; then
   mkdir ~/$RUNTIME
fi
if [ ! -d ~/$RUNTIME/Logs ]; then
   mkdir ~/$RUNTIME/Logs
fi
if [ ! -d ~/$RUNTIME/Lists ]; then
   mkdir ~/$RUNTIME/Lists
fi
cp -rv Bin ~/$RUNTIME
cp *.ico ~/$RUNTIME
cp runAppStarter.sh ~/$RUNTIME
cp Minos.sh ~/$RUNTIME

read -n 1 -p "Do you want to copy the configuration to ~/$RUNTIME (press y/n)? " ans;


cp -rv Configuration ~/$RUNTIME

