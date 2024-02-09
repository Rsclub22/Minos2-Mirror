#include <QSettings>
#include "voicekeyerCommonConstants.h"



using namespace voiceKeyerCommon;

bool readSaveVoiceCWMemoryButtonByRadioNameFromIni()
{
    QString fileName = VOICEKEYER_COMMON_PARAMS_PATH + VOICE_KEYER_BASE_FILE_NAME + keyerTypes[VoiceKeyerId::RigControl] + ".ini";
    QSettings readConfig(fileName, QSettings::IniFormat);

    return readConfig.value("Common/SaveButtonByRadioName", false).toBool();

}

void writeSaveVoiceCWMemoryButtonByRadioNameToIni(bool data)
{
    QString fileName = VOICEKEYER_COMMON_PARAMS_PATH + VOICE_KEYER_BASE_FILE_NAME + keyerTypes[VoiceKeyerId::RigControl] + ".ini";
    QSettings config(fileName, QSettings::IniFormat);

    config.setValue("Common/SaveButtonByRadioName", data);

}

