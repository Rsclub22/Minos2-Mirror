#ifndef RPCCOMMANDRCONSTANTS_H
#define RPCCOMMANDRCONSTANTS_H
#include <QString>

// we DON'T want these translated - so QT_TR_NOOP in not required
namespace rpcConstants
{
// PubSub

static const QString publish            ("Minos:PubSub:Publish");
static const QString serverSubscribe    ("Minos:PubSub:ServerSubscribe");
static const QString remoteSubscribe    ("Minos:PubSub:RemoteSubscribe");
static const QString subscribe          ("Minos:PubSub:Subscribe");
static const QString serverNotify       ("Minos:PubSub:ServerNotify");
static const QString clientNotify       ("Minos:PubSub:ClientNotify");

static const QString selected           ("Selected");
static const QString loggerUuid           ("logger");

static const QString paramName          ("Name");
static const QString paramValue          ("Value");

// Server Categories

static const QString StationCategory    ("Station");
static const QString LocalStationCategory    ("LocalStation");
// station name is the key, IP is the value

// Logger
static const QString loggerApp         ("MinosQtLogger");
static const QString LoggerCategory    ("MinosLogger");

// Monitor
static const QString monitorApp         ("mqtMonitor");
static const QString monitorLogCategory ("MonitorLog");
static const QString loggerStanzaRequest ("Minos:Logger:GetStanza");
static const QString loggerStanzaResponse ("Minos:Logger:StanzaReturned");



// Cluster
static const QString clusterApp         ("mqtCluster");
static const QString clusterMethod      ("Minos:Cluster");
static const QString clusterClientServer("MinosClusterClientServer");
static const QString sendClusterSpot    ("SendClusterSpot");
static const QString txSpotToCluster    ("txSpotToCluster");
static const QString hfFlagToCluster    ("hfFlagToCluster");
static const QString txSpotParamFreq    ("Freq");
static const QString txSpotParamCallsign ("Callsign");
static const QString txSpotParamLocator ("Locator");
static const QString txSpotParamUTC    ("UTC");
static const QString clusterResendSpots       ("MinosClusterResend");
static const QString clusterResendSpotsCmd    ("MinosClusterResendCmd");
static const QString clusterCategory    ("ClusterCategory");
static const QString clusterReport      ("ClusterReport");
static const QString clusterTXSpotEnableState ("ClusterTXSpotEnableState");
static const QString clusterBandmask ("ClusterBandmask");
static const QString clusterFrameId ("ClusterFrameId");
static const QString clusterReconnect ("ClusterReconnect");
static const QString clusterHfFlag ("ClusterHfFlag");

// QRZ Server
static const QString qrzServerApp  ("qrzServerApp");
static const QString qrzDisplayApp ("qrzDisplayApp");
static const QString qrzCategory  ("MinosQRZ");
static const QString qrzMethod     ("Minos:QrzServer");
static const QString qrzCluster    ("qrzCluster");
static const QString qrzLogger     ("qrzLogger");
static const QString qrzLogFrameId ("qrzLogFrameId");
static const QString qrzClusterResponse ("qrzClusterResponse");
static const QString qrzLoggerResponse ("qrzLoggerResponse");
static const QString qrzDxCallsign   ("qrzDxCallsign");
static const QString qrzSpotterCallsign ("qrzSpotterCallsign");
static const QString qrzFirstName       ("qrzFirstName");
static const QString qrzName            ("qrzName");
static const QString qrzQth            ("qrzName");
static const QString qrzCounty         ("qrzCounty");
static const QString qrzCountry         ("qrzCountry");
static const QString qrzLat             ("qrzLat");
static const QString qrzLon             ("qrzLon");
static const QString qrzDxGrid            ("qrzDxGrid");
static const QString qrzSpotterGrid     ("qrzSpotterGrid");
static const QString qrzCqZone          ("qrzCqZone");
static const QString qrzItuZone         ("qrzItuZone");
static const QString qrzDxReplyState      ("qrzDxReplyState");
static const QString qrzSpotterReplyState      ("qrzSpotterReplyState");

static const QString qrzServerLoggedOut ("qrzServerLoggedOut");
static const QString qrzServerLoggerIn  ("qrzServerLoggedIn");
static const QString qrzServerCallOK    ("qrzServerCallOK");

// Chat
static const QString chatApp            ("mqtChat");
static const QString chatMethod         ("Minos:Chat");
static const QString SendChatMessage    ("SendChatMessage");
static const QString ChatResult         ("ChatResult");
static const QString ChatCategory       ("MinosChat");
static const QString ChatServer         ("MinosChatServer");
static const QString ChatServerFrequency("MinosChatServerFrequency");

// Keyer
static const QString keyerApp           ("mqtKeyer");
static const QString keyerMethod        ("Minos:KeyerControl");
static const QString keyerPlayFile      ("PlayFile");
static const QString keyerRecordFile    ("RecordFile");
static const QString keyerTone          ("Tone");
static const QString keyerTwoTone       ("TwoTone");
static const QString keyerStop          ("Stop");

static const QString KeyerCategory      ("Keyer");
static const QString keyerReport     ("Report");

// Control
static const QString controlApp         ("mqtControl");
static const QString lineControlCategory ("LineControl");

static const QString controlParamName   ("Name");
static const QString controlParamLine   ("Line");
static const QString controlResult      ("ControlResult");
static const QString controlLineState   ("LineState");
static const QString controlState       ("CtrlState");
static const QString controlGetLine     ("GetLine");
static const QString controlSetLine     ("SetLine");
static const QString controlSetTransverter ("SetTransverter");
static const QString lineSet            ("set");



// Rotator

enum RotateDirection :int {eRotateLeft, eRotateDirect, eRotateRight, eRotateStop};

static const QString rotatorApp         ("mqtRotator");
static const QString rotatorMethod      ("Minos:Rotator");
static const QString rotatorResult      ("RotatorResult");

static const QString rotatorParamDirection    ("RotatorDirection");
static const QString rotatorParamAngle  ("RotatorAngle");

static const QString RotatorCategory    ("Rotator");
static const QString rotatorDetailCategory    ("RotatorDetail");
static const QString rotatorStateCategory    ("RotatorState");
static const QString rotatorPresetsCategory ("RotatorPresets");
static const QString rotatorStatus       ("RotStatus");
static const QString rotatorBearing     ("Bearing");
static const QString rotatorList        ("rotList");
static const QString rotPreset  ("RotPreset");
static const QString rotPresetList  ("RotPresetList");
static const QString rotCwCcwCmdEnable ("RotCwCcwCmdEnable");

static const QString rotatorSelectAntennaName ("SelectAntennaName");
static const QString rotatorMaxAzimuth  ("MaxAzimuth");
static const QString rotatorMinAzimuth  ("MinAzimuth");
static const QString supportStopCommand ("SupportStopCommand");

// Rig Control
static const QString rigControlApp         ("mqtRigControl");
static const QString rigControlResult      ("RigControlResult");
static const QString rigControlMethod      ("Minos:RigControl");
static const QString rigControlCategory    ("RigControl");
static const QString rigStateCategory      ("RigState");
static const QString rigDetailsCategory    ("RigDetails");
static const QString rigControlRadioList   ("RadioList");
static const QString rigControlBandList    ("BandList");
static const QString rigControlStatus      ("RigStatus");
static const QString rigControlErrorMsg    ("ErrorMsg");
static const QString rigControlRadioFreq   ("RadioFrequency");
static const QString rigControlLogFreq     ("LogFrequency");
static const QString rigControlLogBand     ("LogBand");
static const QString rigVoiceMessageNum    ("RigVoiceMessageNum");
static const QString rigControlRadioMode   ("RadioMode");
static const QString rigControlLogMode     ("LogMode");
static const QString rigControlSelectRadioName   ("SelectRadioName");
static const QString rigControlTxVertEnabled ("TransVertEnabled");
static const QString rigControlTxVertStatus ("TransVertStatus");
static const QString rigControlTxVertOffsetFreq ("TransVertOffSetFreq");
static const QString rigControlTxVertSwitch ("TransVertSwitch");
static const QString rigControlRadioRitFreq      ("RadioRitFrequency");
static const QString rigControlLogRitFreq      ("LogRitFrequency");
static const QString rigRitOnOffStatus      ("RitOnOffStatus");
static const QString rigRitEnableStatus     ("RitEnableStatus");
static const QString rigRitRadioStatus     ("RitRadioStatus");
static const QString rigRitMaxKHz ("RitMaxKHz");
static const QString rigRadioVolLevel      ("RadioVolLevel");
static const QString rigLogVolLevel      ("LogVolLevel");
static const QString rigVolStatus           ("VolStatus");
static const QString rigHfFlag           ("HfFlag");
static const QString rigPttEnabled ("PttEnabled");
static const QString rigPttType ("PttType");
static const QString rigPttState  ("PttState");
static const QString rigVoiceMemAvail ("VoiceMemAvail");
static const QString rigCwMemAvail ("CwMemAvail");


// KSTClient
static const QString KSTClientApp         ("mqtKSTClient");

}
#endif // RPCCOMMANDRCONSTANTS_H
