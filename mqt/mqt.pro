TEMPLATE = subdirs

SUBDIRS += \
    KeyerBase \
    MqtBase \
    MqtUtils \
    TinyXML \
    XMPPLib \
    RPCTest \
    MqtAppStarter \
    MqtControl \
    MqtChat \
    MqtCluster\
    MqtKeyer \
    MqtKSTClient \
    MqtLogger \
    MqtMonitor \
    MqtQrzServer \
    MqtRigControl \
    MqtRigSync \
    MqtRotator \
    MqtServer \
    Qs1rSync

win32 {
greaterThan(QT_MAJOR_VERSION, 4) : greaterThan(QT_MINOR_VERSION, 8) {
   SUBDIRS += mqtKeyerTest
   mqtKeyerTest.depends = MqtUtils TinyXML XMPPLib MqtBase KeyerBase
}
}


KeyerBase.depends = MqtUtils TinyXML XMPPLib
RPCTest.depends = MqtUtils TinyXML XMPPLib MqtBase
MqtAppStarter.depends = MqtUtils TinyXML XMPPLib MqtBase
MqtChat.depends = MqtUtils TinyXML XMPPLib MqtBase
MqtCluster.depends = MqtUtils TinyXML XMPPLib MqtBase
MqtControl.depends = MqtUtils TinyXML XMPPLib MqtBase
MqtKeyer.depends = MqtUtils TinyXML XMPPLib MqtBase KeyerBase
MqtKSTClient.depends = MqtUtils TinyXML XMPPLib MqtBase
MqtLogger.depends = MqtUtils TinyXML XMPPLib MqtBase
MqtMonitor.depends = MqtUtils TinyXML XMPPLib MqtBase
MqtQrzServer.depends = MqtUtils TinyXML XMPPLib MqtBase
MqtRigControl.depends = MqtUtils TinyXML XMPPLib MqtBase
MqtRigSync.depends = MqtUtils TinyXML XMPPLib MqtBase
MqtRotator.depends = MqtUtils TinyXML XMPPLib MqtBase
MqtServer.depends = MqtUtils TinyXML XMPPLib MqtBase
Qs1rSync.depends = MqtUtils TinyXML XMPPLib MqtBase


