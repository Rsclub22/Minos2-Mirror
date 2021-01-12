#include "base_pch.h"

#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include "profiles.h"

#include "ScreenConfigFile.h"
#include "ScreenConfigElement.h"
/*
[{"name": "default",
"rows":[
[{"type": "Log"},{"type": "Aux"}],
[{"type": "Rig"},{"type": "Call"},{"type": "Rot"},{"type": "RotP"}],
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
                    { "type": "Call" },
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
static QString defaultConfig = "[{\"name\": \"%1\","
        "\"rows\":["
        "[{\"type\": \"%2\"},{\"type\": \"%3\"}],"
        "[{\"type\": \"%4\"},{\"type\": \"%5\"},{\"type\": \"%6\"},{\"type\": \"%7\"}],"
        "[{\"type\": \"%8\"},{\"type\": \"%9\"}],"
        "[{\"type\": \"%10\"},{\"type\": \"%11\"},{\"type\": \"%12\"}]"
        "]}]";

static QString protectedConfig  = "[{\"name\": \"%1\","
                               "\"rows\":["
                               "[{\"type\": \"%2\"}],"
                               "[{\"type\": \"%3\"}],"
                               "[{\"type\": \"%4\"}]"
                               "]}]";

ScreenConfigFile::ScreenConfigFile()
{
}

ScreenConfigFile::~ScreenConfigFile()
{

}
void ScreenConfigFile::loadFile(QWidget *parent)
{
    readFile("./Configuration/ScreenConfigs.json", parent);
}
bool ScreenConfigFile::dumpFile()
{
    return writeFile("./Configuration/ScreenConfigs.json");
}
void ScreenConfigFile::procScreens(QVector<SCScreen> &elescr, QJsonArray &screens)
{
    for (auto const &s: screens)
    {
        SCScreen screen;
        QJsonArray elearray = s.toArray();
        for (auto const &e: elearray)
        {
            QJsonObject ele = e.toObject();
            QString key = ele.keys().at(0);
            if (key == "mainscreen")
            {
                screen.baseElement = QSharedPointer<SCElement>(new SCElement());
                screen.baseElement->type = sctMainScreen;
                screen.name = ele.value(key).toString();
            }
            else if (key == "screen")
            {
                screen.baseElement = QSharedPointer<SCElement>(new SCElement());
                screen.baseElement->type = sctScreen;
                screen.name = ele.value(key).toString();
            }
            else if (key == "rows")
            {
                QJsonArray rows = ele.value(key).toArray();
                procRows(screen.baseElement->rows, rows);
            }

        }
    }
}
void ScreenConfigFile::procSingleScreen(SC &config, QJsonValue &base)
{
    // Wrap an old style config in new style mainScreen
    config.baseElement->screens.append(SCScreen());

    SCScreen &scs =config.baseElement->screens[0] ;
    scs.baseElement = QSharedPointer<SCElement>(new SCElement());
    scs.baseElement->type = sctMainScreen;

    QJsonArray rows = base.toArray();
    procRows(scs.baseElement->rows, rows);
}

void ScreenConfigFile::procRows(QVector<SCRow> &elerows, QJsonArray &rows)
{
    for (auto const &r: rows)
    {
        SCRow scrow;
        QJsonArray elearray = r.toArray();
        for (auto const &e: elearray)
        {
            SCElement scele;
            QJsonObject ele = e.toObject();
            QString eletype = ele.value("type").toString();
            scele.type = ScreenConfigElement::getScreenType(eletype);
            if (scele.type == sctSplit)
            {
                QJsonArray rows = ele.value("rows").toArray();
                procRows(scele.rows, rows);
            }
            else if (scele.type == sctAux)
            {
                QString auxtype = ele.value("auxtype").toString();
                scele.auxType = StackedInfoFrame::getAuxEntryType(auxtype);
            }
            else
            {
                scele.auxType = aeClock;
            }
//                        trace(QString("Name %1 row %2 ele %3 type %4").arg(name).arg(j).arg(k).arg(eletype));
            scrow.elements.push_back(scele);
        }
        elerows.push_back(scrow);
    }
}
void ScreenConfigFile::readFile(QString f, QWidget *parent)
{
    QString s;

    configs.clear();

    trace("Using default configuration");
    s = defaultConfig
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
    parseConfigString(s);

    trace("Using default protectected configuration");
    s = protectedConfig
            .arg(defaultProtectedLayoutName())
            .arg(ScreenConfigElement::getRawScreenTypeString(sctLog))
            .arg(ScreenConfigElement::getRawScreenTypeString(sctQSOEdit))
            .arg(ScreenConfigElement::getRawScreenTypeString(sctThisMatch));
    parseConfigString(s);

    bool retval = false;

    QFile jf(f);
    if (jf.open(QIODevice::ReadOnly))
    {
        s = jf.readAll();
        retval = parseConfigString(s);
        if (retval == false)
        {
            mShowMessage(tr("Invalid or missing screen configurations; using built in defaults"), parent);
        }
    }
    else
    {
        trace("Failed to open " + f );
    }
}
bool ScreenConfigFile::parseConfigString(QString s)
{
    QJsonParseError err;
    QJsonDocument json = QJsonDocument::fromJson(s.toUtf8(), &err);
    if (!err.error)
    {
        if( json.isArray())
        {
            QJsonArray namearray = json.array();
            for (auto const &n: namearray)
            {
                SC config;
                QJsonObject namestruct = n.toObject();
                QString name = namestruct.value("name").toString();
                config.name = name;
                config.baseElement = QSharedPointer<SCElement>(new SCElement());
                QJsonValue base = namestruct.value("screens");
                if (base != QJsonValue::Undefined)
                {
                    QJsonArray screens = base.toArray();
                    procScreens(config.baseElement->screens, screens);
                    configs[name] = config;
                }
                base = namestruct.value("rows");
                if (base != QJsonValue::Undefined)
                {
                    procSingleScreen(config, base);
                    configs[name] = config;
                }
            }
            return true;
        }
        else
        {
            trace("Not a JSON array");
            return false;
        }
    }
    else
    {
        trace("Err " + err.errorString() + " Bad Json document " + s);
        return false;
    }
}
void ScreenConfigFile::writeTypetoRow(SCElement &e, QJsonArray &scrow)
{
    QJsonObject scele;
    SCType sctype = e.type;
    scele.insert("type", ScreenConfigElement::getRawScreenTypeString(sctype));

    if (sctype == sctSplit)
    {
        QJsonArray splitRows;

        for (auto &j: e.rows)
        {
            QJsonArray splitRow;
           for (auto &k: j.elements)
           {
               writeTypetoRow(k, splitRow);
           }
           splitRows.append(splitRow);
        }
        scele.insert("rows", splitRows);
    }
    else if (sctype == sctAux)
    {
        scele.insert("auxtype", StackedInfoFrame::getRawAuxTypeString(e.auxType));
    }
    scrow.append(scele);
}
bool ScreenConfigFile::writeFile(QString f)
{
    QFile jf(f);
    if (!jf.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        trace("Failed to open " + f );
        return false;
    }
     QJsonDocument json;
     QJsonArray scarray;

     for(auto &i: configs )
     {
         QJsonObject sc;
         QJsonArray scrows;
         QString name = i.name;
         SC &scb = i;
         for (auto &j: scb.baseElement->rows)
         {
            QJsonArray scrow;
            for (auto &k: j.elements)
            {
                writeTypetoRow(k, scrow);
            }
            scrows.append(scrow);
         }
         sc.insert("name", name);
         sc.insert("rows", scrows);

         scarray.append(sc);
     }
     json.setArray(scarray);

     QByteArray s = json.toJson();
     jf.write(s);

     jf.close();

     return true;
}

