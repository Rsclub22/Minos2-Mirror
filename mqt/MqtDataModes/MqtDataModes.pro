include($$PWD/../mqt.pri)
include($$PWD/../mqtapplibs.pri)

QT += core gui network xml
QT += widgets

TARGET = MqtDataModes
TEMPLATE = app

SOURCES += \
    FLDigiFrame.cpp \
    datapainter.cpp \
    engineconfigure.cpp \
    main.cpp \
    dmmainwindow.cpp \
    rxbuffer.cpp

HEADERS += \
    FLDigiFrame.h \
    datapainter.h \
    dmmainwindow.h \
    engineconfigure.h \
    rxbuffer.h

FORMS += \
    FLDigiFrame.ui \
    dmmainwindow.ui \
    engineconfigure.ui

win32{
CONFIG -= embed_manifest_exe
CONFIG -= embed_manifest_dll

RC_ICONS += ../minos.ico

DEFINES += WIN32
QT += axcontainer

QMAKE_MANIFEST += $$PWD/MqtDataModes.exe.manifest

SOURCES += \
    MMTTYFrame.cpp \
    MMVARIFrame.cpp \
    mmvarilib.cpp \
    grittyframe.cpp \

HEADERS += \
    MMTTYFrame.h \
    MMTTY_N1MM.h \
    MMVARIFrame.h \
    mmvarilib.h \
    grittyframe.h \

FORMS += \
    MMTTYFrame.ui \
    MMVARIFrame.ui \
    grittyframe.ui \

LIBS += -lwinmm}

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../maia/release/ -lmaia
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../maia/debug/ -lmaia
else:unix: LIBS += -L$$OUT_PWD/../maia/ -lmaia

INCLUDEPATH += $$PWD/../maia/maia
DEPENDPATH += $$PWD/../maia/maia

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../maia/release/libmaia.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../maia/debug/libmaia.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../maia/release/maia.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../maia/debug/maia.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../maia/libmaia.a
