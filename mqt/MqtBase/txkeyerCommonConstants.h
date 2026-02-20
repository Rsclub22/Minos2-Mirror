#ifndef TXKEYERCOMMONCONSTANTS_H
#define TXKEYERCOMMONCONSTANTS_H



#include <QString>
#include <QStringList>
#include <QValidator>







namespace TxKeyerCommon
{
    // These are Minos max and min keyer buttons
    // Rigcontrol with hamlib will provide max number of voice memories the
        // radio will support.

    const int PC_CW_KEYER_MAXIMUM_BUTTONS = 12;
    const int PC_CW_KEYER_MIN_WPM = 5;
    const int PC_CW_KEYER_MAX_WPM = 40;
    const int MAXIMUM_BUTTONS = 8;
    const int MININUM_BUTTONS = 2;
/*
    const char * TXKEYER_BUTTON_ON_STYLE =
        "background-color: orange;"
        "padding: 4px;";

    const char * TXKEYER_BUTTON_OFF_STYLE =
        "background-color: Gainsboro;"
        "padding: 4px;";
*/
    const int NO_TXKEYER_BUTTON_SELECTED = -1;

    // used in DMButtonFrame edit table
    const int EDIT_DLG_COL0 = 0;
    const int EDIT_DLG_COL1 = 1;
    const int EDIT_DLG_COL2 = 2;
    const int EDIT_DLG_COL3 = 3;
    const int EDIT_DLG_COL4 = 4;
    const int EDIT_DLG_COL5 = 5;
    const int EDIT_DLG_COL6 = 6;
    const int EDIT_DLG_COL7 = 7;


    enum class TxKeyerId
    {
        None = 0,
        RigControl,
        CW_RigControl,    // Rig sends message by CW
        SerialControl,    // Sends user serial message to control external voice keyer
        PcCwKeyer,        // Serial DTR keying of radio
        DigitalModes,
        InternalVoiceKeyer,
        ExternalMqtKeyer,
        WinKeyer

    };




    QString getTxKeyerDisplayName(TxKeyerId id)
    {
        switch (id)
        {

            case TxKeyerId::RigControl:         return "Voice RigControl";
            case TxKeyerId::CW_RigControl:      return "CW RigControl";
            case TxKeyerId::SerialControl:      return "SerialControl";
            case TxKeyerId::PcCwKeyer:          return "PcCwKeyer";
            case TxKeyerId::DigitalModes:       return "Digital Modes";
            case TxKeyerId::InternalVoiceKeyer: return "Internal";
            case TxKeyerId::ExternalMqtKeyer:   return "mqtKeyer";
            case TxKeyerId::WinKeyer:           return "WinKeyer";
            default:                            return "";
        }
    }

    // for filenames
    QString getTxKeyerTypes(TxKeyerId id)
    {
        switch (id)
        {

            case TxKeyerId::RigControl:         return "Voice RigControl";
            case TxKeyerId::CW_RigControl:      return "CW RigControl";
            case TxKeyerId::SerialControl:      return "SerialControl";
            case TxKeyerId::PcCwKeyer:          return "PcCwKeyer";
            case TxKeyerId::DigitalModes:       return "Digital Modes";
            case TxKeyerId::InternalVoiceKeyer: return "Internal";
            case TxKeyerId::ExternalMqtKeyer:   return "mqtKeyer";
            case TxKeyerId::WinKeyer:           return "WinKeyer";
            default:                            return "";
        }
    }







    enum class KeyerEomTypes
    {
        Eom_None = 0,
        CAT,
        Timer,
        InternalSoundCardVoiceKeyer,
        DTRKeyerTXStatus,
        DTRKeyerPttStatus,
        WinKeyerTXStatus
    };

    inline const QString VOICEKEYER_COMMON_KEY = "commonParams";

    //inline const QString VOICE_KEYER_PATH = "./Configuration/VoiceKeyer/";
    inline const QString VOICE_KEYER_BASE_FILE_NAME = "txVoiceMemory";
    //inline const QString VOICEKEYER_COMMON_PARAMS_PATH = VOICE_KEYER_PATH + "CommonParams/";
    inline const QString VOICEKEYER_COMMON_PARAMS_FILENAME = "txVoiceKeyCommonParams.ini";
    inline const QString VOICEKEYER_COMMON_PARAMS_GROUPNAME = VOICEKEYER_COMMON_KEY;
    inline const QString CWKEYER_RADIO_COMMON_PARAMS_FILENAME = "cwRadioParams";
    inline const QString PC_CW_KEYER_COMMON_PARAMS_FILENAME = "pcCwKeyerParams";
    inline const QString DEFAULT_SUPPORTED_CW_CHARACTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 /?{}";
    inline const QString DEFAULT_MACRO_CW_CHARACTERS = "{}*#!%$";
    inline const QString DEFAULT_LENGTH_CW_MESSAGE = "24";
    inline const QString CW_MESSAGE_BASE_FILE_NAME = "txCwMemory";

    inline const QString ALL_RADIOS_GROUP_NAME = "AllRadios";

    inline const QString KEYER_NO_RADIO = "noRadio";
    inline const QString KEYER_DEFAULT_CONTEST = "default";

    inline const int VOICEKEYER_MAX_NUMBUTTONS = 8;

    inline const QString specialCwCharEscapeChar = "^";

    inline const QString STOP_VOICE_MESSAGE = "STOPVOICE";
    inline const QString STOP_CW_MESSAGE = "STOPCW";
}



QString VOICE_KEYER_PATH();
QString VOICEKEYER_COMMON_PARAMS_PATH();

QString TX_KEYER_PATH();
QString TXKEYER_COMMON_PARAMS_PATH();

bool readSaveVoiceCWMemoryButtonByRadioNameFromIni(TxKeyerCommon::TxKeyerId id);
void writeSaveVoiceCWMemoryButtonByRadioNameToIni(bool data, TxKeyerCommon::TxKeyerId id);
void getListOfRadioNamesForMemoryButtons(QStringList &radioNames, TxKeyerCommon::TxKeyerId id);

bool getRigCWKeyerSupportedSpecialCharacters(QString filePath, QMap<QString, QChar> &specialCharMap, QString radioMfg, const QString filename);
bool getRigCWKeyerMacroCharacter(QString filePath, QString &macroChars, QString radioMfg, const QString filename);
bool getRigCWKeyerListOfRadiosSupportSpecialCharacters(QString filePath, QStringList &listOfRadioModels, QString radioMfg, const QString filename);
bool getRigCWKeyerSupportedCharacters(QString filePath, QString &supportedChars, QString radioMfg, const QString filename);
bool getRigCWMessageStoredInRadioFlag(bool &storedInRadio, QString radioMfg, const QString filename);
bool getRigCWKeyerMaxMessageLength(QString filePath, int &messageLength, QString radioMfg, const QString filename);

QString getCwRadioManufacturer(int cwMemType);




#endif // TXKEYERCOMMONCONSTANTS_H
