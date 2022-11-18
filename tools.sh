#!/bin/bash

apt-get update -y
apt-get upgrade -y

apt-get install git -y
apt-get install git-gui -y

apt-get  install libtool -y
apt-get install automake -y

apt-get install cmake -y
apt-get install build-essential -y
apt-get install qtbase5-dev qtchooser qt5-qmake qtbase5-dev-tools
apt-get install qtcreator -y
apt-get install qtmultimedia5-dev libqt5multimediawidgets5 libqt5multimedia5-plugins libqt5multimedia5 -y
apt-get install libqt5serialport5 -y
apt-get install libqt5serialport5-dev -y
apt-get install qttools5-dev -y
apt-get install libqt5charts5-dev -y

# QML needed for maps, but it all seems broken under Linux
# so for now we leave it out

#apt-get install qtdeclarative5-dev -y
#apt-get install qml-module-qtquick-controls2 -y
#apt-get install qtlocation5-dev -y
#apt-get install qtpositioning-dev -y
#apt-get install qml-module-qtlocation -y
#apt-get install qml-module-qtpositioning -y

#apt-get install libglu1-mesa-dev freeglut3-dev mesa-common-dev -y

# ubuntu 21 doesn't seem to have g++ by default

apt-get install g++ -y
apt-get install libasound2 -y
apt-get install libasound2-dev -y

apt remove libhamlib2 -y

apt-get autoremove -y

