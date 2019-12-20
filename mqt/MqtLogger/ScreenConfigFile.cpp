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
[{"type": "Rig"},{"type": "Rot"},{"type": "RotP"}],
[{"type": "QSO"},{"type": "Crib"}],
[{"type": "This"},{"type": "Other"},{"type": "Arch"}]
]}]
*/
static QString defaultConfig = "[{\"name\": \"%1\","
        "\"rows\":["
        "[{\"type\": \"%2\"},{\"type\": \"%3\"}],"
        "[{\"type\": \"%4\"},{\"type\": \"%5\"},{\"type\": \"%6\"}],"
        "[{\"type\": \"%7\"},{\"type\": \"%8\"}],"
        "[{\"type\": \"%9\"},{\"type\": \"%10\"},{\"type\": \"%11\"}]"
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
void ScreenConfigFile::procRows(QVector<SCRow> &elerows, QJsonArray &rows)
{
    for (int j = 0; j < rows.count(); j++)
    {
        SCRow scrow;
        QJsonArray elearray = rows[j].toArray();
        for (int k = 0; k < elearray.count(); k++)
        {
            SCElement scele;
            QJsonObject ele = elearray[k].toObject();
            QString eletype = ele.value("type").toString();
            scele.type = getScreenType(eletype);
            if (scele.type == sctSplit)
            {
                QJsonArray rows = ele.value("rows").toArray();
                procRows(scele.rows, rows);
            }
            else if (scele.type == sctAux)
            {
                QString auxtype = ele.value("auxtype").toString();
                scele.auxType = getAuxEntryType(auxtype);
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
    QFile jf(f);
    QString s;
    bool retval = false;
    if (jf.open(QIODevice::ReadOnly))
    {
        s = jf.readAll();
        retval = parseConfigString(s);
        if (retval == false)
        {
            mShowMessage("Invalid or missing screen configurations; using built in defaults", parent);
        }
    }
    else
    {
        trace("Failed to open " + f );
    }
    if (retval == false)
    {
        trace("Using default configuration");
        s = defaultConfig
                .arg(defaultLayoutName)
                .arg(getScreenTypeString(sctLog))
                .arg(getScreenTypeString(sctAux))
                .arg(getScreenTypeString(sctRigControl))
                .arg(getScreenTypeString(sctRotControl))
                .arg(getScreenTypeString(sctRotPresets))
                .arg(getScreenTypeString(sctQSOEdit))
                .arg(getScreenTypeString(sctNextQSODetails))
                .arg(getScreenTypeString(sctThisMatch))
                .arg(getScreenTypeString(sctOtherMatch))
                .arg(getScreenTypeString(sctArchiveMatch));
        parseConfigString(s);
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
            configs.clear();
            QJsonArray namearray = json.array();
            for (int i = 0; i < namearray.count(); i++)
            {
                SC config;
                QJsonObject namestruct = namearray[i].toObject();
                QString name = namestruct.value("name").toString();
                config.name = name;
                config.baseElement = QSharedPointer<SCElement>(new SCElement());
                QJsonArray rows = namestruct.value("rows").toArray();
                procRows(config.baseElement->rows, rows);
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
void ScreenConfigFile::writeTypetoRow(SCElement &e, QJsonArray &scrow)
{
    QJsonObject scele;
    SCType sctype = e.type;
    scele.insert("type", getScreenTypeString(sctype));

    if (sctype == sctSplit)
    {
        QJsonArray splitRows;

        for (int j = 0; j < e.rows.count(); j++)
        {
            QJsonArray splitRow;
           for (int k = 0; k < e.rows[j].elements.count(); k++)
           {
               writeTypetoRow(e.rows[j].elements[k], splitRow);
           }
           splitRows.append(splitRow);
        }
        scele.insert("rows", splitRows);
    }
    else if (sctype == sctAux)
    {
        scele.insert("auxtype", getAuxTypeString(e.auxType));
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

     for(QMap<QString, SC>::iterator i = configs.begin(); i != configs.end(); i++ )
     {
         QJsonObject sc;
         QJsonArray scrows;
         QString name = i.value().name;
         SC &scb = i.value();
         for (int j = 0; j < scb.baseElement->rows.count(); j++)
         {
            QJsonArray scrow;
            for (int k = 0; k < scb.baseElement->rows[j].elements.count(); k++)
            {
                writeTypetoRow(scb.baseElement->rows[j].elements[k], scrow);
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
