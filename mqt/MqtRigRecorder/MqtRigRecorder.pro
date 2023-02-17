include($$PWD/../mqt.pri)
include($$PWD/../keyerbase.pri)
include($$PWD/../mqtapplibs.pri)

QT       += core gui network widgets

TARGET = MqtRigRecorder
TEMPLATE = app


win32:RC_ICONS += ../MinosRecorder.ico

unix:!macos{DEFINES += __LINUX_ALSA__}
win32{DEFINES += __WINDOWS_WASAPI__}
INCLUDEPATH += ../rtaudio


SOURCES += \
    main.cpp \
    rcmainwindow.cpp \
    rrsoundsys.cpp

HEADERS += \
    rcmainwindow.h \
    rrsoundsys.h \
    vucallback.h

FORMS += \
    rcmainwindow.ui
