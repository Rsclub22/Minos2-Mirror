#include "kstscreenoptions.h"

QString KSTScreenOptions::defaultConfig ;

QString KSTScreenOptions::protectedConfig;
QVector <SCTypeOption> KSTScreenOptions::kstScreenOptions =
    {
        {sctMainScreen, QT_TR_NOOP("mainscreen"), QT_TR_NOOP("Main Screen")},
        {sctScreen, QT_TR_NOOP("screen"), QT_TR_NOOP("Secondary Screen")},

        {sctSplit, QT_TR_NOOP("HSplit"), QT_TR_NOOP("Horizontally split element")},

        {sctNone, QT_TR_NOOP("None"), QT_TR_NOOP("Not in use")}
};


KSTScreenOptions::KSTScreenOptions()
{
    ScreenConfigFile::setFileName("KSTScreenConfigs.json");
    ScreenConfigElement::setScreenOptions(kstScreenOptions);
    ScreenConfigFile::setDefProt(defaultConfig, protectedConfig);
}
