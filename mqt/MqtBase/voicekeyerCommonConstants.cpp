#include <QSettings>
#include "voicekeyerCommonConstants.h"
#include "AppStartup.h"




using namespace voiceKeyerCommon;

QString VOICE_KEYER_PATH()
{
    return getDirectoryLocation(dlConfiguration) + "/VoiceKeyer/";
}

QString VOICEKEYER_COMMON_PARAMS_PATH()
{
    return VOICE_KEYER_PATH() + "CommonParams/";
}


bool readSaveVoiceCWMemoryButtonByRadioNameFromIni()
{
    QString fileName = VOICEKEYER_COMMON_PARAMS_PATH() + VOICE_KEYER_BASE_FILE_NAME + keyerTypes[VoiceKeyerId::RigControl] + ".ini";
    QSettings readConfig(fileName, QSettings::IniFormat);

    return readConfig.value("Common/SaveButtonByRadioName", false).toBool();

}

void writeSaveVoiceCWMemoryButtonByRadioNameToIni(bool data)
{
    QString fileName = VOICEKEYER_COMMON_PARAMS_PATH() + VOICE_KEYER_BASE_FILE_NAME + keyerTypes[VoiceKeyerId::RigControl] + ".ini";
    QSettings config(fileName, QSettings::IniFormat);

    config.setValue("Common/SaveButtonByRadioName", data);

}


void getListOfRadioNamesForMemoryButtons(QStringList &radioNames, VoiceKeyerId id)
{

    QString fileName = VOICE_KEYER_PATH() + VOICE_KEYER_BASE_FILE_NAME + keyerTypes[id] + ".ini";
    QSettings config(fileName, QSettings::IniFormat);

    QStringList voiceMemoryKeys = config.childGroups();

    for (const auto &name: qAsConst(voiceMemoryKeys))
    {
        if (name != ALL_RADIOS_GROUP_NAME)
        {
            if (!radioNames.contains(name))
            {
                radioNames.append(name);
            }
        }
    }



}



