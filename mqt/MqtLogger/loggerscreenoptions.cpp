#include "loggerscreenoptions.h"
#include "ScreenConfigManager.h"

QVector <SCTypeOption> LoggerScreenOptions::loggerScreenOptions =
    {
        {sctMainScreen, QT_TR_NOOP("mainscreen"), QT_TR_NOOP("Main Screen")},
        {sctScreen, QT_TR_NOOP("screen"), QT_TR_NOOP("Secondary Screen")},
        {sctAux, QT_TR_NOOP("Auxiliary"), QT_TR_NOOP("Auxiliary Display")},
        {sctChat, QT_TR_NOOP("Chat Display"), QT_TR_NOOP("Chat Display")},
        {sctCluster, QT_TR_NOOP("Cluster Display"), QT_TR_NOOP("Cluster Display")},
        {sctLog, QT_TR_NOOP("Log List"), QT_TR_NOOP("QSO Log List")},
        {sctNextQSODetails, QT_TR_NOOP("Next QSO Details"), QT_TR_NOOP("Next QSO details")},
        {sctQSOEdit, QT_TR_NOOP("QSO Edit"), QT_TR_NOOP("QSO Edit")},
        {sctRigControl, QT_TR_NOOP("Rig Control"), QT_TR_NOOP("Rig Control")},
        {sctBandSwitch, QT_TR_NOOP("HF Band Switching"), QT_TR_NOOP("HF Band Switching")},
        {sctRunButtons, QT_TR_NOOP("Call Freq Buttons"), QT_TR_NOOP("Run Freq Buttons")},
        {sctRotControl, QT_TR_NOOP("Rotator Control"), QT_TR_NOOP("Rotator Control")},
        {sctSkyScanControl, QT_TR_NOOP("SkyScan Control"), QT_TR_NOOP("SkyScan Control")},
        {sctRotCompassDisplay, QT_TR_NOOP("Rotator Compass Display"), QT_TR_NOOP("Rotator Compass Display")},
        {sctRotPresets, QT_TR_NOOP("Rotator Presets"), QT_TR_NOOP("Rotator Presets")},
        {sctRotSkyScanPresets, QT_TR_NOOP("SkyScan Presets"), QT_TR_NOOP("SkyScan Presets")},
        {sctThisMatch, QT_TR_NOOP("This Contest Match"), QT_TR_NOOP("This Contest Matches")},
        {sctOtherMatch, QT_TR_NOOP("Other Contest Match"), QT_TR_NOOP("Other Contest Matches") },
        {sctArchiveMatch, QT_TR_NOOP("Archive Match"), QT_TR_NOOP("Archive List Matches") },
        {sctWsjtx, QT_TR_NOOP("WSJT-X Connector"), QT_TR_NOOP("WSJT-X Connector") },
        {sctBandmap, QT_TR_NOOP("Bandmap Display"), QT_TR_NOOP("Bandmap Display")},
        {sctSplit, QT_TR_NOOP("HSplit"), QT_TR_NOOP("Horizontally split element")},
        {sctTxVmButtons, QT_TR_NOOP("Keyer"), QT_TR_NOOP("Keyer")},
        {sctQrzDisplay, QT_TR_NOOP("QRZ Display"), QT_TR_NOOP("QRZ Display")},
        {sctQsoMap, QT_TR_NOOP("QSO Map"), QT_TR_NOOP("QSO Map")},
        {sctDMButtons, QT_TR_NOOP("Data Modes Buttons"), QT_TR_NOOP("Data Modes Buttons")},
        {sctNone, QT_TR_NOOP("None"), QT_TR_NOOP("Not in use")}
};

/*
[{"name": "default",
"rows":[
[{"type": "Log"},{"type": "Aux"}],
[{"type": "Rig"},{"type": "Run"},{"type": "Rot"},{"type": "RotP"}],
[{"type": "QSO"},{"type": "Crib"}],
[{"type": "This"},{"type": "Other"},{"type": "Arch"}]
]}]
*/
/*
[{"name": "protected",
"rows":[[
[{"rows": [
[{"type": "Log List"}],
[{"type": "QSO Edit"}],
[{"type": "This Contest Match"}]
],"type": "HSplit"},{
"rows": [
[{"auxtype": "Stats","type": "Auxiliary"}],
[{"auxtype": "Locator Map","type": "Auxiliary"}],
[{"auxtype": "Clock","type": "Auxiliary"}]
],"type": "HSplit"}]
]}
*/
// test example for multiple screen layout
/*
[
    {
        "name": "multiscreen",
          "screens": [
            [
              { "mainscreen": "Main Screen" },
              { "rows": [
                  [
                    { "type": "Log" },
                    { "type": "Aux" }
                  ],
                  [
                    { "type": "Rig" },
                    { "type": "Run" },
                    { "type": "Rot" },
                    { "type": "RotP" }
                  ],
                  [
                    { "type": "QSO" },
                    { "type": "Crib" }
                  ],
                  [
                    { "type": "This" },
                    { "type": "Other" },
                    { "type": "Arch" }
                  ]
                ]
              }
            ],
            [
              { "screen": "WSJT-X Screen" },
              { "rows": [
                  [
                    { "type": "WSJT-X Connector" }
                  ]
                ]
              }
            ]
          ]
    }
]
*/
QString LoggerScreenOptions::defaultConfig = "[{\"name\": \"%1\","
                                          "\"rows\":["
                                          "[{\"type\": \"%2\"},{\"type\": \"%3\"}],"
                                          "[{\"type\": \"%4\"},{\"type\": \"%5\"},{\"type\": \"%6\"},{\"type\": \"%7\"}],"
                                          "[{\"type\": \"%8\"},{\"type\": \"%9\"}],"
                                          "[{\"type\": \"%10\"},{\"type\": \"%11\"},{\"type\": \"%12\"}]"
                                          "]}]";

QString LoggerScreenOptions::protectedConfig  = "[{\"name\": \"%1\","
                                            "\"rows\":["
                                            "[{\"type\": \"%2\"}],"
                                            "[{\"type\": \"%3\"}],"
                                            "[{\"type\": \"%4\"}]"
                                            "]}]";


LoggerScreenOptions::LoggerScreenOptions():QObject()
{
    ScreenConfigFile::setFileName("ScreenConfigs.json");

    ScreenConfigElement::setScreenOptions(loggerScreenOptions);

    QString def = defaultConfig
            .arg(defaultLayoutName())
            .arg(ScreenConfigElement::getRawScreenTypeString(sctLog))
            .arg(ScreenConfigElement::getRawScreenTypeString(sctAux))
            .arg(ScreenConfigElement::getRawScreenTypeString(sctRigControl))
            .arg(ScreenConfigElement::getRawScreenTypeString(sctRunButtons))
            .arg(ScreenConfigElement::getRawScreenTypeString(sctRotControl))
            .arg(ScreenConfigElement::getRawScreenTypeString(sctRotPresets))
            .arg(ScreenConfigElement::getRawScreenTypeString(sctQSOEdit))
            .arg(ScreenConfigElement::getRawScreenTypeString(sctNextQSODetails))
            .arg(ScreenConfigElement::getRawScreenTypeString(sctThisMatch))
            .arg(ScreenConfigElement::getRawScreenTypeString(sctOtherMatch))
            .arg(ScreenConfigElement::getRawScreenTypeString(sctArchiveMatch));

    QString prot = protectedConfig
            .arg(defaultProtectedLayoutName(),
                            ScreenConfigElement::getRawScreenTypeString(sctLog),
                            ScreenConfigElement::getRawScreenTypeString(sctQSOEdit),
                            ScreenConfigElement::getRawScreenTypeString(sctThisMatch));

    ScreenConfigFile::setDefProt(def, prot);

}

