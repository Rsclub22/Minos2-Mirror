include($$PWD/../mqt.pri)
include($$PWD/../mqtapplibs.pri)
include($$PWD/../keyerbase.pri)

QT       += core gui network widgets

TARGET = MqtRigRecorder
TEMPLATE = app


win32:RC_ICONS += ../MinosRecorder.ico
INCLUDEPATH += $$PWD/../rtaudio

unix:!macos{DEFINES += __LINUX_ALSA__}
win32{DEFINES += __WINDOWS_DS__}


SOURCES += \
    main.cpp \
    rcmainwindow.cpp \
    rrsoundsys.cpp

HEADERS += \
    rcmainwindow.h \
    rr_pch.h \
    rrsoundsys.h \
    vucallback.h

FORMS += \
    rcmainwindow.ui
