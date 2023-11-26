#!/bin/bash

set -x

echo $OSTYPE
SCRIPT=$(basename $0)
DIR=$(echo `pwd`/../..)
QTDIR=~/Qt/6.6.0/macos

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

make -j4
retVal=$?

if [ $retVal -ne 0 ]; then
    echo "make failed; please fix errors and rebuild"
    exit $retVal 
fi

  for j in TinyXML XMPPLib KeyerBase MqtBase MqtUtils MqtAppStarter MqtChat MqtCluster MqtControl MqtKeyer MqtKSTClient MqtLogger MqtMonitor MqtQrzServer MqtRigControl MqtRigSync MqtRigRecorder MqtRotator MqtServer
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

cp -r ../Minos2/build/MqtLogger/MqtLogger.app .
cd MqtLogger.app/Contents/Resources

mkdir Configuration
mkdir Lists
mkdir Logs
mkdir Bin
mkdir Bin/translations
mkdir Docs

for i in en_GB fr_FR
do
  for j in MqtAppStarter MqtChat MqtCluster MqtControl MqtKeyer MqtKSTClient MqtLogger MqtMonitor MqtQrzServer MqtRigControl MqtRigRecorder MqtRigSync MqtRotator MqtServer
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

for j in MqtAppStarter MqtChat MqtCluster MqtControl MqtKeyer MqtKSTClient MqtMonitor MqtQrzServer MqtRigControl MqtRigRecorder MqtRigSync MqtRotator MqtServer
do
  cp -r $DIR/build/${j}/${j}.app Bin
done

cd ../../..

macdeployqt MqtLogger.app -qmldir=$DIR/mqt/MqtBase/QSOView

cd MqtLogger.app/Contents

sudo cp /usr/local/lib/libhamlib.4.dylib Frameworks

for j in MqtAppStarter MqtChat MqtCluster MqtControl MqtKeyer MqtKSTClient MqtMonitor MqtQrzServer MqtRigControl MqtRigRecorder MqtRigSync MqtRotator MqtServer
do
  ln -s ../../../../Frameworks   Resources/Bin/${j}.app/Contents/Frameworks
  ln -s ../../../../Plugins      Resources/Bin/${j}.app/Contents/Plugins
done

cd ../..

for j in MqtAppStarter MqtChat MqtCluster MqtControl MqtKeyer MqtKSTClient MqtMonitor MqtQrzServer MqtRigControl MqtRigRecorder MqtRigSync MqtRotator MqtServer
do
    install_name_tool -change /usr/local/lib/libhamlib.4.dylib @executable_path/../Frameworks/libhamlib.4.dylib MqtLogger.app/Contents/Resources/Bin/${j}.app/Contents/MacOS/${j}
    install_name_tool -change $QTDIR/lib/QtWidgets.framework/Versions/A/QtWidgets @executable_path/../Frameworks/QtWidgets.framework/Versions/A/QtWidgets MqtLogger.app/Contents/Resources/Bin/${j}.app/Contents/MacOS/${j}
    install_name_tool -change $QTDIR/lib/QtGui.framework/Versions/A/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/A/QtGui MqtLogger.app/Contents/Resources/Bin/${j}.app/Contents/MacOS/${j}
    install_name_tool -change $QTDIR/lib/QtSerialPort.framework/Versions/A/QtSerialPort @executable_path/../Frameworks/QtSerialPort.framework/Versions/A/QtSerialPort MqtLogger.app/Contents/Resources/Bin/${j}.app/Contents/MacOS/${j}
    install_name_tool -change $QTDIR/lib/QtNetwork.framework/Versions/A/QtNetwork @executable_path/../Frameworks/QtNetwork.framework/Versions/A/QtNetwork MqtLogger.app/Contents/Resources/Bin/${j}.app/Contents/MacOS/${j}
    install_name_tool -change $QTDIR/lib/QtCore.framework/Versions/A/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/A/QtCore MqtLogger.app/Contents/Resources/Bin/${j}.app/Contents/MacOS/${j}
done
cd MqtLogger.app/Contents/Resources


cp -r $DIR/mqt/ControlFiles/Configuration/* ./Configuration 
cp -r $DIR/mqt/ControlFiles/Configuration/LinuxFiles/* ./Configuration 

cp $DIR/mqt/Docs/*.* ./Docs 

rm -rf ./Configuration/OSXFiles
rm -rf ./Configuration/LinuxFiles
rm -rf ./Configuration/WindowsFiles

cd Configuration

wget https://www.country-files.com/cty/cty.dat -O cty.dat
wget https://www.rsgbcc.org/cgi-bin/vhfenter.pl?afsdownload=y -O clublist.txt

wget https://www.rsgbcc.org/vhf/vhfcontests20.xml -O vhfcontests23.xml
wget https://www.rsgbcc.org/vhf/vhfcontests21.xml -O vhfcontests24.xml

wget https://www.rsgbcc.org/vhf/hfcontests20.xml -O hfcontests23.xml
wget https://www.rsgbcc.org/vhf/hfcontests21.xml -O hfcontests24.xml

wget https://microwave.rsgbcc.org/microcontests20.xml -O microcontests23.xml
wget https://microwave.rsgbcc.org/microcontests21.xml -O microcontests24.xml

cd ..


codesign --preserve-metadata=entitlements --force --verbose --sign "3rd Party Mac Developer Application: Phil Taylor (6255R636WZ)" --deep /Users/phil/source/minos-runtime/MqtLogger.app --entitlements /Users/phil/source/minos/mqt/ControlFiles/minos.entitlements
