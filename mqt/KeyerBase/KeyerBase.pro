include($$PWD/../mqt.pri)

QT       += core gui network widgets charts

TEMPLATE = lib
CONFIG += staticlib
Target = KeyerBase

INCLUDEPATH += $$PWD/../rtaudio
INCLUDEPATH += $$PWD/../Chunkware
INCLUDEPATH += ../MqtUtils
INCLUDEPATH += ../MqtBase
INCLUDEPATH += ../XMPPLib
INCLUDEPATH += ../TinyXML



unix:!macos{DEFINES += __LINUX_ALSA__}
win32{DEFINES += __WINDOWS_DS__}

SOURCES += \
    ../rtaudio/RtAudio.cpp \
    ../Chunkware/SimpleComp.cpp \
    ../Chunkware/SimpleCompProcess.inl \
    ../Chunkware/SimpleEnvelope.cpp \
    KeyerJson.cpp \
    VKMixer.cpp \
    WaveShowDialog.cpp \
    keyerBase.cpp \
    levelmeter.cpp \
    riff.cpp \
    sbdriver.cpp \
    soundsys.cpp

HEADERS += \
    ../rtaudio/RtAudio.h \
    ../rtaudio/include/dsound.h \
    ../rtaudio/include/ginclude.h \
    ../rtaudio/include/iasiodrv.h \
    ../rtaudio/include/soundcard.h \
    ../Chunkware/SimpleComp.h \
    ../Chunkware/SimpleEnvelope.h \
    ../Chunkware/SimpleGain.h \
    ../Chunkware/SimpleHeader.h \
    KeyerJson.h \
    VKMixer.h \
    WaveShowDialog.h \
    ddc.h \
    keyctrl.h \
    keyerBase.h \
    keyerlog.h \
    levelmeter.h \
    riff.h \
    sbdriver.h \
    soundsys.h

# Default rules for deployment.
unix {
    target.path = $$[QT_INSTALL_PLUGINS]/generic
}
!isEmpty(target.path): INSTALLS += target

FORMS += \
    WaveShowDialog.ui
