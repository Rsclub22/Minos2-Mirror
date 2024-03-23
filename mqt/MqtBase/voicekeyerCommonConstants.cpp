#include <QSettings>
#include "QtUtils.h"
#include "voicekeyerCommonConstants.h"
#include "AppStartup.h"
#include "rigcontrolcommonconstants.h"

using namespace voiceKeyerCommon;

QString VOICE_KEYER_PATH()
{
    return getDirectoryLocation(dlConfiguration) + "/VoiceKeyer/";
}

QString VOICEKEYER_COMMON_PARAMS_PATH()
{
    return VOICE_KEYER_PATH() + "CommonParams/";
}


bool readSaveVoiceCWMemoryButtonByRadioNameFromIni(VoiceKeyerId id)
{
    QString fileName = VOICEKEYER_COMMON_PARAMS_PATH() + VOICE_KEYER_BASE_FILE_NAME + keyerTypes[id] + ".ini";
    QSettings readConfig(fileName, QSettings::IniFormat);

    return readConfig.value("Common/SaveButtonByRadioName", false).toBool();

}

void writeSaveVoiceCWMemoryButtonByRadioNameToIni(bool data, VoiceKeyerId id)
{
    QString fileName = VOICEKEYER_COMMON_PARAMS_PATH() + VOICE_KEYER_BASE_FILE_NAME + keyerTypes[id] + ".ini";
    QSettings config(fileName, QSettings::IniFormat);

    config.setValue("Common/SaveButtonByRadioName", data);

}


void getListOfRadioNamesForMemoryButtons(QStringList &radioNames, VoiceKeyerId id)
{

    QString fileName = VOICE_KEYER_PATH() + VOICE_KEYER_BASE_FILE_NAME + keyerTypes[id] + ".ini";
    QSettings config(fileName, QSettings::IniFormat);

    QStringList voiceMemoryKeys = config.childGroups();

    for (const auto &name: QASCONST(voiceMemoryKeys))
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



bool getRigCWMessageStoredInRadioFlag(bool &storedInRadio, QString radioMfg)
{
    QString fileName = VOICEKEYER_COMMON_PARAMS_PATH() + CWKEYER_RADIO_COMMON_PARAMS_FILENAME + ".ini";
    QSettings config(fileName, QSettings::IniFormat);

    if (config.childGroups().contains(radioMfg))
    {
        config.beginGroup(radioMfg);
        storedInRadio =  config.value("messageStoredInRadio", false).toBool();
        config.endGroup();
        return true;
    }

    return false;    // error
}

bool getRigCWKeyerMaxMessageLength(int &messageLength, QString radioMfg)
{

    QString fileName = VOICEKEYER_COMMON_PARAMS_PATH() + CWKEYER_RADIO_COMMON_PARAMS_FILENAME + ".ini";
    QSettings config(fileName, QSettings::IniFormat);

    if (config.childGroups().contains(radioMfg))
    {
        config.beginGroup(radioMfg);
        messageLength = config.value("maxMsgLength", 0).toInt();
        config.endGroup();
        return true;
    }

    return false;    // error


}


bool getRigCWKeyerSupportedCharacters(QString &supportedChars, QString radioMfg)
{
    QString fileName = VOICEKEYER_COMMON_PARAMS_PATH() + CWKEYER_RADIO_COMMON_PARAMS_FILENAME + ".ini";
    QSettings config(fileName, QSettings::IniFormat);

    if (config.childGroups().contains(radioMfg))
    {
        config.beginGroup(radioMfg);
        supportedChars =  config.value("supportedCharacters", "").toString();
        config.endGroup();
        if (supportedChars.isEmpty())
        {
            return false;
        }

        return true;
    }

    return false;    // error
}



bool getRigCWKeyerListOfRadiosSupportSpecialCharacters(QStringList &listOfRadioModels, QString radioMfg)
{
    QString fileName = VOICEKEYER_COMMON_PARAMS_PATH() + CWKEYER_RADIO_COMMON_PARAMS_FILENAME + ".ini";
    QSettings config(fileName, QSettings::IniFormat);

    QString radios;

    if (config.childGroups().contains(radioMfg))
    {
        config.beginGroup(radioMfg);
        radios = config.value("radiosSupportSpecialChars", "").toString();
        config.endGroup();

        if (!radios.isEmpty())
        {
            listOfRadioModels = radios.split(',');
            if (!listOfRadioModels.isEmpty())
            {
                foreach (const QString radio, listOfRadioModels)
                {
                    listOfRadioModels.append(radio.trimmed());
                }

                return true;
            }
        }
    }

    return false;
}


bool getRigCWKeyerSupportedSpecialCharacters(QMap<QString, QChar> &specialCharMap, QString radioMfg)
{
    QString fileName = VOICEKEYER_COMMON_PARAMS_PATH() + CWKEYER_RADIO_COMMON_PARAMS_FILENAME + ".ini";
    QSettings config(fileName, QSettings::IniFormat);

    QString specialChars;
    QStringList listOfSpecialChars;

    if (config.childGroups().contains(radioMfg))
    {
        config.beginGroup(radioMfg);
        specialChars = config.value("specialCharacters", "").toString();
        config.endGroup();

        if (!specialChars.isEmpty())
        {
            listOfSpecialChars = specialChars.split(',');
            if (!listOfSpecialChars.isEmpty())
            {
                foreach (const QString specialChar, listOfSpecialChars)
                {
                    if (specialChar.length() == 4)
                    {
                        QString spCharStr = specialChar.left(2);
                        QChar spCharCode = specialChar.at(3);
                        specialCharMap.insert(spCharStr, spCharCode);
                    }
                    else
                    {
                        return false;
                    }
                }

                if (!specialCharMap.isEmpty())
                {
                    return true;
                }
            }
        }
    }

    return false;

}


QString getCwRadioManufacturer(int cwMemType)
{
    QString type;
    if (cwMemType == hamlibData::CW_MEMORY_TYPES::ICOM)
    {
        type = "Icom";
    }
    else if (cwMemType == hamlibData::CW_MEMORY_TYPES::YAESU_MEM_RECALL)
    {
        type = "Yaesu";
    }
    else if (cwMemType == hamlibData::CW_MEMORY_TYPES::KENWOOD_MEM_RECALL)
    {
        type = "Kenwood";
    }
    else if (cwMemType == hamlibData::CW_MEMORY_TYPES::ELECRAFT)
    {
        type = "Elecraft";
    }
    else if (cwMemType == hamlibData::CW_MEMORY_TYPES::NONE)
    {
        type = "None";
    }

    return type;
}




