#-------------------------------------------------
#
# Project created by QtCreator 2017-07-05T13:31:47
#
#-------------------------------------------------
include($$PWD/../mqt.pri)
include($$PWD/../mqtapplibs.pri)
include($$PWD/../keyerbase.pri)

QT       += core gui charts
QT       += widgets
QT       += network

TARGET = mqtKeyerTest
TEMPLATE = app

INCLUDEPATH += $$PWD/../Chunkware

SOURCES += \
        main.cpp \
        mqtktMainWindow.cpp \
        mqtktWaveShowDialog.cpp

HEADERS += \
        mqtktMainWindow.h \
        mqtktWaveShowDialog.h

FORMS += \
        mqtktMainWindow.ui \
        mqtktWaveShowDialog.ui

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../MqtUtils/release/ -lMqtUtils
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../MqtUtils/debug/ -lMqtUtils
else:unix: LIBS += -L$$OUT_PWD/../MqtUtils/ -lMqtUtils

INCLUDEPATH += $$PWD/../MqtUtils
DEPENDPATH += $$PWD/../MqtUtils

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../MqtUtils/release/libMqtUtils.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../MqtUtils/debug/libMqtUtils.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../MqtUtils/release/MqtUtils.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../MqtUtils/debug/MqtUtils.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../MqtUtils/libMqtUtils.a
