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
mkdir Docs
mkdir Help

echo $OSTYPE

if [[ "$OSTYPE" == "darwin"* ]] ; then 		#MacOS

	cp -r ../Minos2/build/MqtAppStarter/MqtAppStarter.app Bin
	cp -r ../Minos2/build/MqtChat/MqtChat.app Bin
	cp -r ../Minos2/build/MqtCluster/MqtCluster.app Bin
	cp -r ../Minos2/build/MqtControl/MqtControl.app Bin
	cp -r ../Minos2/build/MqtKeyer/MqtKeyer.app Bin
	cp -r ../Minos2/build/MqtKSTClient/MqtKSTClient.app Bin
	cp -r ../Minos2/build/MqtLogger/MqtLogger.app Bin
	cp -r ../Minos2/build/MqtMonitor/MqtMonitor.app Bin
	cp -r ../Minos2/build/MqtRigControl/MqtRigControl.app Bin
	cp -r ../Minos2/build/MqtRotator/MqtRotator.app Bin
	cp -r ../Minos2/build/MqtServer/MqtServer.app Bin

else
	cp ../Minos2/build/MqtAppStarter/MqtAppStarter Bin
	cp ../Minos2/build/MqtChat/MqtChat Bin
	cp ../Minos2/build/MqtCluster/MqtCluster Bin
	cp ../Minos2/build/MqtControl/MqtControl Bin
	cp ../Minos2/build/MqtKeyer/MqtKeyer Bin
	cp ../Minos2/build/MqtKSTClient/MqtKSTClient Bin
	cp ../Minos2/build/MqtLogger/MqtLogger Bin
	cp ../Minos2/build/MqtMonitor/MqtMonitor Bin
	cp ../Minos2/build/MqtRigControl/MqtRigControl Bin
	cp ../Minos2/build/MqtRotator/MqtRotator Bin
	cp ../Minos2/build/MqtServer/MqtServer Bin
fi

cp -r ../Minos2/mqt/ControlFiles/Configuration/* ./Configuration
cp -r ../Minos2/mqt/ControlFiles/Configuration/LinuxFiles/* ./Configuration
rm -rf ./Configuration/WindowsFiles
rm -rf ./Configuration/LinuxFiles
cp ../Minos2/mqt/Installer/Minos.sh . 
cp ../Minos2/mqt/Docs/*.* ./Docs
cp ../Minos2/mqt/Help/*.* ./Help

cd Configuration

wget http://www.country-files.com/cty/cty.dat -O cty.dat
wget http://www.rsgbcc.org/vhf/vhfcontests17.xml -O vhfcontests17.xml
wget http://www.rsgbcc.org/vhf/vhfcontests18.xml -O vhfcontests18.xml
wget http://www.rsgbcc.org/vhf/vhfcontests19.xml -O vhfcontests19.xml

cd ..

read -n 1 -p "Do you want to copy the build to ~/runtime (press y/n)? " ans;

case $ans in
    y|Y)
        ;;
    n|N|*)
        exit;;
esac


cp -rv Bin ~/runtime

