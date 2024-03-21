#ifndef VOICEKEYERCOMMONCONSTANTS_H
#define VOICEKEYERCOMMONCONSTANTS_H



#include <QString>
#include <QStringList>





namespace voiceKeyerCommon
{
    // These are Minos max and min keyer buttons
    // Rigcontrol with hamlib will provide max number of memories the
    // radio will support.

    const int MAXIMUM_BUTTONS = 8;
    const int MININUM_BUTTONS = 2;

    enum VoiceKeyerId
    {
        None = 0,
        RigControl,
        CW_RigControl,    // Rig sends message by CW
        SerialControl,    // Sends user serial message to control external voice keyer
        InternalVoiceKeyer,
        ExternalVoiceKeyer
    };

    const QStringList keyerTypes = {"", "Voice RigControl", "CW RigControl", "SerialControl", "Internal", "mqtKeyer"};

    const QString VOICEKEYER_COMMON_KEY = "commonParams";

    //const QString VOICE_KEYER_PATH = "./Configuration/VoiceKeyer/";
    const QString VOICE_KEYER_BASE_FILE_NAME = "txVoiceMemory";
    //const QString VOICEKEYER_COMMON_PARAMS_PATH = VOICE_KEYER_PATH + "CommonParams/";
    const QString VOICEKEYER_COMMON_PARAMS_FILENAME = "txVoiceKeyCommonParams.ini";
    const QString VOICEKEYER_COMMON_PARAMS_GROUPNAME = VOICEKEYER_COMMON_KEY;
    const QString CWKEYER_RADIO_COMMON_PARAMS_FILENAME = "cwRadioParams";
    const QString CW_MESSAGE_BASE_FILE_NAME = "txCwMemory";

    const QString ALL_RADIOS_GROUP_NAME = "AllRadios";

    const int VOICEKEYER_MAX_NUMBUTTONS = 8;

    const QString specialCwCharEscapeChar = "^";
}



QString VOICE_KEYER_PATH();
QString VOICEKEYER_COMMON_PARAMS_PATH();
bool readSaveVoiceCWMemoryButtonByRadioNameFromIni(voiceKeyerCommon::VoiceKeyerId id);
void writeSaveVoiceCWMemoryButtonByRadioNameToIni(bool data, voiceKeyerCommon::VoiceKeyerId id);
void getListOfRadioNamesForMemoryButtons(QStringList &radioNames, voiceKeyerCommon::VoiceKeyerId id);

bool getRigCWKeyerSupportedSpecialCharacters(QMap<QString, QChar> &specialCharMap, QString radioMfg);
bool getRigCWKeyerListOfRadiosSupportSpecialCharacters(QStringList &listOfRadioModels, QString radioMfg);
bool getRigCWKeyerSupportedCharacters(QString &supportedChars, QString radioMfg);
bool getRigCWMessageStoredInRadioFlag(bool &storedInRadio, QString radioMfg);
bool getRigCWKeyerMaxMessageLength(int &messageLength, QString radioMfg);

QString getCwRadioManufacturer(int cwMemType);


#endif // VOICEKEYERCOMMONCONSTANTS_H
