#!/bin/bash

#cd ~/Minos2
SCRIPT=$(readlink -f $0)
DIR=$(echo ${SCRIPT%/*}/../..)
cd $DIR

git pull origin master

if [ ! -d ./build ]; then
   mkdir build
fi

cd build

qmake ../mqt/mqt.pro

retVal=$?
if [ $retVal -ne 0 ]; then
    echo "qmake failed; please fix errors and rebuild"
    exit $retVal
fi

make
retVal=$?
if [ $retVal -ne 0 ]; then
    echo "make failed; please fix errors and rebuild"
    exit $retVal
fi

  for j in TinyXML XMPPLib MqtBase MqtUtils MqtAppStarter MqtChat MqtCluster MqtKSTClient MqtLogger MqtMonitor MqtRigControl MqtRotator MqtServer
  do
    lrelease -verbose ../mqt/${j}/${j}.pro
    if [ ! -d ${j}/translations ]; then
        mkdir ${j}/translations
    fi
    mv ../mqt/${j}/translations/*.qm ${j}/translations
  done

cd ../..

if [ -d ./minos-runtime ]; then
  rm -rf ./minos-runtime
fi

mkdir ./minos-runtime

cd ./minos-runtime

mkdir Configuration
mkdir Lists
mkdir Logs
mkdir Bin
mkdir Bin/translations
mkdir Docs
mkdir Help

for i in en_GB fr_FR
do
  for j in MqtAppStarter MqtChat MqtCluster MqtKSTClient MqtLogger MqtMonitor MqtRigControl MqtRotator MqtServer
  do
    lconvert -verbose -o Bin/translations/${j}_${i}.qm \
    $DIR/build/MqtUtils/translations/minos_${i}.qm \
    $DIR/build/TinyXML/translations/minos_${i}.qm \
    $DIR/build/XMPPLib/translations/minos_${i}.qm \
    $DIR/build/MqtBase/translations/minos_${i}.qm \
    $DIR/build/${j}/translations/minos_${i}.qm
  done
done
cp /usr/share/qt5/translations/qt*.qm Bin/translations

echo $OSTYPE

if [[ "$OSTYPE" == "darwin"* ]] ; then 		#MacOS

	cp -r $DIR/build/MqtAppStarter/MqtAppStarter.app Bin
	cp -r $DIR/build/MqtChat/MqtChat.app Bin
	cp -r $DIR/build/MqtCluster/MqtCluster.app Bin
	cp -r $DIR/build/MqtControl/MqtControl.app Bin
	cp -r $DIR/build/MqtKeyer/MqtKeyer.app Bin
	cp -r $DIR/build/MqtKSTClient/MqtKSTClient.app Bin
	cp -r $DIR/build/MqtLogger/MqtLogger.app Bin
	cp -r $DIR/build/MqtMonitor/MqtMonitor.app Bin
	cp -r $DIR/build/MqtRigControl/MqtRigControl.app Bin
	cp -r $DIR/build/MqtRotator/MqtRotator.app Bin
	cp -r $DIR/build/MqtServer/MqtServer.app Bin

else
	cp $DIR/build/MqtAppStarter/MqtAppStarter Bin
	cp $DIR/build/MqtChat/MqtChat Bin
	cp $DIR/build/MqtCluster/MqtCluster Bin
	cp $DIR/build/MqtControl/MqtControl Bin
	cp $DIR/build/MqtKeyer/MqtKeyer Bin
	cp $DIR/build/MqtKSTClient/MqtKSTClient Bin
	cp $DIR/build/MqtLogger/MqtLogger Bin
	cp $DIR/build/MqtMonitor/MqtMonitor Bin
	cp $DIR/build/MqtRigControl/MqtRigControl Bin
	cp $DIR/build/MqtRotator/MqtRotator Bin
	cp $DIR/build/MqtServer/MqtServer Bin
fi

cp -r $DIR/mqt/ControlFiles/Configuration/* ./Configuration
cp -r $DIR/mqt/ControlFiles/Configuration/LinuxFiles/* ./Configuration
rm -rf ./Configuration/WindowsFiles
rm -rf ./Configuration/LinuxFiles
cp $DIR/mqt/Installer/Minos.sh . 
cp $DIR/mqt/Docs/*.* ./Docs
cp $DIR/mqt/Help/*.* ./Help

cd Configuration

wget https://www.country-files.com/cty/cty.dat -O cty.dat
wget https://www.rsgbcc.org/vhf/vhfcontests19.xml -O vhfcontests19.xml
wget https://www.rsgbcc.org/vhf/vhfcontests20.xml -O vhfcontests20.xml

cd ..

read -n 1 -p "Do you want to copy the build to ~/runtime (press y/n)? " ans;

case $ans in
    y|Y)
        ;;
    n|N|*)
        exit;;
esac


cp -rv Bin ~/runtime

read -n 1 -p "Do you want to copy the configuration to ~/runtime (press y/n)? " ans;


cp -rv Configuration ~/runtime

