include($$PWD/../mqt.pri)

QT       += core gui network widgets charts

TEMPLATE = lib
CONFIG += staticlib
Target = KeyerBase

INCLUDEPATH += ../rtaudio
INCLUDEPATH += ../Chunkware
INCLUDEPATH += ../filter_c
INCLUDEPATH += ../MqtUtils
INCLUDEPATH += ../MqtBase
INCLUDEPATH += ../XMPPLib
INCLUDEPATH += ../TinyXML

unix:!macos{DEFINES += __LINUX_ALSA__}
win32{DEFINES += __WINDOWS_WASAPI__}

SOURCES += \
    ../filter_c/adis_filter.c \
    ../rtaudio/RtAudio.cpp \
    ../Chunkware/SimpleComp.cpp \
    ../Chunkware/SimpleCompProcess.inl \
    ../Chunkware/SimpleEnvelope.cpp \
    CompressorParams.cpp \
    KeyerJson.cpp \
    MqtLogCompressor.cpp \
    SliderSpinner.cpp \
    VKMixer.cpp \
    WaveShowDialog.cpp \
    databuffer.cpp \
    inbuff.cpp \
    ipsystem.cpp \
    keyerBase.cpp \
    levelmeter.cpp \
    riff.cpp \
    riffwriter.cpp \
    sbdriver.cpp \
    soundsys.cpp \
    vudata.cpp

HEADERS += \
    ../filter_c/adis_filter.h \
    ../rtaudio/RtAudio.h \
    ../rtaudio/include/dsound.h \
    ../rtaudio/include/ginclude.h \
    ../rtaudio/include/iasiodrv.h \
    ../rtaudio/include/soundcard.h \
    ../Chunkware/SimpleComp.h \
    ../Chunkware/SimpleEnvelope.h \
    ../Chunkware/SimpleGain.h \
    ../Chunkware/SimpleHeader.h \
    CompressorParams.h \
    KeyerJson.h \
    MqtLogCompressor.h \
    SliderSpinner.h \
    VKMixer.h \
    WaveShowDialog.h \
    databuffer.h \
    ddc.h \
    inbuff.h \
    ipsystem.h \
    keyctrl.h \
    keyerBase.h \
    keyerlog.h \
    levelmeter.h \
    riff.h \
    riffwriter.h \
    sbdriver.h \
    soundsys.h \
    vudata.h

# Default rules for deployment.
unix {
    target.path = $$[QT_INSTALL_PLUGINS]/generic
}
!isEmpty(target.path): INSTALLS += target

FORMS += \
    SliderSpinner.ui \
    WaveShowDialog.ui
