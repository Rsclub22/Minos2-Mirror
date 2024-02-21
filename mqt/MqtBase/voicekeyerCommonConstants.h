#ifndef VOICEKEYERCOMMONCONSTANTS_H
#define VOICEKEYERCOMMONCONSTANTS_H



#include <QString>
#include <QStringList>


namespace voiceKeyerCommon
{

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

    const QString CW_MESSAGE_BASE_FILE_NAME = "txCwMemory";

    const QString ALL_RADIOS_GROUP_NAME = "AllRadios";

    const int VOICEKEYER_MAX_NUMBUTTONS = 8;
}



QString VOICE_KEYER_PATH();
QString VOICEKEYER_COMMON_PARAMS_PATH();
bool readSaveVoiceCWMemoryButtonByRadioNameFromIni();
void writeSaveVoiceCWMemoryButtonByRadioNameToIni(bool data);
void getListOfRadioNamesForMemoryButtons(QStringList &radioNames, voiceKeyerCommon::VoiceKeyerId id);


#endif // VOICEKEYERCOMMONCONSTANTS_H
