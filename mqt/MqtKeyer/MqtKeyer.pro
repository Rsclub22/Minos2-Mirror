#-------------------------------------------------
#
# Project created by QtCreator 2016-09-16T11:01:45
#
#-------------------------------------------------
include($$PWD/../mqt.pri)
include($$PWD/../mqtapplibs.pri)

QT       += core gui network widgets
QT       += multimedia

TARGET = MqtKeyer
TEMPLATE = app

win32:RC_ICONS += ../MinosKeyer.ico

unix:!macos{DEFINES += __LINUX_ALSA__}
win32{DEFINES += __WINDOWS_DS__}
INCLUDEPATH += $$PWD/../rtaudio
INCLUDEPATH += $$PWD/../Chunkware

unix:!macos{ LIBS += -lasound}
win32{ LIBS += -lole32 -lwinmm -luuid -lksuser -ldsound -lUser32}

SOURCES += main.cpp\
        KeyerMain.cpp \
    keyerAbout.cpp \
    keyconf.cpp \
    KeyerRPCServer.cpp \
    keyers.cpp \
    portcon.cpp \
    riff.cpp \
    sbdriver.cpp \
    soundsys.cpp \
    VKMixer.cpp \
    levelmeter.cpp \
    windowMonitor.cpp \
    ../rtaudio/RtAudio.cpp \
    ../Chunkware/SimpleComp.cpp \
    ../Chunkware/SimpleCompProcess.inl \
    ../Chunkware/SimpleEnvelope.cpp

HEADERS  += KeyerMain.h \
    keyerAbout.h \
    ddc.h \
    keyconf.h \
    keyctrl.h \
    keyerlog.h \
    KeyerRPCServer.h \
    keyers.h \
    portcon.h \
    riff.h \
    sbdriver.h \
    soundsys.h \
    VKMixer.h \
    levelmeter.h \
    windowMonitor.h \
    ../rtaudio/RtAudio.h \
    ../rtaudio/include/dsound.h \
    ../rtaudio/include/ginclude.h \
    ../rtaudio/include/iasiodrv.h \
    ../rtaudio/include/soundcard.h \
    ../Chunkware/SimpleComp.h \
    ../Chunkware/SimpleEnvelope.h \
    ../Chunkware/SimpleGain.h \
    ../Chunkware/SimpleHeader.h

FORMS    += KeyerMain.ui \
    keyerAbout.ui \
    windowMonitor.ui


