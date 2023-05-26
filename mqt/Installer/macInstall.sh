#!/bin/bash

#cd ~/Minos2

echo $OSTYPE
SCRIPT=$(basename $0)
DIR=$(echo `pwd`/../..)
QTDIR=~/Qt/5.15.2/clang_64

cd $DIR
echo Working Dir: $DIR

git pull origin

if [ ! -d ./build ]; then
   mkdir build 
fi

cd build

qmake ../mqt/mqt.pro -config release
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

  for j in TinyXML XMPPLib KeyerBase MqtBase MqtUtils MqtAppStarter MqtChat MqtCluster MqtKSTClient MqtLogger MqtMonitor MqtQrzServer MqtRigControl MqtRigSync MqtRigRecorder MqtRotator MqtServer
  do
    lrelease -verbose ../mqt/${j}/${j}.pro
    if [ ! -d ${j}/translations ]; then
        mkdir ${j}/translations
    fi
    mv ../mqt/${j}/translations/*.qm ${j}/translations
  done

cd ../..

if [ -d ./minos-runtime ]; then
  sudo rm -rf ./minos-runtime 
fi

mkdir ./minos-runtime

cd ./minos-runtime

cp -r ../minos/build/MqtLogger/MqtLogger.app .
cd MqtLogger.app/Contents/Resources

mkdir Configuration
mkdir Lists
mkdir Logs
mkdir Bin
mkdir Bin/translations
mkdir Docs
mkdir Help

for i in en_GB fr_FR
do
  for j in MqtAppStarter MqtChat MqtCluster MqtKSTClient MqtLogger MqtMonitor MqtQrzServer MqtRigControl MqtRigSync MqtRigRecorder MqtRotator MqtServer
  do
    lconvert -verbose -o Bin/translations/${j}_${i}.qm \
    $DIR/build/${j}/translations/minos_${i}.qm \
    $DIR/build/TinyXML/translations/minos_${i}.qm \
    $DIR/build/XMPPLib/translations/minos_${i}.qm \
    $DIR/build/MqtBase/translations/minos_${i}.qm \
    $DIR/build/KeyerBase/translations/minos_${i}.qm \
    $DIR/build/${j}/translations/minos_${i}.qm
  done
done

cp $QTDIR/translations/qt*.qm Bin/translations

echo $OSTYPE

cp -r $DIR/build/MqtAppStarter/MqtAppStarter.app Bin
cp -r $DIR/build/MqtRigControl/MqtRigControl.app Bin
cp -r $DIR/build/MqtChat/MqtChat.app Bin
cp -r $DIR/build/MqtCluster/MqtCluster.app Bin
cp -r $DIR/build/MqtControl/MqtControl.app Bin
cp -r $DIR/build/MqtKeyer/MqtKeyer.app Bin
cp -r $DIR/build/MqtMonitor/MqtMonitor.app Bin
cp -r $DIR/build/MqtQrzServer/MqtQrzServer.app Bin
cp -r $DIR/build/MqtRigSync/MqtRigSync.app Bin
cp -r $DIR/build/MqtRigRecorder/MqtRigRecorder.app Bin
cp -r $DIR/build/MqtRotator/MqtRotator.app Bin
cp -r $DIR/build/MqtServer/MqtServer.app Bin
cp -r $DIR/build/MqtKstClient/MqtKstClient.app Bin

cd ../../..
macdeployqt MqtLogger.app/
cd MqtLogger.app/Contents

mkdir Frameworks	
sudo cp /usr/local/lib/libhamlib.4.dylib Frameworks
sudo cp /usr/local/lib/libusb-1.0.0.dylib Frameworks
sudo cp -R $QTDIR/lib/QtMultimedia.framework Frameworks
sudo cp -R $QTDIR/lib/QtSerialPort.framework Frameworks
sudo rm -rf Frameworks/QtMultimedia.framework/Headers
sudo rm -rf Frameworks/QtSerialPort.framework/Headers
sudo rm -rf Frameworks/QtSerialPort.framework/Versions/Current/Headers
sudo chown -R phil Frameworks/QtMultimedia.framework
sudo chown -R phil Frameworks/QtSerialPort.framework

ln -s ../../../../Frameworks Resources/Bin/MqtAppStarter.app/Contents/Frameworks
ln -s ../../../../Plugins Resources/Bin/MqtAppStarter.app/Contents/Plugins
ln -s ../../../../Frameworks Resources/Bin/MqtRigControl.app/Contents/Frameworks
ln -s ../../../../Plugins Resources/Bin/MqtRigControl.app/Contents/Plugins
ln -s ../../../../Frameworks Resources/Bin/MqtChat.app/Contents/Frameworks
ln -s ../../../../Plugins Resources/Bin/MqtChat.app/Contents/Plugins
ln -s ../../../../Frameworks Resources/Bin/MqtCluster.app/Contents/Frameworks
ln -s ../../../../Plugins Resources/Bin/MqtCluster.app/Contents/Plugins
ln -s ../../../../Frameworks Resources/Bin/MqtControl.app/Contents/Frameworks
ln -s ../../../../Plugins Resources/Bin/MqtControl.app/Contents/Plugins
ln -s ../../../../Frameworks Resources/Bin/MqtKeyer.app/Contents/Frameworks
ln -s ../../../../Plugins Resources/Bin/MqtKeyer.app/Contents/Plugins
ln -s ../../../../Frameworks Resources/Bin/MqtMonitor.app/Contents/Frameworks
ln -s ../../../../Plugins Resources/Bin/MqtMonitor.app/Contents/Plugins
ln -s ../../../../Frameworks Resources/Bin/MqtQrzServer.app/Contents/Frameworks
ln -s ../../../../Plugins Resources/Bin/MqtQrzServer.app/Contents/Plugins
ln -s ../../../../Frameworks Resources/Bin/MqtRigSync.app/Contents/Frameworks
ln -s ../../../../Plugins Resources/Bin/MqtRigSync.app/Contents/Plugins
ln -s ../../../../Frameworks Resources/Bin/MqtRigRecorder.app/Contents/Frameworks
ln -s ../../../../Plugins Resources/Bin/MqtRigRecorder.app/Contents/Plugins
ln -s ../../../../Frameworks Resources/Bin/MqtRotator.app/Contents/Frameworks
ln -s ../../../../Plugins Resources/Bin/MqtRotator.app/Contents/Plugins
ln -s ../../../../Frameworks Resources/Bin/MqtServer.app/Contents/Frameworks
ln -s ../../../../Plugins Resources/Bin/MqtServer.app/Contents/Plugins
ln -s ../../../../Frameworks Resources/Bin/MqtKstClient.app/Contents/Frameworks
ln -s ../../../../Plugins Resources/Bin/MqtKstClient.app/Contents/Plugins

cd ../..

	sudo install_name_tool -change /usr/local/lib/libusb-1.0.0.dylib @executable_path/../Frameworks/libusb-1.0.0.dylib MqtLogger.app/Contents/Frameworks/libhamlib.4.dylib
	sudo install_name_tool -change $QTDIR/lib/QtCore.framework/Versions/5/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore MqtLogger.app/Contents/Frameworks/QtMultimedia.framework/Versions/5/QtMultimedia
	sudo install_name_tool -change $QTDIR/lib/QtMultimedia.framework/Versions/5/QtMultimedia @executable_path/../Frameworks/QtMultimedia.framework/Versions/5/QtMultimedia MqtLogger.app/Contents/Frameworks/QtMultimedia.framework/Versions/5/QtMultimedia
	sudo install_name_tool -change $QTDIR/lib/QtSerialPort.framework/Versions/5/QtSerialPort @executable_path/../Frameworks/QtSerialPort.framework/Versions/5/QtSerialPort MqtLogger.app/Contents/Frameworks/QtSerialPort.framework/Versions/5/QtSerialPort
	sudo install_name_tool -change $QTDIR/lib/QtNetwork.framework/Versions/5/QtNetwork @executable_path/../Frameworks/QtNetwork.framework/Versions/5/QtNetwork MqtLogger.app/Contents/Frameworks/QtMultimedia.framework/Versions/5/QtMultimedia
	sudo install_name_tool -change $QTDIR/lib/QtGui.framework/Versions/5/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui MqtLogger.app/Contents/Frameworks/QtMultimedia.framework/Versions/5/QtMultimedia
	sudo install_name_tool -change $QTDIR/lib/QtCore.framework/Versions/5/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore MqtLogger.app/Contents/Frameworks/QtSerialPort.framework/Versions/5/QtSerialPort

        install_name_tool -change /usr/local/lib/libhamlib.4.dylib @executable_path/../Frameworks/libhamlib.4.dylib MqtLogger.app/Contents/Resources/Bin/MqtRigControl.app/Contents/MacOS/MqtRigControl
        install_name_tool -change /usr/local/lib/libusb-1.0.0.dylib @executable_path/../Frameworks/libusb-1.0.0.dylib MqtLogger.app/Contents/Resources/Bin/MqtRigControl.app/Contents/MacOS/MqtRigControl
        install_name_tool -change $QTDIR/lib/QtWidgets.framework/Versions/5/QtWidgets @executable_path/../Frameworks/QtWidgets.framework/Versions/5/QtWidgets MqtLogger.app/Contents/Resources/Bin/MqtRigControl.app/Contents/MacOS/MqtRigControl
        install_name_tool -change $QTDIR/lib/QtGui.framework/Versions/5/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui MqtLogger.app/Contents/Resources/Bin/MqtRigControl.app/Contents/MacOS/MqtRigControl
        install_name_tool -change $QTDIR/lib/QtSerialPort.framework/Versions/5/QtSerialPort @executable_path/../Frameworks/QtSerialPort.framework/Versions/5/QtSerialPort MqtLogger.app/Contents/Resources/Bin/MqtRigControl.app/Contents/MacOS/MqtRigControl
        install_name_tool -change $QTDIR/lib/QtNetwork.framework/Versions/5/QtNetwork @executable_path/../Frameworks/QtNetwork.framework/Versions/5/QtNetwork MqtLogger.app/Contents/Resources/Bin/MqtRigControl.app/Contents/MacOS/MqtRigControl
        install_name_tool -change $QTDIR/lib/QtCore.framework/Versions/5/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore MqtLogger.app/Contents/Resources/Bin/MqtRigControl.app/Contents/MacOS/MqtRigControl

        install_name_tool -change /usr/local/lib/libhamlib.4.dylib @executable_path/../Frameworks/libhamlib.4.dylib MqtLogger.app/Contents/Resources/Bin/MqtRotator.app/Contents/MacOS/MqtRotator
        install_name_tool -change /usr/local/lib/libusb-1.0.0.dylib @executable_path/../Frameworks/libusb-1.0.0.dylib MqtLogger.app/Contents/Resources/Bin/MqtRotator.app/Contents/MacOS/MqtRotator
        install_name_tool -change $QTDIR/lib/QtWidgets.framework/Versions/5/QtWidgets @executable_path/../Frameworks/QtWidgets.framework/Versions/5/QtWidgets MqtLogger.app/Contents/Resources/Bin/MqtRotator.app/Contents/MacOS/MqtRotator
        install_name_tool -change $QTDIR/lib/QtGui.framework/Versions/5/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui MqtLogger.app/Contents/Resources/Bin/MqtRotator.app/Contents/MacOS/MqtRotator
        install_name_tool -change $QTDIR/lib/QtSerialPort.framework/Versions/5/QtSerialPort @executable_path/../Frameworks/QtSerialPort.framework/Versions/5/QtSerialPort MqtLogger.app/Contents/Resources/Bin/MqtRotator.app/Contents/MacOS/MqtRotator
        install_name_tool -change $QTDIR/lib/QtNetwork.framework/Versions/5/QtNetwork @executable_path/../Frameworks/QtNetwork.framework/Versions/5/QtNetwork MqtLogger.app/Contents/Resources/Bin/MqtRotator.app/Contents/MacOS/MqtRotator
        install_name_tool -change $QTDIR/lib/QtCore.framework/Versions/5/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore MqtLogger.app/Contents/Resources/Bin/MqtRotator.app/Contents/MacOS/MqtRotator

        install_name_tool -change $QTDIR/lib/QtWidgets.framework/Versions/5/QtWidgets @executable_path/../Frameworks/QtWidgets.framework/Versions/5/QtWidgets MqtLogger.app/Contents/Resources/Bin/MqtAppStarter.app/Contents/MacOS/MqtAppStarter
        install_name_tool -change $QTDIR/lib/QtGui.framework/Versions/5/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui MqtLogger.app/Contents/Resources/Bin/MqtAppStarter.app/Contents/MacOS/MqtAppStarter
        install_name_tool -change $QTDIR/lib/QtNetwork.framework/Versions/5/QtNetwork @executable_path/../Frameworks/QtNetwork.framework/Versions/5/QtNetwork MqtLogger.app/Contents/Resources/Bin/MqtAppStarter.app/Contents/MacOS/MqtAppStarter
        install_name_tool -change $QTDIR/lib/QtCore.framework/Versions/5/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore MqtLogger.app/Contents/Resources/Bin/MqtAppStarter.app/Contents/MacOS/MqtAppStarter

        install_name_tool -change $QTDIR/lib/QtWidgets.framework/Versions/5/QtWidgets @executable_path/../Frameworks/QtWidgets.framework/Versions/5/QtWidgets MqtLogger.app/Contents/Resources/Bin/MqtChat.app/Contents/MacOS/MqtChat
        install_name_tool -change $QTDIR/lib/QtGui.framework/Versions/5/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui MqtLogger.app/Contents/Resources/Bin/MqtChat.app/Contents/MacOS/MqtChat
        install_name_tool -change $QTDIR/lib/QtNetwork.framework/Versions/5/QtNetwork @executable_path/../Frameworks/QtNetwork.framework/Versions/5/QtNetwork MqtLogger.app/Contents/Resources/Bin/MqtChat.app/Contents/MacOS/MqtChat
        install_name_tool -change $QTDIR/lib/QtCore.framework/Versions/5/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore MqtLogger.app/Contents/Resources/Bin/MqtChat.app/Contents/MacOS/MqtChat

        install_name_tool -change $QTDIR/lib/QtWidgets.framework/Versions/5/QtWidgets @executable_path/../Frameworks/QtWidgets.framework/Versions/5/QtWidgets MqtLogger.app/Contents/Resources/Bin/MqtCluster.app/Contents/MacOS/MqtCluster
        install_name_tool -change $QTDIR/lib/QtGui.framework/Versions/5/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui MqtLogger.app/Contents/Resources/Bin/MqtCluster.app/Contents/MacOS/MqtCluster
        install_name_tool -change $QTDIR/lib/QtNetwork.framework/Versions/5/QtNetwork @executable_path/../Frameworks/QtNetwork.framework/Versions/5/QtNetwork MqtLogger.app/Contents/Resources/Bin/MqtCluster.app/Contents/MacOS/MqtCluster
        install_name_tool -change $QTDIR/lib/QtSql.framework/Versions/5/QtSql @executable_path/../Frameworks/QtSql.framework/Versions/5/QtSql MqtLogger.app/Contents/Resources/Bin/MqtCluster.app/Contents/MacOS/MqtCluster
        install_name_tool -change $QTDIR/lib/QtCore.framework/Versions/5/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore MqtLogger.app/Contents/Resources/Bin/MqtCluster.app/Contents/MacOS/MqtCluster

        install_name_tool -change $QTDIR/lib/QtWidgets.framework/Versions/5/QtWidgets @executable_path/../Frameworks/QtWidgets.framework/Versions/5/QtWidgets MqtLogger.app/Contents/Resources/Bin/MqtControl.app/Contents/MacOS/MqtControl
        install_name_tool -change $QTDIR/lib/QtGui.framework/Versions/5/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui MqtLogger.app/Contents/Resources/Bin/MqtControl.app/Contents/MacOS/MqtControl
        install_name_tool -change $QTDIR/lib/QtNetwork.framework/Versions/5/QtNetwork @executable_path/../Frameworks/QtNetwork.framework/Versions/5/QtNetwork MqtLogger.app/Contents/Resources/Bin/MqtControl.app/Contents/MacOS/MqtControl
        install_name_tool -change $QTDIR/lib/QtCore.framework/Versions/5/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore MqtLogger.app/Contents/Resources/Bin/MqtControl.app/Contents/MacOS/MqtControl

        install_name_tool -change $QTDIR/lib/QtWidgets.framework/Versions/5/QtWidgets @executable_path/../Frameworks/QtWidgets.framework/Versions/5/QtWidgets MqtLogger.app/Contents/Resources/Bin/MqtKeyer.app/Contents/MacOS/MqtKeyer
        install_name_tool -change $QTDIR/lib/QtMultimedia.framework/Versions/5/QtMultimedia @executable_path/../Frameworks/QtMultimedia.framework/Versions/5/QtMultimedia MqtLogger.app/Contents/Resources/Bin/MqtKeyer.app/Contents/MacOS/MqtKeyer
        install_name_tool -change $QTDIR/lib/QtGui.framework/Versions/5/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui MqtLogger.app/Contents/Resources/Bin/MqtKeyer.app/Contents/MacOS/MqtKeyer
        install_name_tool -change $QTDIR/lib/QtNetwork.framework/Versions/5/QtNetwork @executable_path/../Frameworks/QtNetwork.framework/Versions/5/QtNetwork MqtLogger.app/Contents/Resources/Bin/MqtKeyer.app/Contents/MacOS/MqtKeyer
        install_name_tool -change $QTDIR/lib/QtCore.framework/Versions/5/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore MqtLogger.app/Contents/Resources/Bin/MqtKeyer.app/Contents/MacOS/MqtKeyer

        install_name_tool -change $QTDIR/lib/QtWidgets.framework/Versions/5/QtWidgets @executable_path/../Frameworks/QtWidgets.framework/Versions/5/QtWidgets MqtLogger.app/Contents/Resources/Bin/MqtMonitor.app/Contents/MacOS/MqtMonitor
        install_name_tool -change $QTDIR/lib/QtGui.framework/Versions/5/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui MqtLogger.app/Contents/Resources/Bin/MqtMonitor.app/Contents/MacOS/MqtMonitor
        install_name_tool -change $QTDIR/lib/QtNetwork.framework/Versions/5/QtNetwork @executable_path/../Frameworks/QtNetwork.framework/Versions/5/QtNetwork MqtLogger.app/Contents/Resources/Bin/MqtMonitor.app/Contents/MacOS/MqtMonitor
        install_name_tool -change $QTDIR/lib/QtCore.framework/Versions/5/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore MqtLogger.app/Contents/Resources/Bin/MqtMonitor.app/Contents/MacOS/MqtMonitor

        install_name_tool -change $QTDIR/lib/QtWidgets.framework/Versions/5/QtWidgets @executable_path/../Frameworks/QtWidgets.framework/Versions/5/QtWidgets MqtLogger.app/Contents/Resources/Bin/MqtQrzServer.app/Contents/MacOS/MqtQrzServer
        install_name_tool -change $QTDIR/lib/QtGui.framework/Versions/5/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui MqtLogger.app/Contents/Resources/Bin/MqtQrzServer.app/Contents/MacOS/MqtQrzServer
        install_name_tool -change $QTDIR/lib/QtNetwork.framework/Versions/5/QtNetwork @executable_path/../Frameworks/QtNetwork.framework/Versions/5/QtNetwork MqtLogger.app/Contents/Resources/Bin/MqtQrzServer.app/Contents/MacOS/MqtQrzServer
        install_name_tool -change $QTDIR/lib/QtCore.framework/Versions/5/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore MqtLogger.app/Contents/Resources/Bin/MqtQrzServer.app/Contents/MacOS/MqtQrzServer

        install_name_tool -change $QTDIR/lib/QtWidgets.framework/Versions/5/QtWidgets @executable_path/../Frameworks/QtWidgets.framework/Versions/5/QtWidgets MqtLogger.app/Contents/Resources/Bin/MqtRigSync.app/Contents/MacOS/MqtRigSync
        install_name_tool -change $QTDIR/lib/QtGui.framework/Versions/5/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui MqtLogger.app/Contents/Resources/Bin/MqtRigSync.app/Contents/MacOS/MqtRigSync
        install_name_tool -change $QTDIR/lib/QtNetwork.framework/Versions/5/QtNetwork @executable_path/../Frameworks/QtNetwork.framework/Versions/5/QtNetwork MqtLogger.app/Contents/Resources/Bin/MqtRigSync.app/Contents/MacOS/MqtRigSync
        install_name_tool -change $QTDIR/lib/QtCore.framework/Versions/5/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore MqtLogger.app/Contents/Resources/Bin/MqtRigSync.app/Contents/MacOS/MqtRigSync

        install_name_tool -change $QTDIR/lib/QtWidgets.framework/Versions/5/QtWidgets @executable_path/../Frameworks/QtWidgets.framework/Versions/5/QtWidgets MqtLogger.app/Contents/Resources/Bin/MqtRigRecorder.app/Contents/MacOS/MqtRigRecorder
        install_name_tool -change $QTDIR/lib/QtGui.framework/Versions/5/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui MqtLogger.app/Contents/Resources/Bin/MqtRigRecorder.app/Contents/MacOS/MqtRigRecorder
        install_name_tool -change $QTDIR/lib/QtNetwork.framework/Versions/5/QtNetwork @executable_path/../Frameworks/QtNetwork.framework/Versions/5/QtNetwork MqtLogger.app/Contents/Resources/Bin/MqtRigRecorder.app/Contents/MacOS/MqtRigRecorder
        install_name_tool -change $QTDIR/lib/QtCore.framework/Versions/5/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore MqtLogger.app/Contents/Resources/Bin/MqtRigRecorder.app/Contents/MacOS/MqtRigRecorder

        install_name_tool -change $QTDIR/lib/QtWidgets.framework/Versions/5/QtWidgets @executable_path/../Frameworks/QtWidgets.framework/Versions/5/QtWidgets MqtLogger.app/Contents/Resources/Bin/MqtServer.app/Contents/MacOS/MqtServer
        install_name_tool -change $QTDIR/lib/QtGui.framework/Versions/5/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui MqtLogger.app/Contents/Resources/Bin/MqtServer.app/Contents/MacOS/MqtServer
        install_name_tool -change $QTDIR/lib/QtNetwork.framework/Versions/5/QtNetwork @executable_path/../Frameworks/QtNetwork.framework/Versions/5/QtNetwork MqtLogger.app/Contents/Resources/Bin/MqtServer.app/Contents/MacOS/MqtServer
        install_name_tool -change $QTDIR/lib/QtCore.framework/Versions/5/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore MqtLogger.app/Contents/Resources/Bin/MqtServer.app/Contents/MacOS/MqtServer

        install_name_tool -change $QTDIR/lib/QtWidgets.framework/Versions/5/QtWidgets @executable_path/../Frameworks/QtWidgets.framework/Versions/5/QtWidgets MqtLogger.app/Contents/Resources/Bin/MqtKstClient.app/Contents/MacOS/MqtKstClient
        install_name_tool -change $QTDIR/lib/QtGui.framework/Versions/5/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui MqtLogger.app/Contents/Resources/Bin/MqtKstClient.app/Contents/MacOS/MqtKstClient
        install_name_tool -change $QTDIR/lib/QtNetwork.framework/Versions/5/QtNetwork @executable_path/../Frameworks/QtNetwork.framework/Versions/5/QtNetwork MqtLogger.app/Contents/Resources/Bin/MqtKstClient.app/Contents/MacOS/MqtKstClient
        install_name_tool -change $QTDIR/lib/QtSql.framework/Versions/5/QtSql @executable_path/../Frameworks/QtSql.framework/Versions/5/QtSql MqtLogger.app/Contents/Resources/Bin/MqtKstClient.app/Contents/MacOS/MqtKstClient
        install_name_tool -change $QTDIR/lib/QtCore.framework/Versions/5/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore MqtLogger.app/Contents/Resources/Bin/MqtKstClient.app/Contents/MacOS/MqtKstClient

	cd MqtLogger.app/Contents/Resources


cp -r $DIR/mqt/ControlFiles/Configuration/* ./Configuration 
cp -r $DIR/mqt/ControlFiles/Configuration/LinuxFiles/* ./Configuration 

cp $DIR/mqt/Docs/*.* ./Docs 
cp $DIR/mqt/Help/*.* ./Help

rm -rf ./Configuration/OSXFiles
rm -rf ./Configuration/LinuxFiles
rm -rf ./Configuration/WindowsFiles

cd Configuration

wget https://www.country-files.com/cty/cty.dat -O cty.dat

wget https://www.rsgbcc.org/vhf/vhfcontests20.xml -O vhfcontests20.xml
wget https://www.rsgbcc.org/vhf/vhfcontests21.xml -O vhfcontests21.xml

wget https://www.rsgbcc.org/vhf/hfcontests20.xml -O hfcontests20.xml
wget https://www.rsgbcc.org/vhf/hfcontests21.xml -O hfcontests21.xml

wget https://microwave.rsgbcc.org/microcontests20.xml -O microcontests20.xml
wget https://microwave.rsgbcc.org/microcontests21.xml -O microcontests21.xml

cd ..


codesign --preserve-metadata=entitlements --force --verbose --sign "3rd Party Mac Developer Application: Phil Taylor (6255R636WZ)" --deep /Users/phil/source/minos-runtime/MqtLogger.app --entitlements /Users/phil/source/minos/mqt/ControlFiles/minos.entitlements
