include($$PWD/../mqt.pri)
include($$PWD/../mqtapplibs.pri)

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
