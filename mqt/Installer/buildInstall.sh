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

make

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

echo $OSTYPE

if [[ "$OSTYPE" == "darwin"* ]] ; then 		#MacOS

	cp -r ../Minos2/build/MqtAppStarter/MqtAppStarter.app Bin
	cp -r ../Minos2/build/MqtChat/MqtChat.app Bin
	cp -r ../Minos2/build/MqtControl/MqtControl.app Bin
	cp -r ../Minos2/build/MqtKeyer/MqtKeyer.app Bin
	cp -r ../Minos2/build/MqtLogger/MqtLogger.app Bin
	cp -r ../Minos2/build/MqtMonitor/MqtMonitor.app Bin
	cp -r ../Minos2/build/MqtRigControl/MqtRigControl.app Bin
	cp -r ../Minos2/build/MqtRotator/MqtRotator.app Bin
	cp -r ../Minos2/build/MqtServer/MqtServer.app Bin

else
	cp ../Minos2/build/MqtAppStarter/MqtAppStarter Bin
	cp ../Minos2/build/MqtChat/MqtChat Bin
	cp ../Minos2/build/MqtControl/MqtControl Bin
	cp ../Minos2/build/MqtKeyer/MqtKeyer Bin
	cp ../Minos2/build/MqtLogger/MqtLogger Bin
	cp ../Minos2/build/MqtMonitor/MqtMonitor Bin
	cp ../Minos2/build/MqtRigControl/MqtRigControl Bin
	cp ../Minos2/build/MqtRotator/MqtRotator Bin
	cp ../Minos2/build/MqtServer/MqtServer Bin
fi

cp -r ../Minos2/mqt/ControlFiles/Configuration/* ./Configuration
cp -r ../Minos2/mqt/ControlFiles/Configuration/LinuxFiles/* ./Configuration
cp -r ../Minos2/mqt/Installer/Minos.sh . 

cd Configuration

wget http://www.country-files.com/cty/cty.dat -O cty.dat
wget http://www.rsgbcc.org/vhf/vhfcontests16.xml -O vhfcontests16.xml
wget http://www.rsgbcc.org/vhf/vhfcontests17.xml -O vhfcontests17.xml
wget http://www.rsgbcc.org/vhf/vhfcontests18.xml -O vhfcontests18.xml
