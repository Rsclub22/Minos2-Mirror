#ifndef VOICEKEYERCOMMONCONSTANTS_H
#define VOICEKEYERCOMMONCONSTANTS_H



#include <QString>
#include <QStringList>
#include <QValidator>





namespace voiceKeyerCommon
{
    // These are Minos max and min keyer buttons
    // Rigcontrol with hamlib will provide max number of voice memories the
        // radio will support.

    const int PC_CW_KEYER_MAXIMUM_BUTTONS = 12;
    const int PC_CW_KEYER_MIN_WPM = 5;
    const int PC_CW_KEYER_MAX_WPM = 40;
    const int MAXIMUM_BUTTONS = 8;
    const int MININUM_BUTTONS = 2;


    enum VoiceKeyerId
    {
        None = 0,
        RigControl,
        CW_RigControl,    // Rig sends message by CW
        SerialControl,    // Sends user serial message to control external voice keyer
        PcCwKeyer,        // Serial DTR keying of radio
        InternalVoiceKeyer,
        ExternalVoiceKeyer
    };

    enum VoiceCwKeyerEomTypes
    {
        Eom_None = 0,
        CAT,
        Timer,
        InternalSoundCardVoiceKeyer,
        DTRKeyerTXStatus
    };

    inline const QStringList keyerTypes = {"", "Voice RigControl", "CW RigControl", "SerialControl", "PcCwKeyer", "Internal", "mqtKeyer"};

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

    inline const int VOICEKEYER_MAX_NUMBUTTONS = 8;

    inline const QString specialCwCharEscapeChar = "^";

    inline const QString STOP_VOICE_MESSAGE = "STOPVOICE";
    inline const QString STOP_CW_MESSAGE = "STOPCW";
}



QString VOICE_KEYER_PATH();
QString VOICEKEYER_COMMON_PARAMS_PATH();
bool readDisableESCKeyerStopFromIni();
void writeDisableESCKeyerStopToIni(bool disable);
bool readSaveVoiceCWMemoryButtonByRadioNameFromIni(voiceKeyerCommon::VoiceKeyerId id);
void writeSaveVoiceCWMemoryButtonByRadioNameToIni(bool data, voiceKeyerCommon::VoiceKeyerId id);
void getListOfRadioNamesForMemoryButtons(QStringList &radioNames, voiceKeyerCommon::VoiceKeyerId id);

bool getRigCWKeyerSupportedSpecialCharacters(QMap<QString, QChar> &specialCharMap, QString radioMfg, const QString filename);
bool getRigCWKeyerMacroCharacter(QString &macroChars, QString radioMfg, const QString filename);
void addCWKeyerMacroCharsToValidCwKeyerChars(QString &mergedValidChar, const QString supportedCwChars, const QString macroCwChars);
bool getRigCWKeyerListOfRadiosSupportSpecialCharacters(QStringList &listOfRadioModels, QString radioMfg, const QString filename);
bool getRigCWKeyerSupportedCharacters(QString &supportedChars, QString radioMfg, const QString filename);
bool getRigCWMessageStoredInRadioFlag(bool &storedInRadio, QString radioMfg, const QString filename);
bool getRigCWKeyerMaxMessageLength(int &messageLength, QString radioMfg, const QString filename);

QString getCwRadioManufacturer(int cwMemType);

bool getVmButtonsLeftJustifyFlag(const QString filename);




#endif // VOICEKEYERCOMMONCONSTANTS_H
