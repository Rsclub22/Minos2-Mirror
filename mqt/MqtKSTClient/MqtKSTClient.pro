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
    airscoutlink.cpp \
    changename.cpp \
    kstactivechatsframe.cpp \
    kstbuttonsframe.cpp \
    kstcallgridmodel.cpp \
    kstcallsframe.cpp \
    kstconfigure.cpp \
    kstloginframe.cpp \
    kstmainframe.cpp \
    kstmessagegridmodel.cpp \
    kstmonitoredlogs.cpp \
    kstmsgframe.cpp \
    kstpageframe.cpp \
    kstplanesframe.cpp \
    kstplanesmodel.cpp \
    kstscreenoptions.cpp \
    kstsendmeepframe.cpp \
    ksttomeframe.cpp \
    main.cpp \
    kstmainwindow.cpp

HEADERS += \
    KSTMinosParameters.h \
    airscoutlink.h \
    changename.h \
    kstactivechatsframe.h \
    kstbuttonsframe.h \
    kstcallgridmodel.h \
    kstcallsframe.h \
    kstconfigure.h \
    kstloginframe.h \
    kstmainframe.h \
    kstmainwindow.h \
    kstmessagegridmodel.h \
    kstmonitoredlogs.h \
    kstmsgframe.h \
    kstpageframe.h \
    kstplanesframe.h \
    kstplanesmodel.h \
    kstscreenoptions.h \
    kstsendmeepframe.h \
    ksttomeframe.h

FORMS += \
    changename.ui \
    kstactivechatsframe.ui \
    kstbuttonsframe.ui \
    kstcallsframe.ui \
    kstconfigure.ui \
    kstloginframe.ui \
    kstmainframe.ui \
    kstmainwindow.ui \
    kstmonitoredlogs.ui \
    kstmsgframe.ui \
    kstpageframe.ui \
    kstplanesframe.ui \
    kstsendmeepframe.ui \
    ksttomeframe.ui
