#include "base_pch.h"

#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>

#include "ScreenConfigFile.h"

/*
[
    {
        "name": "default",
        "rows": [
            [
                {
                    "type": "Log"
                },
                {
                    "type": "Aux"
                }
            ],
            [
                 {
                    "type": "QSO"
                },
                {
                    "type": "Crib"
                }
            ],
            [
                 {
                    "type": "Rot"
                },
                 {
                    "type": "Rig"
                }
            ]
       ]
    }
]
*/
ScreenConfigFile::ScreenConfigFile()
{
}
bool ScreenConfigFile::loadFile()
{
    bool ret = false;
    ret = readFile("./Configuration/ScreenConfigs.json");
    if (!ret)
        ret = readFile("./Configuration/DefaultScreen.json");
    return ret;
}
bool ScreenConfigFile::dumpFile()
{
    return writeFile("./Configuration/ScreenConfigs.json");
}
bool ScreenConfigFile::readFile(QString f)
{
    QJsonParseError err;
    QFile jf(f);
    if (!jf.open(QIODevice::ReadOnly))
    {
        trace("Failed to open " + f );
        return false;
    }
    QString s = jf.readAll();
    QJsonDocument json = QJsonDocument::fromJson(s.toUtf8(), &err);
    if (!err.error)
    {
        if( json.isArray())
        {
            configs.clear();
            SC config;
            QJsonArray namearray = json.array();
            for (int i = 0; i < namearray.count(); i++)
            {
                QJsonObject namestruct = namearray[i].toObject();
                QString name = namestruct.value("name").toString();
                config.name = name;
                QJsonArray rows = namestruct.value("rows").toArray();
                for (int j = 0; j < rows.count(); j++)
                {
                    SCRow scrow;
                    QJsonArray elearray = rows[j].toArray();
                    for (int k = 0; k < elearray.count(); k++)
                    {
                        SCElement scele;
                        QJsonObject ele = elearray[k].toObject();
                        QString eletype = ele.value("type").toString();
                        scele.type = eletype;
                        trace(QString("Name %1 row %2 ele %3 type %4").arg(name).arg(j).arg(k).arg(eletype));
                        scrow.elements.push_back(scele);
                    }
                    config.rows.push_back(scrow);
                }
                configs[name] = config;
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

     for(QMap<QString, SC>::iterator i = configs.begin(); i != configs.end(); i++ )
     {
         QJsonObject sc;
         QJsonArray scrows;
         for (int j = 0; j < i.value().rows.count(); j++)
         {
             QJsonArray scrow;
            for (int k = 0; k < i.value().rows[j].elements.count(); k++)
            {
                QJsonObject scele;
                scele.insert("type", i.value().rows[j].elements[k].type);
                scrow.append(scele);
            }
            scrows.append(scrow);
         }
         sc.insert("name", i.value().name);
         sc.insert("rows", scrows);

         scarray.append(sc);
     }
     json.setArray(scarray);

     QByteArray s = json.toJson();
     jf.write(s);

     jf.close();

     return true;
}
