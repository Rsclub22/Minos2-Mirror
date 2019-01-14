#-------------------------------------------------
#
# Project created by QtCreator 2019-01-13T14:55:33
#
#-------------------------------------------------

include($$PWD/../mqt.pri)
include($$PWD/../mqtapplibs.pri)

QT       += core gui network widgets

TARGET = MqtWsjtxConnector
TEMPLATE = app
win32:RC_ICONS += ../minos.ico

SOURCES += \
        main.cpp \
        WConMain.cpp \
    MessageServer.cpp \
    NetworkMessage.cpp \
    qt_helpers.cpp \
    Radio.cpp \
    RadioMetaType.cpp

HEADERS += \
        WConMain.h \
    MessageServer.hpp \
    NetworkMessage.hpp \
    pimpl_h.hpp \
    pimpl_impl.hpp \
    qt_helpers.hpp \
    Radio.hpp

FORMS += \
        WConMain.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
