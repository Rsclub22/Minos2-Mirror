include($$PWD/../mqt.pri)

QT       += core gui network widgets

TEMPLATE = lib
CONFIG += staticlib
Target = KeyerBase

unix:!macos{DEFINES += __LINUX_ALSA__}
win32{DEFINES += __WINDOWS_DS__}

SOURCES += \
    ../rtaudio/RtAudio.cpp \
    ../Chunkware/SimpleComp.cpp \
    ../Chunkware/SimpleCompProcess.inl \
    ../Chunkware/SimpleEnvelope.cpp \
    VKMixer.cpp \
    levelmeter.cpp \
    riff.cpp

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
    VKMixer.h \
    ddc.h \
    levelmeter.h \
    riff.h

# Default rules for deployment.
unix {
    target.path = $$[QT_INSTALL_PLUGINS]/generic
}
!isEmpty(target.path): INSTALLS += target
