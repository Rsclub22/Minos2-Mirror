include($$PWD/../mqt.pri)
include($$PWD/../mqtapplibs.pri)

QT += core gui network xml
QT += widgets

TARGET = MqtDataModes
TEMPLATE = app

win32:RC_ICONS += ../minos.ico

win32 {
    DEFINES += WIN32
    QT += axcontainer
}
CONFIG -= embed_manifest_exe
CONFIG -= embed_manifest_dll

QMAKE_MANIFEST += $$PWD/MqtDataModes.exe.manifest

SOURCES += \
    FLDigiFrame.cpp \
    MMTTYFrame.cpp \
    MMVARIFrame.cpp \
    engineconfigure.cpp \
    grittyframe.cpp \
    main.cpp \
    dmmainwindow.cpp \
    mmvarilib.cpp

HEADERS += \
    FLDigiFrame.h \
    MMTTYFrame.h \
    MMTTY_N1MM.h \
    MMVARIFrame.h \
    dmmainwindow.h \
    engineconfigure.h \
    grittyframe.h \
    mmvarilib.h

FORMS += \
    FLDigiFrame.ui \
    MMTTYFrame.ui \
    MMVARIFrame.ui \
    dmmainwindow.ui \
    engineconfigure.ui \
    grittyframe.ui

win32{ LIBS += -lwinmm}

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
