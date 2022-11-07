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
    mqtKeyerTest \
    MqtKSTClient \
    MqtLogger \
    MqtMonitor \
    MqtQrzServer \
    MqtRigControl \
    MqtRigRecorder \
    MqtRigSync \
    MqtRotator \
    MqtServer

KeyerBase.depends = MqtUtils TinyXML XMPPLib
RPCTest.depends = MqtUtils TinyXML XMPPLib MqtBase
MqtAppStarter.depends = MqtUtils TinyXML XMPPLib MqtBase
MqtChat.depends = MqtUtils TinyXML XMPPLib MqtBase
MqtCluster.depends = MqtUtils TinyXML XMPPLib MqtBase
MqtControl.depends = MqtUtils TinyXML XMPPLib MqtBase
MqtKeyer.depends = MqtUtils TinyXML XMPPLib MqtBase KeyerBase
mqtKeyerTest.depends = MqtUtils TinyXML XMPPLib MqtBase KeyerBase
MqtRigRecorder.depends = MqtUtils TinyXML XMPPLib MqtBase KeyerBase
MqtKSTClient.depends = MqtUtils TinyXML XMPPLib MqtBase
MqtLogger.depends = MqtUtils TinyXML XMPPLib MqtBase KeyerBase
MqtMonitor.depends = MqtUtils TinyXML XMPPLib MqtBase
MqtQrzServer.depends = MqtUtils TinyXML XMPPLib MqtBase
MqtRigControl.depends = MqtUtils TinyXML XMPPLib MqtBase
MqtRigSync.depends = MqtUtils TinyXML XMPPLib MqtBase
MqtRotator.depends = MqtUtils TinyXML XMPPLib MqtBase
MqtServer.depends = MqtUtils TinyXML XMPPLib MqtBase


