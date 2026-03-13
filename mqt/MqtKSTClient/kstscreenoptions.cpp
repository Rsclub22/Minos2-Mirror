#include "kstscreenoptions.h"
#include "ScreenConfigManager.h"

QString KSTScreenOptions::defaultConfig =
    "[{\"name\": \"%1\","
    "\"rows\":[[{\"type\": \"%2\"},{\"type\": \"%3\"}],"
    "[{\"rows\": [[{\"type\": \"%4\"}],[{\"type\": \"%5\"}]],\"type\": \"%6\"},"
    "{\"rows\": [[{\"type\": \"%7\"}],[{\"type\": \"%8\"}]],\"type\": \"%9\"}],"
    "[{\"type\": \"%10\"}],"
    "[{\"type\": \"%11\"}]"
    "]}]";


QString KSTScreenOptions::protectedConfig;

QVector <SCTypeOption> KSTScreenOptions::kstScreenOptions =
    {
        {sctMainScreen, QT_TR_NOOP("mainscreen"), QT_TR_NOOP("Main Screen")},
        {sctScreen, QT_TR_NOOP("screen"), QT_TR_NOOP("Secondary Screen")},
        {sctSplit, QT_TR_NOOP("HSplit"), QT_TR_NOOP("Horizontally split element")},

    {sctNone, QT_TR_NOOP("None"), QT_TR_NOOP("Not in use")},

        {sctkLogins, QT_TR_NOOP("Chats"), QT_TR_NOOP("Chats logged in")},
        {sctkCallList, QT_TR_NOOP("Call List"), QT_TR_NOOP("Active Callsigns")},
        {sctkAirScout, QT_TR_NOOP("AirScout"), QT_TR_NOOP("AirScout")},
        {sctkASActive, QT_TR_NOOP("AirScout Activation"), QT_TR_NOOP("AirScout Activation")},
        {sctkMessageList, QT_TR_NOOP("Messages"), QT_TR_NOOP("Messages")},
        {sctkMeepList, QT_TR_NOOP("Meeps"), QT_TR_NOOP("Messages to me")},
        {sctkSendMeep, QT_TR_NOOP("SendMeep"), QT_TR_NOOP("Sending Meeps")},
        {sctkButtons, QT_TR_NOOP("Buttons"), QT_TR_NOOP("Buttons")}
};


KSTScreenOptions::KSTScreenOptions()
{
    ScreenConfigFile::setFileName("KSTScreenConfigs.json");
    ScreenConfigElement::setScreenOptions(kstScreenOptions);

    QString def = defaultConfig
                      .arg(defaultLayoutName())
                      .arg(ScreenConfigElement::getRawScreenTypeString(sctkLogins))
                      .arg(ScreenConfigElement::getRawScreenTypeString(sctkASActive))
                      .arg(ScreenConfigElement::getRawScreenTypeString(sctkCallList))
                      .arg(ScreenConfigElement::getRawScreenTypeString(sctkAirScout))
                      .arg(ScreenConfigElement::getRawScreenTypeString(sctSplit))
                      .arg(ScreenConfigElement::getRawScreenTypeString(sctkMessageList))
                      .arg(ScreenConfigElement::getRawScreenTypeString(sctkMeepList))
                      .arg(ScreenConfigElement::getRawScreenTypeString(sctSplit))
                      .arg(ScreenConfigElement::getRawScreenTypeString(sctkSendMeep))
                      .arg(ScreenConfigElement::getRawScreenTypeString(sctkButtons));

    ScreenConfigFile::setDefProt(def, protectedConfig);
}
