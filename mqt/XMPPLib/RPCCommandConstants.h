#ifndef RPCCOMMANDRCONSTANTS_H
#define RPCCOMMANDRCONSTANTS_H
#include <QString>

namespace rpcConstants
{
// PubSub

static const QString publish            = QT_TR_NOOP("Minos:PubSub:Publish");
static const QString serverSubscribe    = QT_TR_NOOP("Minos:PubSub:ServerSubscribe");
static const QString remoteSubscribe    = QT_TR_NOOP("Minos:PubSub:RemoteSubscribe");
static const QString subscribe          = QT_TR_NOOP("Minos:PubSub:Subscribe");
static const QString serverNotify       = QT_TR_NOOP("Minos:PubSub:ServerNotify");
static const QString clientNotify       = QT_TR_NOOP("Minos:PubSub:ClientNotify");

static const QString selected           = QT_TR_NOOP("Selected");
static const QString loggerUuid           = QT_TR_NOOP("logger");

static const QString paramName          = QT_TR_NOOP("Name");
static const QString paramValue          = QT_TR_NOOP("Value");

// Server Categories

static const QString StationCategory    = QT_TR_NOOP("Station");
static const QString LocalStationCategory    = QT_TR_NOOP("LocalStation");
// station name is the key, IP is the value

// Logger
static const QString loggerApp         = QT_TR_NOOP("MinosQtLogger");
static const QString LoggerCategory    = QT_TR_NOOP("MinosLogger");

// Monitor
static const QString monitorApp         = QT_TR_NOOP("mqtMonitor");
static const QString monitorLogCategory = QT_TR_NOOP("MonitorLog");
static const QString loggerStanzaRequest = QT_TR_NOOP("Minos:Logger:GetStanza");
static const QString loggerStanzaResponse = QT_TR_NOOP("Minos:Logger:StanzaReturned");

// Bandmap
static const QString bandmapApp         = QT_TR_NOOP("mqtBandMap");
static const QString bandmapMethod      = QT_TR_NOOP("Minos:BandMap");

static const QString bandmapParamFreq   = QT_TR_NOOP("Freq");
static const QString bandmapParamCallsign = QT_TR_NOOP("Callsign");
static const QString bandmapParamLocator= QT_TR_NOOP("Locator");
static const QString bandmapParamUTC    = QT_TR_NOOP("UTC");
static const QString bandmapParamQTH    = QT_TR_NOOP("QTH");

static const QString BandMapCategory    = QT_TR_NOOP("BandMap");
static const QString bandmapKeyLoaded    = QT_TR_NOOP("Loaded");


// Cluster
static const QString clusterApp         = QT_TR_NOOP("mqtCluster");
static const QString clusterMethod      = QT_TR_NOOP("Minos:Cluster");
static const QString clusterClientServer= QT_TR_NOOP("MinosClusterClientServer");
static const QString sendClusterSpot    = QT_TR_NOOP("SendClusterSpot");


// Chat
static const QString chatApp            = QT_TR_NOOP("mqtChat");
static const QString chatMethod         = QT_TR_NOOP("Minos:Chat");
static const QString SendChatMessage    = QT_TR_NOOP("SendChatMessage");
static const QString ChatResult         = QT_TR_NOOP("ChatResult");
static const QString ChatCategory       = QT_TR_NOOP("MinosChat");
static const QString ChatServer         = QT_TR_NOOP("MinosChatServer");
static const QString ChatServerFrequency= QT_TR_NOOP("MinosChatServerFrequency");

// Keyer
static const QString keyerApp           = QT_TR_NOOP("mqtKeyer");
static const QString keyerMethod        = QT_TR_NOOP("Minos:KeyerControl");
static const QString keyerPlayFile      = QT_TR_NOOP("PlayFile");
static const QString keyerRecordFile    = QT_TR_NOOP("RecordFile");
static const QString keyerTone          = QT_TR_NOOP("Tone");
static const QString keyerTwoTone       = QT_TR_NOOP("TwoTone");
static const QString keyerStop          = QT_TR_NOOP("Stop");

static const QString KeyerCategory      = QT_TR_NOOP("Keyer");
static const QString keyerReport     = QT_TR_NOOP("Report");

// Control
static const QString controlApp         = QT_TR_NOOP("mqtControl");
static const QString lineControlCategory = QT_TR_NOOP("LineControl");

static const QString controlParamName   = QT_TR_NOOP("Name");
static const QString controlParamLine   = QT_TR_NOOP("Line");
static const QString controlResult      = QT_TR_NOOP("ControlResult");
static const QString controlLineState   = QT_TR_NOOP("LineState");
static const QString controlState       = QT_TR_NOOP("CtrlState");
static const QString controlGetLine     = QT_TR_NOOP("GetLine");
static const QString controlSetLine     = QT_TR_NOOP("SetLine");
static const QString controlSetTransverter = QT_TR_NOOP("SetTransverter");
static const QString lineSet            = QT_TR_NOOP("set");



// Rotator

enum RotateDirection :int {eRotateLeft, eRotateDirect, eRotateRight, eRotateStop};

static const QString rotatorApp         = QT_TR_NOOP("mqtRotator");
static const QString rotatorMethod      = QT_TR_NOOP("Minos:Rotator");
static const QString rotatorResult      = QT_TR_NOOP("RotatorResult");

static const QString rotatorParamDirection    = QT_TR_NOOP("RotatorDirection");
static const QString rotatorParamAngle  = QT_TR_NOOP("RotatorAngle");

static const QString RotatorCategory    = QT_TR_NOOP("Rotator");
static const QString rotatorDetailCategory    = QT_TR_NOOP("RotatorDetail");
static const QString rotatorStateCategory    = QT_TR_NOOP("RotatorState");
static const QString rotatorPresetsCategory = QT_TR_NOOP("RotatorPresets");
static const QString rotatorStatus       = QT_TR_NOOP("RotStatus");
static const QString rotatorBearing     = QT_TR_NOOP("Bearing");
static const QString rotatorList        = QT_TR_NOOP("rotList");
static const QString rotPreset  = QT_TR_NOOP("RotPreset");
static const QString rotPresetList  = QT_TR_NOOP("RotPresetList");
static const QString rotCwCcwCmdEnable = QT_TR_NOOP("RotCwCcwCmdEnable");

static const QString rotatorSelectAntennaName = QT_TR_NOOP("SelectAntennaName");
static const QString rotatorMaxAzimuth  = QT_TR_NOOP("MaxAzimuth");
static const QString rotatorMinAzimuth  = QT_TR_NOOP("MinAzimuth");

// Rig Control
static const QString rigControlApp         = QT_TR_NOOP("mqtRigControl");
static const QString rigControlResult      = QT_TR_NOOP("RigControlResult");
static const QString rigControlMethod      = QT_TR_NOOP("Minos:RigControl");
static const QString rigControlCategory    = QT_TR_NOOP("RigControl");
static const QString rigStateCategory      = QT_TR_NOOP("RigState");
static const QString rigDetailsCategory    = QT_TR_NOOP("RigDetails");
static const QString rigControlRadioList   = QT_TR_NOOP("RadioList");
static const QString rigControlBandList    = QT_TR_NOOP("BandList");
static const QString rigControlStatus      = QT_TR_NOOP("RigStatus");
static const QString rigControlErrorMsg    = QT_TR_NOOP("ErrorMsg");
static const QString rigControlFreq        = QT_TR_NOOP("Frequency");
static const QString rigControlMode        = QT_TR_NOOP("Mode");
static const QString rigControlSelectRadioName   = QT_TR_NOOP("SelectRadioName");
static const QString rigControlTxVertStatus = QT_TR_NOOP("TransVertStatus");
static const QString rigControlTxVertOffsetFreq = QT_TR_NOOP("TransVertOffSetFreq");
static const QString rigControlTxVertSwitch = QT_TR_NOOP("TransVertSwitch");
static const QString rigControlRitFreq      = QT_TR_NOOP("RitFrequency");
static const QString rigRitOnOffStatus      = QT_TR_NOOP("RitOnOffStatus");
static const QString rigRitEnableStatus     = QT_TR_NOOP("RitEnableStatus");
static const QString rigRitRadioStatus     = QT_TR_NOOP("RitRadioStatus");
static const QString rigVolLevel            = QT_TR_NOOP("VolLevel");
static const QString rigVolStatus           = QT_TR_NOOP("VolStatus");
static const QString rigTpm                 = QT_TR_NOOP("Tpm");
}
#endif // RPCCOMMANDRCONSTANTS_H
