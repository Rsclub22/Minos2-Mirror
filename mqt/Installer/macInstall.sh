#!/bin/bash

set -x

echo $OSTYPE

if [[ "$OSTYPE" != "darwin"* ]] ; then 		#MacOS
echo Please use buildInstall.sh instead
exit 1
fi

SCRIPT=$(basename $0)
DIR=$(echo `pwd`/../..)
QTDIR=~/Qt/6.10.2/macos
BUILDDIR="build/install"

cd $DIR
echo Working Dir: $DIR

git pull origin

if [ ! -d ./$BUILDDIR ]; then
   mkdir -p $BUILDDIR 
fi

cd $BUILDDIR

qmake ../../mqt/mqt.pro -config release
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

echo Current Dir: `pwd`

  for j in TinyXML XMPPLib KeyerBase MqtBase MqtUtils MqtAppStarter MqtChat MqtCluster MqtControl MqtKeyer MqtKSTClient MqtLogger MqtMonitor MqtPcCwKeyer MqtQrzServer MqtRigControl MqtRigSync MqtRigRecorder MqtRotator MqtServer MqtWinkeyer
  do
    lrelease -verbose ../../mqt/${j}/${j}.pro
    if [ ! -d ${j}/translations ]; then
        mkdir ${j}/translations
    fi
    mv ../../mqt/${j}/translations/*.qm ${j}/translations
  done

cd ../../..

if [ -d ./minos-runtime ]; then
  sudo rm -rf ./minos-runtime 
fi

mkdir ./minos-runtime

cd ./minos-runtime

cp -r ../Minos2/$BUILDDIR/MqtLogger/MqtLogger.app .
cd MqtLogger.app/Contents/Resources

mkdir Configuration
mkdir Lists
mkdir Logs
mkdir Bin
mkdir Bin/translations
mkdir Docs

for i in en_GB fr_FR
do
  for j in MqtAppStarter MqtChat MqtCluster MqtControl MqtKeyer MqtKSTClient MqtLogger MqtMonitor MqtPcCwKeyer MqtQrzServer MqtRigControl MqtRigRecorder MqtRigSync MqtRotator MqtServer MqtWinkeyer
  do
    lconvert -verbose -o Bin/translations/${j}_${i}.qm \
    $DIR/$BUILDDIR/${j}/translations/minos_${i}.qm \
    $DIR/$BUILDDIR/TinyXML/translations/minos_${i}.qm \
    $DIR/$BUILDDIR/XMPPLib/translations/minos_${i}.qm \
    $DIR/$BUILDDIR/MqtBase/translations/minos_${i}.qm \
    $DIR/$BUILDDIR/KeyerBase/translations/minos_${i}.qm \
    $DIR/$BUILDDIR/${j}/translations/minos_${i}.qm
  done
done

cp $QTDIR/translations/qt*.qm Bin/translations

for j in MqtAppStarter MqtChat MqtCluster MqtControl MqtKeyer MqtKSTClient MqtMonitor MqtPcCwKeyer MqtQrzServer MqtRigControl MqtRigRecorder MqtRigSync MqtRotator MqtServer MqtWinkeyer
do
  cp -r $DIR/$BUILDDIR/${j}/${j}.app Bin
done

cd ../../..

macdeployqt MqtLogger.app -qmldir=$DIR/mqt/MqtBase/QSOView

cd MqtLogger.app/Contents

sudo cp /usr/local/lib/libhamlib.4.dylib Frameworks

for j in MqtAppStarter MqtChat MqtCluster MqtControl MqtKeyer MqtKSTClient MqtMonitor MqtPcCwKeyer MqtQrzServer MqtRigControl MqtRigRecorder MqtRigSync MqtRotator MqtServer MqtWinkeyer
do
  ln -s ../../../../Frameworks   Resources/Bin/${j}.app/Contents/Frameworks
  ln -s ../../../../Plugins      Resources/Bin/${j}.app/Contents/Plugins
done

cd ../..

for j in MqtAppStarter MqtChat MqtCluster MqtControl MqtKeyer MqtKSTClient MqtMonitor MqtPcCwKeyer MqtQrzServer MqtRigControl MqtRigRecorder MqtRigSync MqtRotator MqtServer MqtWinkeyer
do
    install_name_tool -change /usr/local/lib/libhamlib.4.dylib @executable_path/../Frameworks/libhamlib.4.dylib MqtLogger.app/Contents/Resources/Bin/${j}.app/Contents/MacOS/${j}
    install_name_tool -change $QTDIR/lib/QtWidgets.framework/Versions/A/QtWidgets @executable_path/../Frameworks/QtWidgets.framework/Versions/A/QtWidgets MqtLogger.app/Contents/Resources/Bin/${j}.app/Contents/MacOS/${j}
    install_name_tool -change $QTDIR/lib/QtGui.framework/Versions/A/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/A/QtGui MqtLogger.app/Contents/Resources/Bin/${j}.app/Contents/MacOS/${j}
    install_name_tool -change $QTDIR/lib/QtSerialPort.framework/Versions/A/QtSerialPort @executable_path/../Frameworks/QtSerialPort.framework/Versions/A/QtSerialPort MqtLogger.app/Contents/Resources/Bin/${j}.app/Contents/MacOS/${j}
    install_name_tool -change $QTDIR/lib/QtNetwork.framework/Versions/A/QtNetwork @executable_path/../Frameworks/QtNetwork.framework/Versions/A/QtNetwork MqtLogger.app/Contents/Resources/Bin/${j}.app/Contents/MacOS/${j}
    install_name_tool -change $QTDIR/lib/QtCore.framework/Versions/A/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/A/QtCore MqtLogger.app/Contents/Resources/Bin/${j}.app/Contents/MacOS/${j}
	install_name_tool -change $QTDIR/lib/QtMultimedia.framework/Versions/A/QtMultimedia @executable_path/../Frameworks/QtMultimedia.framework/Versions/A/QtMultimedia MqtLogger.app/Contents/Resources/Bin/${j}.app/Contents/MacOS/${j}

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

wget https://www.rsgbcc.org/vhf/vhfcontests25.xml -O vhfcontests25.xml
wget https://www.rsgbcc.org/vhf/vhfcontests26.xml -O vhfcontests26.xml

wget https://www.rsgbcc.org/vhf/hfcontests25.xml -O hfcontests25.xml
wget https://www.rsgbcc.org/vhf/hfcontests26.xml -O hfcontests26.xml

wget https://microwave.rsgbcc.org/microcontests25.xml -O microcontests25.xml
wget https://microwave.rsgbcc.org/microcontests26.xml -O microcontests26.xml


wget https://microwave.rsgbcc.org/bartgcontests25.xml -O bartgcontests25.xml
wget https://microwave.rsgbcc.org/bartgcontests26.xml -O bartgcontests26.xml

cd ..


codesign --preserve-metadata=entitlements --force --verbose --sign "3rd Party Mac Developer Application: Phil Taylor (6255R636WZ)" --deep /Users/phil/source/minos-runtime/MqtLogger.app --entitlements /Users/phil/source/minos/mqt/ControlFiles/minos.entitlements
