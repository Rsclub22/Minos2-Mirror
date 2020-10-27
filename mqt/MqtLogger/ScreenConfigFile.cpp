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
static QString defaultConfig = "[{\"name\": \"%1\","
        "\"rows\":["
        "[{\"type\": \"%2\"},{\"type\": \"%3\"}],"
        "[{\"type\": \"%4\"},{\"type\": \"%5\"},{\"type\": \"%6\"},{\"type\": \"%7\"}],"
        "[{\"type\": \"%8\"},{\"type\": \"%9\"}],"
        "[{\"type\": \"%10\"},{\"type\": \"%11\"},{\"type\": \"%12\"}]"
        "]}]";

ScreenConfigFile::ScreenConfigFile()
{
}

ScreenConfigFile::~ScreenConfigFile()
{

}
void ScreenConfigFile::loadFile(bool getDefault, QWidget *parent)
{
    readFile("./Configuration/ScreenConfigs.json", getDefault, parent);
}
bool ScreenConfigFile::dumpFile()
{
    return writeFile("./Configuration/ScreenConfigs.json");
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
void ScreenConfigFile::readFile(QString f, bool getDefault, QWidget *parent)
{
    QString s;
    bool retval = false;
    if (!getDefault)
    {
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
    if (retval == false)
    {
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
            for (auto const &n: namearray)
            {
                SC config;
                QJsonObject namestruct = n.toObject();
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
