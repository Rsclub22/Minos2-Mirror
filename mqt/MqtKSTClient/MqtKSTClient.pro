include($$PWD/../mqt.pri)
include($$PWD/../mqtapplibs.pri)

QT       += core gui network
QT += widgets

TARGET = MqtKSTClient
TEMPLATE = app

win32:RC_ICONS += ../MinosKST.ico

win32:LIBS += -lWs2_32

SOURCES += \
    KSTMinosParameters.cpp \
    airscoutlink.cpp \
    changename.cpp \
    kstcallgridmodel.cpp \
    kstconfigure.cpp \
    kstmessagegridmodel.cpp \
    kstplanesmodel.cpp \
    main.cpp \
    kstmainwindow.cpp

HEADERS += \
    KSTMinosParameters.h \
    airscoutlink.h \
    changename.h \
    kstcallgridmodel.h \
    kstconfigure.h \
    kstmainwindow.h \
    kstmessagegridmodel.h \
    kstplanesmodel.h

FORMS += \
    changename.ui \
    kstconfigure.ui \
    kstmainwindow.ui
