#-------------------------------------------------
#
# Project created by QtCreator 2015-11-27T19:07:12
#
#-------------------------------------------------
include($$PWD/../mqt.pri)

# Third party code - accept the warnings
QMAKE_CXXFLAGS_WARN_ON += -Wno-old-style-cast

QT       += core

TARGET = TinyXML
TEMPLATE = lib
CONFIG += staticlib

SOURCES += tinyxml.cpp \
    tinyxmlerror.cpp \
    tinyxmlparser.cpp \
    TinyUtils.cpp

HEADERS += tinyxml.h \
    TinyUtils.h

