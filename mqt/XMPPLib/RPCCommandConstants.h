#ifndef RPCCOMMANDRCONSTANTS_H
#define RPCCOMMANDRCONSTANTS_H
#include <QString>

// we DON'T want these translated - so QT_TR_NOOP in not required
namespace rpcConstants
{
// PubSub

inline static const QString publish =            QStringLiteral("Minos:PubSub:Publish");
inline static const QString routerSubscribe =    QStringLiteral("Minos:PubSub:ServerSubscribe");
inline static const QString remoteSubscribe =    QStringLiteral("Minos:PubSub:RemoteSubscribe");
inline static const QString subscribe =          QStringLiteral("Minos:PubSub:Subscribe");
inline static const QString routerNotify =       QStringLiteral("Minos:PubSub:ServerNotify");
inline static const QString clientNotify =       QStringLiteral("Minos:PubSub:ClientNotify");

inline static const QString selected =           QStringLiteral("Selected");
inline static const QString loggerUuid =         QStringLiteral("logger");

inline static const QString paramName =          QStringLiteral("Name");
inline static const QString paramValue=          QStringLiteral("Value");

// Server Categories

inline static const QString StationCategory =         QStringLiteral("Station");
inline static const QString LocalStationCategory =    QStringLiteral("LocalStation");
// station name is the key, IP is the value

// Logger
inline static const QString loggerApp =             QStringLiteral("MinosQtLogger");
inline static const QString LoggerCategory =        QStringLiteral("MinosLogger");
inline static const QString loggerTakeFocus =       QStringLiteral("Minos:Logger:TakeFocus");
inline static const QString currentLogCategory =    QStringLiteral("Minos:CurrentLog");
// Monitor
inline static const QString monitorApp =            QStringLiteral("mqtMonitor");
inline static const QString monitorLogCategory =    QStringLiteral("MonitorLog");
inline static const QString loggerStanzaRequest =   QStringLiteral("Minos:Logger:GetStanza");
inline static const QString loggerStanzaResponse =  QStringLiteral("Minos:Logger:StanzaReturned");



// Cluster
inline static const QString clusterApp =            QStringLiteral("mqtCluster");
inline static const QString clusterMethod =         QStringLiteral("Minos:Cluster");
inline static const QString clusterClientServer =   QStringLiteral("MinosClusterClientServer");
inline static const QString sendClusterSpot =       QStringLiteral("SendClusterSpot");
inline static const QString txSpotToCluster =       QStringLiteral("txSpotToCluster");
inline static const QString txSpotParamFreq =       QStringLiteral("Freq");
inline static const QString txSpotParamCallsign =   QStringLiteral("Callsign");
inline static const QString txSpotParamLocator =    QStringLiteral("Locator");
inline static const QString txSpotParamUTC =        QStringLiteral("UTC");
inline static const QString clusterResendSpots =    QStringLiteral("MinosClusterResend");
inline static const QString clusterResendSpotsCmd = QStringLiteral("MinosClusterResendCmd");
inline static const QString clusterCategory =       QStringLiteral("ClusterCategory");
inline static const QString clusterReport =         QStringLiteral("ClusterReport");
inline static const QString clusterTXSpotEnableState = QStringLiteral("ClusterTXSpotEnableState");
inline static const QString clusterBandmask =       QStringLiteral("ClusterBandmask");
inline static const QString clusterFrameId =        QStringLiteral("ClusterFrameId");
inline static const QString clusterReconnect =      QStringLiteral("ClusterReconnect");


// QRZ Server
inline static const QString qrzServerApp =          QStringLiteral("qrzServerApp");
inline static const QString qrzDisplayApp =         QStringLiteral("qrzDisplayApp");
inline static const QString qrzCategory =           QStringLiteral("MinosQRZ");
inline static const QString qrzMethod =             QStringLiteral("Minos:QrzServer");
inline static const QString qrzCluster =            QStringLiteral("qrzCluster");
inline static const QString qrzLogger =             QStringLiteral("qrzLogger");
inline static const QString qrzLogFrameId =         QStringLiteral("qrzLogFrameId");
inline static const QString qrzClusterResponse =    QStringLiteral("qrzClusterResponse");
inline static const QString qrzLoggerResponse =     QStringLiteral("qrzLoggerResponse");
inline static const QString qrzSource =             QStringLiteral("qrzSource");
inline static const QString qrzDxCallsign =         QStringLiteral("qrzDxCallsign");
inline static const QString qrzSpotterCallsign =    QStringLiteral("qrzSpotterCallsign");
inline static const QString qrzFirstName =          QStringLiteral("qrzFirstName");
inline static const QString qrzName =               QStringLiteral("qrzName");
inline static const QString qrzAddr1 =              QStringLiteral("qrzAddr1");
inline static const QString qrzAddr2 =              QStringLiteral("qrzAddr2");
inline static const QString qrzCounty =             QStringLiteral("qrzCounty");
inline static const QString qrzCountry =            QStringLiteral("qrzCountry");
inline static const QString qrzLat =                QStringLiteral("qrzLat");
inline static const QString qrzLon =                QStringLiteral("qrzLon");
inline static const QString qrzDxGrid =             QStringLiteral("qrzDxGrid");
inline static const QString qrzSpotterGrid =        QStringLiteral("qrzSpotterGrid");
inline static const QString qrzCqZone =             QStringLiteral("qrzCqZone");
inline static const QString qrzItuZone =            QStringLiteral("qrzItuZone");
inline static const QString qrzServerLogonState =   QStringLiteral("qrzServerLogonState");
inline static const QString qrzServerStateMessage = QStringLiteral("qrzServerStateMessage");
inline static const QString qrzServerState =        QStringLiteral("qrzServerState");
inline static const QString qrzDxReplyState =       QStringLiteral("qrzDxReplyState");
inline static const QString qrzSpotterReplyState =  QStringLiteral("qrzSpotterReplyState");
inline static const QString qrzmoddate =            QStringLiteral("qrzmoddate");
inline static const QString qrzdbdate =             QStringLiteral("qrzdbdate");
inline static const QString qrzmessage =            QStringLiteral("qrzmessage");
inline static const QString qrzdbrecords =          QStringLiteral("qrzdbrecords");
inline static const QString qrzdbrecalls =          QStringLiteral("qrzdbrecallsdate");
inline static const QString qrzrecalls =            QStringLiteral("qrzrecalls");

inline static const QString qrzServerLoggedOut =    QStringLiteral("qrzServerLoggedOut");
inline static const QString qrzServerLoggedIn =     QStringLiteral("qrzServerLoggedIn");
inline static const QString qrzServerLoggedState =  QStringLiteral("qrzServerLoggedState");
inline static const QString qrzServerCallOK =       QStringLiteral("qrzServerCallOK");

// Chat
inline static const QString chatApp =               QStringLiteral("mqtChat");
inline static const QString chatMethod =            QStringLiteral("Minos:Chat");
inline static const QString SendChatMessage =       QStringLiteral("SendChatMessage");
inline static const QString ChatResult =            QStringLiteral("ChatResult");
inline static const QString ChatCategory =          QStringLiteral("MinosChat");
inline static const QString ChatServer =            QStringLiteral("MinosChatServer");
inline static const QString ChatServerFrequency =   QStringLiteral("MinosChatServerFrequency");

// Keyer
inline static const QString keyerApp =              QStringLiteral("mqtKeyer");
inline static const QString keyerMethod =           QStringLiteral("Minos:KeyerControl");
inline static const QString keyerUser =             QStringLiteral("KeyerUser");
inline static const QString keyerPlayFile =         QStringLiteral("PlayFile");
inline static const QString keyerRecordFile =       QStringLiteral("RecordFile");
inline static const QString keyerTone =             QStringLiteral("Tone");
inline static const QString keyerTwoTone =          QStringLiteral("TwoTone");
inline static const QString keyerStop =             QStringLiteral("Stop");
inline static const QString keyerConfig =           QStringLiteral("Config");
inline static const QString keyerMeter =            QStringLiteral("Meter");
inline static const QString keyerSliders =          QStringLiteral("Sliders");
inline static const QString keyerSendConfig =       QStringLiteral("SendConfig");
inline static const QString keyerSendMS =           QStringLiteral("SendMS");
inline static const QString keyerListen =           QStringLiteral("listen");

inline static const QString KeyerCategory =         QStringLiteral("Keyer");
inline static const QString KeyerConfigCategory =   QStringLiteral("KeyerConfig");
inline static const QString keyerReport =           QStringLiteral("Report");

inline static const QString keyerProxyApp =         QStringLiteral("mqtKeyerProxy");


// Winkeyer
inline static const QString winKeyerApp =           QStringLiteral("mqtWinKeyer");
inline static const QString winKeyerMethod =        QStringLiteral("Minos:winKeyer");
inline static const QString cwMessageToWinKeyer =   QStringLiteral("cwMessageToWinKeyer");
inline static const QString cwStopToWinKeyer =      QStringLiteral("cwStopWinKeyer");
inline static const QString winKeyerCwMessage =     QStringLiteral("winKeyerCwMessage");
inline static const QString winKeyerStopMsg =       QStringLiteral("winKeyerStopMsg");
inline static const QString winKeyerCategory =      QStringLiteral("winKeyerCategory");
inline static const QString winKeyerReport =        QStringLiteral("winKeyerReport");
inline static const QString winKeyerPttEnabled =    QStringLiteral("winKeyerPttEnabled");
inline static const QString winKeyerTxOn =          QStringLiteral("winKeyerTxOn");
inline static const QString cwWpmToWinKeyer =       QStringLiteral("cwWpmToWinKeyer");
inline static const QString winKeyerWpm =           QStringLiteral("winKeyerWpm");
inline static const QString winWpmToLog =           QStringLiteral("winWpmToLog");


// PcCwKeyer
inline static const QString pcCwKeyerApp =          QStringLiteral("mqtPcCwKeyer");
inline static const QString pcCwKeyerMethod =       QStringLiteral("Minos:PcCwKeyer");
inline static const QString cwMessageToPcCwKeyer =  QStringLiteral("cwMessageToPcCwKeyer");
inline static const QString cwStopToPcCwKeyer =     QStringLiteral("cwStopToPcCwKeyer");
inline static const QString pcCwKeyerCwMessage =    QStringLiteral("pcCwKeyerCwMessage");
inline static const QString pcCwKeyerStopMsg =      QStringLiteral("pcCwKeyerStopMsg");
inline static const QString pcCwKeyerCategory =     QStringLiteral("PcCwKeyerCategory");
inline static const QString pcCwKeyerReport =       QStringLiteral("PcCwKeyerReport");
inline static const QString pcCwKeyerPttEnabled =   QStringLiteral("pcCwKeyerPttEnabled");
inline static const QString pcCwKeyerTxOn =         QStringLiteral("pcCwKeyerTxOn");
inline static const QString cwWpmToPcCwKeyer =      QStringLiteral("cwWpmToPcCwKeyer");
inline static const QString pcCwKeyerWpm =          QStringLiteral("pcCwKeyerWpm");
inline static const QString pcCwWpmToLog =          QStringLiteral("pcCwWpmToLog");

// Control
inline static const QString controlApp =            QStringLiteral("mqtControl");
inline static const QString lineControlCategory =   QStringLiteral("LineControl");

inline static const QString controlParamName =      QStringLiteral("Name");
inline static const QString controlParamLine =      QStringLiteral("Line");
inline static const QString controlResult =         QStringLiteral("ControlResult");
inline static const QString controlLineState =      QStringLiteral("LineState");
inline static const QString controlState =          QStringLiteral("CtrlState");
inline static const QString controlGetLine =        QStringLiteral("GetLine");
inline static const QString controlSetLine =        QStringLiteral("SetLine");
inline static const QString controlSetTransverter = QStringLiteral("SetTransverter");
inline static const QString lineSet =               QStringLiteral("set");



// Rotator

enum RotateDirection :int {eRotateLeft, eRotateDirect, eRotateRight, eRotateStop};

inline static const QString rotatorApp =            QStringLiteral("mqtRotator");
inline static const QString rotatorMethod =         QStringLiteral("Minos:Rotator");
inline static const QString rotatorResult =         QStringLiteral("RotatorResult");

inline static const QString rotatorParamDirection =   QStringLiteral("RotatorDirection");
inline static const QString rotatorParamAngle =     QStringLiteral("RotatorAngle");

inline static const QString RotatorCategory =       QStringLiteral("Rotator");
inline static const QString rotatorDetailCategory = QStringLiteral("RotatorDetail");
inline static const QString rotatorStateCategory =  QStringLiteral("RotatorState");
inline static const QString rotatorPresetsCategory =  QStringLiteral("RotatorPresets");
inline static const QString rotatorSkyScanPresetsCategory = QStringLiteral("RotatorSkyScanPresets");
inline static const QString rotatorStatus =         QStringLiteral("RotStatus");
inline static const QString rotatorBearing =        QStringLiteral("Bearing");
inline static const QString rotatorList =           QStringLiteral("rotList");
inline static const QString rotPreset =             QStringLiteral("RotPreset");
inline static const QString rotPresetList =         QStringLiteral("RotPresetList");
inline static const QString rotSkyScanPresetList =  QStringLiteral("RotSkyScanPreset");
inline static const QString rotCwCcwCmdEnable =     QStringLiteral("RotCwCcwCmdEnable");

inline static const QString rotatorSelectAntennaName = QStringLiteral("SelectAntennaName");
inline static const QString rotatorMaxAzimuth =     QStringLiteral("MaxAzimuth");
inline static const QString rotatorMinAzimuth =     QStringLiteral("MinAzimuth");
inline static const QString supportStopCommand =    QStringLiteral("SupportStopCommand");
inline static const QString rotatorStopSouthStopOffsetData = QStringLiteral("StopSouthStopOffsetData");


// SkyScan

inline static const QString skyScanVisible =        QStringLiteral("SkyScanVisible");
inline static const QString skyScanStartBearing =   QStringLiteral("SkyScanStartBearing");
inline static const QString skyScanEndBearing =     QStringLiteral("SkyScanEndBearing");
inline static const QString skyScanNextStep =       QStringLiteral("SkyScanNextStep");
inline static const QString skyScanCountDown =      QStringLiteral("SkyScanCountDown");
inline static const QString skyScanButtonState =    QStringLiteral("SkyScanButtonState");
inline static const QString skyScanPresetNumber =   QStringLiteral("SkyScanPresetNumber");
inline static const QString skyScanReverseScan =    QStringLiteral("SkyScanReverseScan");
inline static const QString skyScanRotatorStartBearing = QStringLiteral("SkyScanRotatorStartBearing");
inline static const QString skyScanRotatorEndBearing = QStringLiteral("SkyScanRotatorEndBearing");

// Rig Control
inline static const QString rigControlApp =         QStringLiteral("mqtRigControl");
inline static const QString rigControlResult =      QStringLiteral("RigControlResult");
inline static const QString rigControlMethod =      QStringLiteral("Minos:RigControl");
inline static const QString rigControlCategory =    QStringLiteral("RigControl");
inline static const QString rigStateCategory =      QStringLiteral("RigState");
inline static const QString rigDetailsCategory =    QStringLiteral("RigDetails");
inline static const QString rigControlRadioList =   QStringLiteral("RadioList");
inline static const QString rigControlChangeList =  QStringLiteral("RigChangeList");
inline static const QString rigControlBandList =    QStringLiteral("BandList");
inline static const QString rigControlStatus =      QStringLiteral("RigStatus");
inline static const QString rigControlErrorMsg =    QStringLiteral("ErrorMsg");
inline static const QString rigControlRadioFreq =   QStringLiteral("RadioFrequency");
inline static const QString rigControlLogFreq =     QStringLiteral("LogFrequency");
inline static const QString rigControlLogBand =     QStringLiteral("LogBand");
inline static const QString rigVoiceMessageNum =    QStringLiteral("RigVoiceMessageNum");
inline static const QString rigStopVoiceMessage =   QStringLiteral("StopVoiceMessage");
inline static const QString rigCwTxMessage =        QStringLiteral("RigTxCwMessage");
inline static const QString rigControlRadioMode =   QStringLiteral("RadioMode");
inline static const QString rigControlLogMode =     QStringLiteral("LogMode");
inline static const QString rigControlSelectRadioName =     QStringLiteral("SelectRadioName");
inline static const QString rigControlTxVertEnabled =       QStringLiteral("TransVertEnabled");
inline static const QString rigControlTxVertStatus =        QStringLiteral("TransVertStatus");
inline static const QString rigControlTxVertOffsetFreq =    QStringLiteral("TransVertOffSetFreq");
inline static const QString rigControlTxVertSwitch =        QStringLiteral("TransVertSwitch");
inline static const QString rigControlRadioRitFreq =        QStringLiteral("RadioRitFrequency");
inline static const QString rigControlLogRitFreq =          QStringLiteral("LogRitFrequency");
inline static const QString rigRitOnOffStatus =     QStringLiteral("RitOnOffStatus");
inline static const QString rigRitEnableStatus =    QStringLiteral("RitEnableStatus");
inline static const QString rigRitRadioStatus =     QStringLiteral("RitRadioStatus");
inline static const QString rigRitMaxKHz =          QStringLiteral("RitMaxKHz");
inline static const QString rigRadioVolLevel =      QStringLiteral("RadioVolLevel");
inline static const QString rigLogVolLevel =        QStringLiteral("LogVolLevel");
inline static const QString rigVolStatus =          QStringLiteral("VolStatus");
inline static const QString rigPttEnabled =         QStringLiteral("PttEnabled");
inline static const QString rigPttType =            QStringLiteral("PttType");
inline static const QString rigPttState =           QStringLiteral("PttState");
inline static const QString rigPttOnOff =           QStringLiteral("PttOnOff");
inline static const QString rigRttyOffset =         QStringLiteral("RttyOffset");
inline static const QString rigPSKOffset =          QStringLiteral("PSKOffset");
inline static const QString rigVoiceMemAvail =      QStringLiteral("VoiceMemAvail");
inline static const QString rigNumberVoiceMessages =            QStringLiteral("NumberOfVoiceMessages");
inline static const QString rigCwMemType =                      QStringLiteral("CwMemType");
inline static const QString rigVoiceKeyerMessageSupportStop =   QStringLiteral("rigVoiceKeyerSupportStopCmd");
inline static const QString rigCwKeyerMessageSupportStop =      QStringLiteral("rigCwKeyerSupportStopCmd");
inline static const QString rigRigModel =           QStringLiteral("rigRigModel");

// KSTClient
inline static const QString KSTClientApp =          QStringLiteral("mqtKSTClient");
inline static const QString KSTTransfer =           QStringLiteral("KSTXfer");
inline static const QString KSTTransferCall =       QStringLiteral("KSTXferCall");
inline static const QString KSTTransferLocator =    QStringLiteral("KSTXferLoc");
inline static const QString KSTTransferFrequency =  QStringLiteral("KSTXferQRG");

// Data Modes
inline static const QString datamodesApp =          QStringLiteral("mqtDataModesApp");
inline static const QString DMCat =                 QStringLiteral("DataModes");
inline static const QString DMSender =              QStringLiteral("Sender");
inline static const QString DMWord =                QStringLiteral("DMWord");
inline static const QString DMKeyPress =            QStringLiteral("DMKeyPress");
inline static const QString DMTransmit =            QStringLiteral("DMTransmit");
inline static const QString DMStopTransmit =        QStringLiteral("DMStopTransmit");
inline static const QString DMFKeys =               QStringLiteral("DMFKeys");
inline static const QString DMFKey =                QStringLiteral("DMFKey");
inline static const QString DMMode =                QStringLiteral("DMMode");
inline static const QString DMMarkFreq =            QStringLiteral("DMMarkFreq");
}
#endif // RPCCOMMANDRCONSTANTS_H
