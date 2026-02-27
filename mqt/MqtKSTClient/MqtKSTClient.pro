include($$PWD/../mqt.pri)
include($$PWD/../keyerbase.pri)
include($$PWD/../mqtapplibs.pri)

QT       += core gui network
QT += widgets

TARGET = MqtKSTClient
TEMPLATE = app

win32:RC_ICONS += ../MinosKST.ico
macx:ICON=../MinosKST.icns

win32:LIBS += -lWs2_32

SOURCES += \
    KSTMinosParameters.cpp \
    activechatsframe.cpp \
    airscoutlink.cpp \
    buttonsframe.cpp \
    callsframe.cpp \
    changename.cpp \
    kstcallgridmodel.cpp \
    kstconfigure.cpp \
    kstmessagegridmodel.cpp \
    kstmonitoredlogs.cpp \
    kstpage.cpp \
    kstplanesmodel.cpp \
    kstscreenoptions.cpp \
    loginframe.cpp \
    main.cpp \
    kstmainwindow.cpp \
    msgframe.cpp \
    planesframe.cpp \
    sendmeepframe.cpp \
    tomeframe.cpp

HEADERS += \
    KSTMinosParameters.h \
    activechatsframe.h \
    airscoutlink.h \
    buttonsframe.h \
    callsframe.h \
    changename.h \
    kstcallgridmodel.h \
    kstconfigure.h \
    kstmainwindow.h \
    kstmessagegridmodel.h \
    kstmonitoredlogs.h \
    kstpage.h \
    kstplanesmodel.h \
    kstscreenoptions.h \
    loginframe.h \
    msgframe.h \
    planesframe.h \
    sendmeepframe.h \
    tomeframe.h

FORMS += \
    activechatsframe.ui \
    buttonsframe.ui \
    callsframe.ui \
    changename.ui \
    kstconfigure.ui \
    kstmainwindow.ui \
    kstmonitoredlogs.ui \
    loginframe.ui \
    msgframe.ui \
    planesframe.ui \
    sendmeepframe.ui \
    tomeframe.ui
