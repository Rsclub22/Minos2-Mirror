#ifndef RIGCONTROLCOMMONCONSTANTS_H
#define RIGCONTROLCOMMONCONSTANTS_H


#include <QString>
#include <QMap>
#include <QMultiMap>

#include <QStringList>
#include <QSettings>
#include "frequency.h"



// Status messages sent to minos logger
extern const char * RIG_STATUS_CONNECTED;
extern const char * RIG_STATUS_DISCONNECTED;
extern const char * RIG_STATUS_ERROR;
extern const char * RIG_SWITCH_COMPLETED;

//#define TXVERT_ON "TXVERTON"
//#define TXVERT_OFF "TXVERTOFF"

class BandInfo;


namespace radioData {

}


namespace hamlibData
{

    // Hamlib Modes

    const QString AM = "AM";             // Amplitude Modulation
    const QString CW =  "CW";            //  normal sideband
    const QString USB = "USB";           // Upper Side Band
    const QString LSB = "LSB";           // Lower Side Band
    const QString RTTY = "RTTY";         //  Radio Teletype
    const QString FM = "FM";             // "narrow" band FM
    const QString WFM = "WFM";           // broadcast wide FM
    const QString CWR = "CW";            //  "reverse" sideband
    const QString RTTYR = "RTTYR";       // RTTY "reverse" sideband
    const QString AMS = "AMS";           // Amplitude Modulation Synchronous
    const QString PKTLSB = "PKTLSB";     // Packet/Digital LSB mode (dedicated port)
    const QString PKTUSB = "PKTUSB";     // Packet/Digital USB mode (dedicated port)
    const QString PKTFM = "PKTFM";       // Packet/Digital FM mode (dedicated port)
    const QString ECSSUSB = "ECSSUSB";   // Exalted Carrier Single Sideband USB
    const QString ECSSLSB = "ECSSLSB";   // Exalted Carrier Single Sideband LSB
    const QString FAX = "FAX";           // Facsimile Mode
    const QString SAM = "SAM";           // Synchronous AM double sideband
    const QString SAL = "SAL";           // Synchronous AM lower sideband
    const QString SAH = "SAH";           // Synchronous AM upper (higher) sideband
    const QString DSB = "DSB";           // Double sideband suppressed carrier
    const QString FMN = "FMN";           // FM Narrow Kenwood ts990s

    const QString MGM = "MGM";           // MGM generically




    const QStringList  modeList = {AM, CW, USB, LSB, RTTY, FM, WFM, CWR, RTTYR, AMS,
                              PKTLSB, PKTUSB, PKTFM, ECSSUSB, ECSSLSB, FAX, SAM,
                              SAL, SAH, DSB, FMN };


    const QStringList supModeList = { CW, USB, FM, MGM};

    //enum pBandState {NAR, NOR, WIDE};

    //const QStringList pBandStateStr = {"NAR", "NOR", "WIDE"};


    const QStringList portTypeList = { "RIG_PORT_NONE", "RIG_PORT_SERIAL", "RIG_PORT_NETWORK", "RIG_PORT_DEVICE",
                                    "RIG_PORT_PACKET", "RIG_PORT_DTMF", "RIG_PORT_ULTRA", "RIG_PORT_RPC",
                                    "RIG_PORT_PARALLEL", "RIG_PORT_USB", "RIG_PORT_UDP_NETWORK", "RIG_PORT_CM108"};

    const int RIGCTL = 2;               // rigctl model number


}

const int Rig_OK = 0;
const int OPEN_OK = 0;
const int OPEN_FAILED = -1;
const int RIGCTLD_FAILED = -2;
const int RIGCTLD_FAILED_TO_STOP = -3;
const int RIGCTLD_EXE_MISSING = -4;

const int NO_TRANSVERTER_NUM = -1;

const QStringList radioOpenMessages = {"Open OK", "Open Failed", "RigCtld Failed",
                                      "RigCtld Failed to Stop", "RigCtld Exe Missing"};

const Frequency NO_BAND_SUPPORT;

const QString TRANSSW_NUM_DEFAULT = "0";


// File Name Constants

const QString DEFAULT_BIN_PATH = "./Bin/";
const QString DEFAULT_WIN32_RIGCTLD_PATH = "./Bin/";
const QString DEFAULT_LINUX_RIGCTLD_PATH = "./Bin/";
const QString DEFAULT_MAC_RIGCTLD_PATH = "./Bin/";
//const QString DEFAULT_LINUX_RIGCTLD_PATH = "/usr/bin/";
//const QString DEFAULT_MAC_RIGCTLD_PATH = "/usr/bin/";

const QString LOCAL_RADIO = "Local";

const QString RIG_CONFIGURATION_FILEPATH_LOGGER = "./Configuration/";
const QString RIG_CONFIGURATION_FILEPATH_LOCAL = "./Configuration/";

const QString MINOS_RADIO_CONFIG_FILE = "MinosRigControlConfig.ini";
const QString MINOS_RIGSYNC_CONFIG_FILE = "MinosRigSyncConfig.ini";


const QString RADIO_PATH_LOGGER = "./Configuration/Radio/";
const QString RADIO_PATH_LOCAL = "./Configuration/Radio/";

const QString TRANSVERT_PATH_LOGGER = "./Configuration/Radio/Transverter/";
const QString TRANSVERT_PATH_LOCAL = "./Configuration/Radio/Transverter/";

const QString FILENAME_AVAIL_RADIOS = "AvailRadio.ini";
const QString FILENAME_CURRENT_RADIO = "CurrentRadio.ini";
const QString FILENAME_TRANSVERT_RADIOS = "TransVertRadio.ini";

const QString FILENAME_FREQ_PRESETS = "FreqPresets.ini";

const QString RIGCTL_WIN32_EXE_FILENAME = "rigctld.exe";
const QString RIGCTL_LINUX_EXE_FILENAME = "rigctld";
const QString RIGCTL_MAC_EXE_FILENAME = "rigctld";

const QString RIGCTLD_GROUP_NAME = "Rigctld";
const QString RIGCTLD_PATH_SETTING_NAME = "RigctldPath";

const QString DEFAULT_FT817_RADIO_RETRY = "3";
const QString DEFAULT_FT817_RADIO_TIMEOUT = "1000";

const QString BANDSWITCH_INI_FILENAME = "./Configuration/BandSwitchData.ini";
const QString BANDSWITCH_KEY_TEXT = "_bandSwData";
const QString BANDSWITCH_ENABLE_KEY_TEXT = "bandSwEnable";
const QString BANDSWITCH_SERIAL_ENABLE_KEY_TEXT = "bandSwSerialEnable";
const QString BANDSWITCH_COMPORT_KEY_TEXT = "comport";

// RadioName when no radio is connected
//const QString NORADIO = "NoRadio";


namespace freqPresetData {




//enum bandOffSet { _1_8MHZ, _3_5MHZ, _7MHZ, _14MHZ, _21MHZ, _28MHZ,  _50MHZ, _70MHZ, _144MHZ, _432MHZ, _1296MHZ, _2300MHZ,
//                 _3_4GHZ, _5_6GHZ, _10GHZ};

//const QStringList bandNames = QStringList() << "1.8 MHz" << "3.5 MHz" << "7 MHz" << "14 MHz" << "21 MHz" << "28 MHz"
//                                           << "50 MHz" << "70 MHz" << "144 MHz" << "432 MHz"
//                                           << "1296 MHz" << "2300 MHz" << "3.4 GHz" << "5.6 GHz" << "10 GHz";

const QStringList presetBands = QStringList() << "50 MHz" << "70 MHz" << "144 MHz" << "432 MHz" << "1296 MHz" << "2300 MHz" << "3.4 GHz" << "5.6 GHz" << "10 GHz";

const QStringList bandFreq = {  "1800000",
                                "3500000",
                                "7000000",
                               "14000000",
                               "21000000",
                               "28000000",
                               "50170000",
                               "70210000",
                              "144290000",
                              "432170000",
                             "1296100000",
                             "2300100000",            // 2300 GHz
                             "3400100000",  // 3.4 GHz
                             "5760100000",  // 5.6 GHz
                            "10368100000",  // 10 GHz

                              };


const QString PRESET_MODE_CW = "CW";
const QString PRESET_MODE_PHONE = "PHONE";
const QString PRESET_MODE_MGM = "MGM";

}

class StoredPresetFreqs
{


public:

    StoredPresetFreqs()
    {

    };

    Frequency presetFreq;
    Frequency lastFreq;
};

class PresetFreq
{

public:
    PresetFreq();

    ~PresetFreq();


/*
    PresetFreq operator= (const PresetFreq& presets)
    {
        QMap<QString, StoredPresetFreqs> mspf;
        QMap<QString, StoredPresetFreqs>*  mspfptr = &mspf;

        QMapIterator<PresetFreq> i(presets);

    }
*/

    void clear();

    Frequency getPresetFreq(const QString mode, const QString band);
    void setPresetFreq(const QString mode, const QString band, const QString freq);
    void setPresetFreq(const QString mode, const QString band, const Frequency freq);

    Frequency getLastFreq(const QString mode, const QString band);
    void setLastFreq(const QString mode, const QString band, const QString freq);
    void setLastFreq(const QString mode, const QString band, const Frequency freq);
    bool contains(QString mode, QString band);

    void copyAllPrevFreqToLastFreqByMode(const QString mode, const QVector<QSharedPointer<BandInfo> > &bands);
    void readSettings(const QVector<QSharedPointer<BandInfo> > &bands);

private:

    QMap<QString, QMap<QString, StoredPresetFreqs>* > modePresetFreqList;
    QMap<QString, StoredPresetFreqs> cwFreqPresets;

    QMap<QString, StoredPresetFreqs> phoneFreqPresets;

    QMap<QString, StoredPresetFreqs> mgmFreqPresets;




};

// TransVerter Switch Message
const char TVSWMSG_START = ':';
const char TVSWMSG_TERM = '\n';

// these two values should be the same
// max slider, matches max vol
const float VOLMULT = 100.0;
const int SLIDERMAX = 100;


// S-Meter Constants
const int SMETERVALUES[17][2] = {
                                    {60, 61},
                                    {50, 60},
                                    {40, 50},
                                    {30, 40},
                                    {20, 30},
                                    {10, 20},
                                    {0, 10},
                                    {-6, 0},
                                    {-12, -6},
                                    {-18, -12},
                                    {-22, -18},
                                    {-28, -22},
                                    {-32, -28},
                                    {-38, -32},
                                    {-42, -38},
                                    {-48, -42},
                                    {-54, -48},

                                };

const int MAX_SMETER_LEVEL = 61;
const int MIN_SMETER_LEVEL = -54;

const QStringList SMETERTEXT = {
                                "S9+60",
                                "S9+50",
                                "S9+40",
                                "S9+30",
                                "S9+20",
                                "S9+10",
                                "S9   ",
                                "S8   ",
                                "S7   ",
                                "S6   ",
                                "S5   ",
                                "S4   ",
                                "S3   ",
                                "S2   ",
                                "S1   ",
                                };


const QString RIG_DEFAULT_POLLINTERVAL = "1";
const QString ROT_DEFAULT_POLLINTERVAL = "1";

const QString SUP_RADIO_INDICATOR_OFF_STYLE = QString("background-color: white;\n");
const QString SUP_RADIO_INDICATOR_RADIO_STYLE = QString("background-color: blue;\n");
const QString SUP_RADIO_INDICATOR_TRANSVERT_STYLE = QString("background-color: yellow;\n");
const QString SUP_RADIO_INDICATOR_TRANSVERT_ON_STYLE = QString("background-color: orange;\n");
const QString RIT_STATUS_OFF_STYLE = QString("background-color:  white;\n");
const QString RIT_STATUS_ON_STYLE = QString("background-color:  orange;\n");
const QString RIT_RADIO_GETSETFREQ_INDICATOR_FALSE = QString("background-color: white;\n");
const QString RIT_RADIO_GETSETFREQ_INDICATOR_TRUE = QString("background-color: blue ;\n");
const QString VOICEMEM_INDICATOR_ON = QString("background-color: blue ;\n");
const QString VOICEMEM_INDICATOR_OFF = QString("background-color: white ;\n");
const QString CWMEM_INDICATOR_ON = QString("background-color: blue ;\n");
const QString CWMEM_INDICATOR_OFF = QString("background-color: white ;\n");
const QString PTT_INDICATOR_ON = QString("background-color: blue ;\n");
const QString PTT_INDICATOR_OFF = QString("background-color: white ;\n");
const QString TX_RX_INDICATOR_ON = QString("background-color: orange; \n");
const QString TX_RX_INDICATOR_OFF = QString("background-color: white; \n");


enum RIGCTLD_INDICATOR_ID  {RIGCTLD_IND_EXT, RIGCTLD_IND_INT, RIGCTLD_IND_OFF};

const QString RIGCTLD_INDICATOR_INTERNAL = QString("background-color: orange;\n");
const QString RIGCTLD_INDICATOR_EXTERNAL = QString("background-color: blue;\n");
const QString RIGCTLD_INDICATOR_OFF = QString("background-color: white;\n");

const int RIGCTLD_STATUS_TIMER_DUR = 1000;
const int RIGCTLD_PROCESS_TIMEOUT = 5;

const QString RIGCTLD_LOCAL_HOST_ADDRESS = "127.0.0.1";
const QString RIGCTLD_DEFAULT_PORT_ADDRESS = "4532";
const int DEFAULT_RIGCTLD_CONNECT_DELAY = 2;
const int MAX_RIGCTLD_CONNECT_DELAY = 5;

const int CHECK_CONTEST_FREQ_MATCH_TIMEOUT = 2000;


QString readBandSwitchDataFromIni(QString band);
void writeBandSwitchDataToIni(QString band, QString data);
bool readEnableBandSwitchFromIni();
void writeEnableBandSwitchDataToIni(bool data);
bool readEnableSerialBandSwitchFromIni();
void writeEnableSerialBandSwitchDataToIni(bool data);
QString readSerialComportBandSwitchFromIni();
void writeSerialComportBandSwitchDataToIni(QString comport);
QString convertBandForIni(QString band);

class RadioSettingsDialogChangeFlag
{
public:
    RadioSettingsDialogChangeFlag()
    {
        clear();
    }

    void clear()
    {
        operatingFreqColor = false;
        ignorePresetFreq = false;
        ignorePreviousFreq = false;
        restoreContestMode = false;
        cwPresetsChanged = false;
        phonePresetsChanged = false;
        mgmPresetsChanged = false;
        enableBandSwitch = false;
        enableSerialBandSwitch = false;
        serialComport = false;
    }

    bool isChanged()
    {
        return operatingFreqColor ||
        ignorePresetFreq ||
        ignorePreviousFreq ||
        restoreContestMode ||
        cwPresetsChanged ||
        phonePresetsChanged ||
        mgmPresetsChanged ||
        enableBandSwitch ||
        enableSerialBandSwitch ||
        serialComport;
    }


    bool operatingFreqColor;
    bool ignorePresetFreq;
    bool ignorePreviousFreq;
    bool restoreContestMode;
    bool cwPresetsChanged;
    bool phonePresetsChanged;
    bool mgmPresetsChanged;
    bool enableBandSwitch;
    bool enableSerialBandSwitch;
    bool serialComport;




};

#endif // RIGCONTROLCOMMONCONSTANTS_H
