#-------------------------------------------------
#
# Project created by QtCreator 2015-11-27T19:07:12
#
#-------------------------------------------------
include($$PWD/../mqt.pri)

QT       += core

TARGET = TinyXML
TEMPLATE = lib
CONFIG += staticlib

*g++*:CONFIG(release, debug|release): QMAKE_CXXFLAGS_WARN_ON -= -Wold-style-cast
else:*g++*:CONFIG(debug, debug|release):QMAKE_CXXFLAGS_WARN_ON -= -Wold-style-cast

SOURCES += tinyxml.cpp \
    tinyxmlerror.cpp \
    tinyxmlparser.cpp \
    TinyUtils.cpp

HEADERS += tinyxml.h \
    TinyUtils.h

