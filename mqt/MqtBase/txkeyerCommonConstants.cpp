#include <QSettings>
#include "QtUtils.h"
#include "txkeyerCommonConstants.h"
#include "AppStartup.h"
#include "rigcontrolcommonconstants.h"

using namespace TxKeyerCommon;



// VoiceKeyerPanel *****************************************************************


QString VOICE_KEYER_PATH()
{
    return getDirectoryLocation(dlConfiguration) + "/VoiceKeyer/";
}

QString VOICEKEYER_COMMON_PARAMS_PATH()
{
    return VOICE_KEYER_PATH() + "CommonParams/";
}

// TxKeyer

QString TX_KEYER_PATH()
{
    return getDirectoryLocation(dlConfiguration) + "/TxKeyer/";
}




QString TXKEYER_COMMON_PARAMS_PATH()
{
    return TX_KEYER_PATH() + "CommonParams/";
}


bool readSaveVoiceCWMemoryButtonByRadioNameFromIni(TxKeyerId id)
{
    QString fileName = VOICEKEYER_COMMON_PARAMS_PATH() + VOICE_KEYER_BASE_FILE_NAME + getTxKeyerTypeFromTxKeyerId(id) + ".ini";
    QSettings readConfig(fileName, QSettings::IniFormat);

    return readConfig.value("Common/SaveButtonByRadioName", false).toBool();

}

void writeSaveVoiceCWMemoryButtonByRadioNameToIni(bool data, TxKeyerId id)
{
    QString fileName = VOICEKEYER_COMMON_PARAMS_PATH() + VOICE_KEYER_BASE_FILE_NAME + getTxKeyerTypeFromTxKeyerId(id) + ".ini";
    QSettings config(fileName, QSettings::IniFormat);

    config.setValue("Common/SaveButtonByRadioName", data);

}


void getListOfRadioNamesForMemoryButtons(QStringList &radioNames, TxKeyerId id)
{

    QString fileName = VOICE_KEYER_PATH() + VOICE_KEYER_BASE_FILE_NAME + getTxKeyerTypeFromTxKeyerId(id) + ".ini";
    QSettings config(fileName, QSettings::IniFormat);

    QStringList voiceMemoryKeys = config.childGroups();

    for (const auto &name: QASCONST(voiceMemoryKeys))
    {
        if (name != ALL_RADIOS_GROUP_NAME && name != "Common")
        {
            if (!radioNames.contains(name))
            {
                radioNames.append(name);
            }
        }
    }



}


// Not used - is it used by voiceKeyer panel
bool getRigCWMessageStoredInRadioFlag(bool &storedInRadio, QString radioMfg, const QString filename)
{
    QString fileName = VOICEKEYER_COMMON_PARAMS_PATH() + filename + ".ini";
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

bool getRigCWKeyerMaxMessageLength(QString filePath, int &messageLength, QString radioMfg, const QString filename)
{

    QString fileName = filePath + filename + ".ini";
    QSettings config(fileName, QSettings::IniFormat);

    if (config.childGroups().contains(radioMfg))
    {
        config.beginGroup(radioMfg);
        messageLength = config.value("maxMsgLength", DEFAULT_LENGTH_CW_MESSAGE).toInt();
        config.endGroup();
        return true;
    }

    return false;    // error


}


bool getRigCWKeyerSupportedCharacters(QString filePath, QString &supportedChars, QString radioMfg, const QString filename)
{
    QString fileName = filePath + filename + ".ini";
    QSettings config(fileName, QSettings::IniFormat);

    if (config.childGroups().contains(radioMfg))
    {
        config.beginGroup(radioMfg);
        supportedChars =  config.value("supportedCharacters", DEFAULT_SUPPORTED_CW_CHARACTERS).toString();
        config.endGroup();
        if (supportedChars.isEmpty())
        {
            return false;
        }

        return true;
    }

    return false;    // error
}

bool getRigCWKeyerMacroCharacter(QString filePath, QString &macroChars, QString radioMfg, const QString filename)
{
    QString fileName = filePath + filename + ".ini";
    QSettings config(fileName, QSettings::IniFormat);

    if (config.childGroups().contains(radioMfg))
    {
        config.beginGroup(radioMfg);
        macroChars =  config.value("macroCharacters", DEFAULT_SUPPORTED_CW_CHARACTERS).toString();
        config.endGroup();
        if (macroChars.isEmpty())
        {
            return false;
        }

        return true;
    }

    return false;    // error
}

bool getRigCWKeyerListOfRadiosSupportSpecialCharacters(QString filePath, QStringList &listOfRadioModels, QString radioMfg, const QString filename)
{
    QString fileName = filePath + filename + ".ini";
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


bool getRigCWKeyerSupportedSpecialCharacters(QString filePath, QMap<QString, QChar> &specialCharMap, QString radioMfg, const QString filename)
{
    QString fileName = filePath + filename + ".ini";
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
    QString type = "None";
    if (cwMemType == hamlibData::CW_MEMORY_TYPES::ICOM)
    {
        type = "Icom";
    }
    else if (cwMemType == hamlibData::CW_MEMORY_TYPES::YAESU)
    {
        type = "Yaesu";
    }
    else if (cwMemType == hamlibData::CW_MEMORY_TYPES::KENWOOD)
    {
        type = "Kenwood";
    }
    else if (cwMemType == hamlibData::CW_MEMORY_TYPES::ELECRAFT)
    {
        type = "Elecraft";
    }
    else if (cwMemType == hamlibData::CW_MEMORY_TYPES::FLEX_RADIO)
    {
        type = "Flex_Radio";
    }
    else if (cwMemType == hamlibData::CW_MEMORY_TYPES::FLEX_RADIO_APACHE)
    {
        type = "Flex_Radio_Apache";
    }
    else if (cwMemType == hamlibData::CW_MEMORY_TYPES::OPENHPSDR)
    {
        type = "OpenHPSDR";
    }
    else if (cwMemType == hamlibData::CW_MEMORY_TYPES::THETIS)
    {
        type = "Thetis";
    }
    else if (cwMemType == hamlibData::CW_MEMORY_TYPES::QRPLABS)
    {
        type = "QrpLabs";
    }
    else if (cwMemType == hamlibData::CW_MEMORY_TYPES::NONE)
    {
        type = "None";
    }

    return type;
}

TxKeyerCommonSettings TxKeyerCommon::loadTxKeyerCommonSettings()
{
    TxKeyerCommonSettings settings;

    QString fileName =
        TXKEYER_COMMON_PARAMS_PATH() +
        TX_KEYER_COMMON_PARAMS_FILENAME;

    QSettings config(fileName, QSettings::IniFormat);

    config.beginGroup(TX_KEYER_COMMON_PARAMS_GROUPNAME);

    QString viewModeStr = config.value("ViewMode","Standalone").toString();

    settings.viewMode = (viewModeStr == "Tabbed") ? KeyerViewMode::Tabbed : KeyerViewMode::Standalone;
    settings.standaloneKeyerName = config.value("StandaloneKeyerName", "").toString();
    settings.tabbedKeyersName = config.value("TabbedKeyers", QStringList()).toStringList();
    settings.activeTab = config.value("ActiveTab", 0).toInt();

    config.endGroup();

    return settings;
}

void TxKeyerCommon::saveTxKeyerCommonSettings(const TxKeyerCommonSettings &settings)
{
    QString fileName =
        TXKEYER_COMMON_PARAMS_PATH() +
        TX_KEYER_COMMON_PARAMS_FILENAME;

    QSettings config(fileName, QSettings::IniFormat);

    config.beginGroup(TX_KEYER_COMMON_PARAMS_GROUPNAME);
    config.setValue("ViewMode", settings.viewMode == KeyerViewMode::Tabbed ? "Tabbed" : "Standalone");
    config.setValue("StandaloneKeyer", settings.standaloneKeyerName);
    config.setValue("TabbedKeyers", settings.tabbedKeyersName);
    config.setValue("ActiveTab", settings.activeTab);
    config.endGroup();
}







