TEMPLATE = subdirs

SUBDIRS += \
    MqtUtils \
    TinyXML \
    XMPPLib \
    MqtBase \
    RPCTest \
    MqtAppStarter \
    MqtControl \
    MqtChat \
    MqtCluster\
    MqtKeyer \
    MqtLogger \
    MqtServer \
    MqtRotator \
    MqtBandmap \
    MqtMonitor \
    MqtRigControl \
    Qs1rSync \
    MqtWsjtxConnector

win32 {
greaterThan(QT_MAJOR_VERSION, 4) : greaterThan(QT_MINOR_VERSION, 8) {
   SUBDIRS += mqtKeyerTest
}
}


RPCTest.depends = MqtUtils TinyXML XMPPLib MqtBase
MqtAppStarter.depends = MqtUtils TinyXML XMPPLib MqtBase
MqtBandmap.depends = MqtUtils TinyXML XMPPLib MqtBase
MqtChat.depends = MqtUtils TinyXML XMPPLib MqtBase
MqtCluster.depends = MqtUtils TinyXML XMPPLib MqtBase
MqtControl.depends = MqtUtils TinyXML XMPPLib MqtBase
MqtKeyer.depends = MqtUtils TinyXML XMPPLib MqtBase
MqtLogger.depends = MqtUtils TinyXML XMPPLib MqtBase
MqtMonitor.depends = MqtUtils TinyXML XMPPLib MqtBase
MqtServer.depends = MqtUtils TinyXML XMPPLib MqtBase
MqtRigControl.depends = MqtUtils TinyXML XMPPLib MqtBase
MqtRotator.depends = MqtUtils TinyXML XMPPLib MqtBase
Qs1rSync.depends = MqtUtils TinyXML XMPPLib MqtBase


