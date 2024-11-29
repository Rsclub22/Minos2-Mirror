#ifndef RIGCONTROLCOMMONCONSTANTS_H
#define RIGCONTROLCOMMONCONSTANTS_H


#include <QString>
#include <QMap>
#include <QMultiMap>

#include <QStringList>
#include <QSettings>
#include "minositem.h"
#include "frequency.h"

// Offset frequencies

const int RTTY_MARK_OFFSET = 2125;
const int BPSK_OFFSET = 1750;

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

    inline const QString AM = "AM";             // Amplitude Modulation
    inline const QString CW =  "CW";            //  normal sideband
    inline const QString USB = "USB";           // Upper Side Band
    inline const QString LSB = "LSB";           // Lower Side Band
    inline const QString RTTY = "RTTY";         //  Radio Teletype
    inline const QString FM = "FM";             // "narrow" band FM
    inline const QString WFM = "WFM";           // broadcast wide FM
    inline const QString CWR = "CW";            //  "reverse" sideband
    inline const QString RTTYR = "RTTYR";       // RTTY "reverse" sideband
    inline const QString AMS = "AMS";           // Amplitude Modulation Synchronous
    inline const QString PKTLSB = "PKTLSB";     // Packet/Digital LSB mode (dedicated port)
    inline const QString PKTUSB = "PKTUSB";     // Packet/Digital USB mode (dedicated port)
    inline const QString PKTFM = "PKTFM";       // Packet/Digital FM mode (dedicated port)
    inline const QString ECSSUSB = "ECSSUSB";   // Exalted Carrier Single Sideband USB
    inline const QString ECSSLSB = "ECSSLSB";   // Exalted Carrier Single Sideband LSB
    inline const QString FAX = "FAX";           // Facsimile Mode
    inline const QString SAM = "SAM";           // Synchronous AM double sideband
    inline const QString SAL = "SAL";           // Synchronous AM lower sideband
    inline const QString SAH = "SAH";           // Synchronous AM upper (higher) sideband
    inline const QString DSB = "DSB";           // Double sideband suppressed carrier
    inline const QString FMN = "FMN";           // FM Narrow Kenwood ts990s

    inline const QString MGM = "MGM";           // MGM generically

    inline const QString RY = "RY";
    inline const QString PSK = "PS";
    inline const QString PH = "PH";


    inline const QStringList portTypeList = { "RIG_PORT_NONE", "RIG_PORT_SERIAL", "RIG_PORT_NETWORK", "RIG_PORT_DEVICE",
                                    "RIG_PORT_PACKET", "RIG_PORT_DTMF", "RIG_PORT_ULTRA", "RIG_PORT_RPC",
                                    "RIG_PORT_PARALLEL", "RIG_PORT_USB", "RIG_PORT_UDP_NETWORK", "RIG_PORT_CM108"};

    const int RIGCTL = 2;               // rigctl model number

    enum CW_MEMORY_TYPES {NONE, ICOM, YAESU, KENWOOD, ELECRAFT, FLEX_RADIO,
                            OPENHPSDR, FLEX_RADIO_APACHE, QRPLABS, THETIS };
}

const int Rig_OK = 0;
const int OPEN_OK = 0;
const int OPEN_FAILED = -1;
const int RIGCTLD_FAILED = -2;
const int RIGCTLD_FAILED_TO_STOP = -3;
const int RIGCTLD_EXE_MISSING = -4;

const int NO_TRANSVERTER_NUM = -1;

inline const QStringList radioOpenMessages = {"Open OK", "Open Failed", "RigCtld Failed",
                                      "RigCtld Failed to Stop", "RigCtld Exe Missing"};

inline const Frequency NO_BAND_SUPPORT;

inline const QString TRANSSW_NUM_DEFAULT = "0";



// File Name Constants

QString DEFAULT_RIGCTLD_PATH();

inline const QString LOCAL_RADIO = "Local";

QString RIG_CONFIGURATION_FILEPATH_LOGGER();
QString RIG_CONFIGURATION_FILEPATH_LOCAL();

inline const QString MINOS_RADIO_CONFIG_FILE = "MinosRigControlConfig.ini";
inline const QString MINOS_RIGSYNC_CONFIG_FILE = "MinosRigSyncConfig.ini";


QString RADIO_PATH_LOGGER();
QString RADIO_PATH_LOCAL();

QString TRANSVERT_PATH_LOGGER();
QString TRANSVERT_PATH_LOCAL();

inline const QString FILENAME_AVAIL_RADIOS = "AvailRadio.ini";
inline const QString FILENAME_CURRENT_RADIO = "CurrentRadio.ini";
inline const QString FILENAME_TRANSVERT_RADIOS = "TransVertRadio.ini";
inline const QString FILENAME_RIGCONTROL_TEST_DATA = "rigcontrolTestData.ini";

inline const QString FILENAME_MINOS_OMNIRIG = "minosOmnirig.ini";

inline const QString FILENAME_FREQ_PRESETS = "FreqPresets.ini";

inline const QString RIGCTL_WIN32_EXE_FILENAME = "rigctld.exe";
inline const QString RIGCTL_LINUX_EXE_FILENAME = "rigctld";
inline const QString RIGCTL_MAC_EXE_FILENAME = "rigctld";

inline const QString RIGCTLD_GROUP_NAME = "Rigctld";
inline const QString RIGCTLD_PATH_SETTING_NAME = "RigctldPath";
inline const QString RIGCTLD_NAME_SETTING_NAME = "RigctldName";

inline const QString DEFAULT_FT817_RADIO_RETRY = "3";
inline const QString DEFAULT_FT817_RADIO_TIMEOUT = "1000";

QString BANDSWITCH_INI_FILENAME();
inline const QString BANDSWITCH_KEY_TEXT = "_bandSwData";
inline const QString BANDSWITCH_ENABLE_KEY_TEXT = "bandSwEnable";
inline const QString BANDSWITCH_SERIAL_ENABLE_KEY_TEXT = "bandSwSerialEnable";
inline const QString BANDSWITCH_COMPORT_KEY_TEXT = "comport";

namespace freqPresetData {

inline const QString PRESET_MODE_CW = "CW";
inline const QString PRESET_MODE_PHONE = "PHONE";
inline const QString PRESET_MODE_MGM = "MGM";
inline const QString PRESET_MODE_RTTY = "RY";
inline const QString PRESET_MODE_PSK = "PS";

}
class StoredPresetFreqs
{


public:

    StoredPresetFreqs()
    {

    };

    MinosItem<Frequency> presetFreq;
    Frequency lastFreq;
};

class PresetFreq
{

public:
    PresetFreq();

    ~PresetFreq();

    void clear();

    bool isDirty(const QString mode, const QString band);
    void clearDirty();

    Frequency getPresetFreq(const QString mode, const QString band);
    void setPresetFreq(const QString mode, const QString band, const QString freq);

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
    QMap<QString, StoredPresetFreqs> rttyFreqPresets;
    QMap<QString, StoredPresetFreqs> pskFreqPresets;

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

inline const QStringList SMETERTEXT = {
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


inline const QString RIG_DEFAULT_POLLINTERVAL = "1";
inline const QString ROT_DEFAULT_POLLINTERVAL = "1";

inline const QString SUP_RADIO_INDICATOR_OFF_STYLE = QString("background-color: white;\n");
inline const QString SUP_RADIO_INDICATOR_RADIO_STYLE = QString("background-color: blue;\n");
inline const QString SUP_RADIO_INDICATOR_TRANSVERT_STYLE = QString("background-color: yellow;\n");
inline const QString SUP_RADIO_INDICATOR_TRANSVERT_ON_STYLE = QString("background-color: orange;\n");
inline const QString RIT_STATUS_OFF_STYLE = QString("background-color:  white;\n");
inline const QString RIT_STATUS_ON_STYLE = QString("background-color:  orange;\n");
inline const QString RIT_RADIO_GETSETFREQ_INDICATOR_FALSE = QString("background-color: white;\n");
inline const QString RIT_RADIO_GETSETFREQ_INDICATOR_TRUE = QString("background-color: blue ;\n");
inline const QString VOICEMEM_INDICATOR_ON = QString("background-color: blue ;\n");
inline const QString VOICEMEM_INDICATOR_OFF = QString("background-color: white ;\n");
inline const QString CWMEM_INDICATOR_ON = QString("background-color: blue ;\n");
inline const QString CWMEM_INDICATOR_OFF = QString("background-color: white ;\n");
inline const QString PTT_INDICATOR_ON = QString("background-color: blue ;\n");
inline const QString PTT_INDICATOR_OFF = QString("background-color: white ;\n");
inline const QString TX_RX_INDICATOR_ON = QString("background-color: orange; \n");
inline const QString TX_RX_INDICATOR_OFF = QString("background-color: white; \n");

inline const QString BACKGROUND_GREY = QString("#f0f0f0");

enum RIGCTLD_INDICATOR_ID  {RIGCTLD_IND_EXT, RIGCTLD_IND_INT, RIGCTLD_IND_OFF};

inline const QString RIGCTLD_INDICATOR_INTERNAL = QString("background-color: orange;\n");
inline const QString RIGCTLD_INDICATOR_EXTERNAL = QString("background-color: blue;\n");
inline const QString RIGCTLD_INDICATOR_OFF = QString("background-color: white;\n");

inline const int RIGCTLD_STATUS_TIMER_DUR = 1000;
inline const int RIGCTLD_PROCESS_TIMEOUT = 5;

inline const QString RIGCTLD_LOCAL_HOST_ADDRESS = "127.0.0.1";
inline const QString RIGCTLD_DEFAULT_PORT_ADDRESS = "4532";
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
        radioReadOnly = false;
        ignorePresetFreq = false;
        ignorePreviousFreq = false;
        restoreContestMode = false;
        cwPresetsChanged = false;
        phonePresetsChanged = false;
        mgmPresetsChanged = false;
        rttyPresetsChanged = false;
        mgmPresetsChanged = false;
        enableBandSwitch = false;
        enableSerialBandSwitch = false;
        serialComport = false;
        saveVoiceMemoryButtonByRadioName = false;
        cqRit = false;
    }

    bool isChanged()
    {
        return operatingFreqColor ||
        radioReadOnly ||
        ignorePresetFreq ||
        ignorePreviousFreq ||
        restoreContestMode ||
        cwPresetsChanged ||
        phonePresetsChanged ||
        mgmPresetsChanged ||
        rttyPresetsChanged ||
        pskPresetsChanged ||
        enableBandSwitch ||
        enableSerialBandSwitch ||
        serialComport ||
        saveVoiceMemoryButtonByRadioName ||
        cqRit;
    }


    bool operatingFreqColor;
    bool radioReadOnly;
    bool ignorePresetFreq;
    bool ignorePreviousFreq;
    bool restoreContestMode;
    bool cwPresetsChanged;
    bool phonePresetsChanged;
    bool mgmPresetsChanged;
    bool rttyPresetsChanged;
    bool pskPresetsChanged;
    bool enableBandSwitch;
    bool enableSerialBandSwitch;
    bool serialComport;
    bool saveVoiceMemoryButtonByRadioName;
    bool cqRit;




};

#endif // RIGCONTROLCOMMONCONSTANTS_H
