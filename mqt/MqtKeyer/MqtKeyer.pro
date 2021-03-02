#-------------------------------------------------
#
# Project created by QtCreator 2016-09-16T11:01:45
#
#-------------------------------------------------
include($$PWD/../mqt.pri)
include($$PWD/../mqtapplibs.pri)

QT       += core gui network widgets

TARGET = MqtKeyer
TEMPLATE = app

win32:RC_ICONS += ../MinosKeyer.ico

unix:!macos{DEFINES += __LINUX_ALSA__}
win32{DEFINES += __WINDOWS_DS__}
INCLUDEPATH += $$PWD/../rtaudio
INCLUDEPATH += $$PWD/../Chunkware

SOURCES += main.cpp\
        KeyerMain.cpp \
    keyerAbout.cpp \
    keyconf.cpp \
    KeyerRPCServer.cpp \
    keyers.cpp \
    portcon.cpp \
    sbdriver.cpp \
    soundsys.cpp \
    windowMonitor.cpp

HEADERS  += KeyerMain.h \
    keyerAbout.h \
    keyconf.h \
    keyctrl.h \
    keyerlog.h \
    KeyerRPCServer.h \
    keyers.h \
    portcon.h \
    sbdriver.h \
    soundsys.h \
    windowMonitor.h

FORMS    += KeyerMain.ui \
    keyerAbout.ui \
    windowMonitor.ui

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../KeyerBase/release/ -lKeyerBase
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../KeyerBase/debug/ -lKeyerBase
else:unix: LIBS += -L$$OUT_PWD/../KeyerBase/ -lKeyerBase

INCLUDEPATH += $$PWD/../KeyerBase
DEPENDPATH += $$PWD/../KeyerBase

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../KeyerBase/release/libKeyerBase.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../KeyerBase/debug/libKeyerBase.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../KeyerBase/release/KeyerBase.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../KeyerBase/debug/KeyerBase.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../KeyerBase/libKeyerBase.a

# include system libs last, so they get included for KeyerBase

unix:!macos{ LIBS += -lasound}
win32{ LIBS += -lole32 -lwinmm -luuid -lksuser -ldsound -lUser32}
