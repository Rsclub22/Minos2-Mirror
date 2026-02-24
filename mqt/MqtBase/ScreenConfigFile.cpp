
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include "QtUtils.h"
#include "AppStartup.h"
#include "MShowMessageDlg.h"
#include "MTrace.h"
#include "auxentries.h"
#include "ScreenConfigFile.h"
#include "ScreenConfigElement.h"
QString ScreenConfigFile::defaultConfig;

QString ScreenConfigFile::protectedConfig;

ScreenConfigFile ScreenConfigFile::scf;
QString ScreenConfigFile::cfileName;
void ScreenConfigFile::setFileName(QString c)
{
    cfileName = c;
}


ScreenConfigFile::ScreenConfigFile()
{
}

ScreenConfigFile::~ScreenConfigFile()
{

}
void ScreenConfigFile::setDefProt(QString d, QString prot)
{
    defaultConfig = d;
    protectedConfig = prot;
}

void ScreenConfigFile::loadFile(QWidget *parent)
{
    readFile(getDirectoryLocation(dlConfiguration) + "/" + cfileName, parent);
    loaded = true;
}
bool ScreenConfigFile::dumpFile()
{
    return writeFile(getDirectoryLocation(dlConfiguration) + "/" + cfileName);
}
void ScreenConfigFile::procScreens(QVector<SCScreen> &elescr, QJsonArray &screens)
{
    for (auto const &s: screens)
    {
        SCScreen screen;

        QJsonValue mainScreenObject = s.toObject().value("mainscreen");
        if ( mainScreenObject != QJsonValue::Undefined)
        {
            screen.baseElement = QSharedPointer<SCElement>(new SCElement());
            screen.baseElement->type = sctMainScreen;
            screen.name = mainScreenObject.toString();
            screen.mainScreen = true;
        }
        else
        {
            QJsonValue screenObject = s.toObject().value("screen");
            if ( screenObject != QJsonValue::Undefined)
            {
                screen.baseElement = QSharedPointer<SCElement>(new SCElement());
                screen.baseElement->type = sctScreen;
                screen.name = screenObject.toString();
                screen.mainScreen = false;
            }
        }

        if (!screen.baseElement.isNull())
        {
            QJsonValue rowsObject = s.toObject().value("rows");
            if ( rowsObject != QJsonValue::Undefined)
            {
                QJsonArray rows = rowsObject.toArray();
                procRows(screen.baseElement->rows, rows);
            }
            elescr.push_back(screen);
        }
    }
}
void ScreenConfigFile::procSingleScreen(SC &config, QJsonValue &base)
{
    // Wrap an old style config in new style mainScreen
    config.baseElement->screens.append(SCScreen());

    SCScreen &scs = config.baseElement->screens[0] ;
    scs.baseElement = QSharedPointer<SCElement>(new SCElement());
    scs.baseElement->type = sctMainScreen;
    scs.mainScreen = true;

    QJsonArray rows = base.toArray();
    procRows(scs.baseElement->rows, rows);
}

void ScreenConfigFile::procRows(QVector<SCRow> &elerows, QJsonArray &rows)
{
    for (auto const &r: rows)
    {
        SCRow scrow;
        QJsonArray elearray = r.toArray();
        for (auto const &e: QASCONST(elearray))
        {
            SCElement scele;
            QJsonObject ele = e.toObject();
            QString eletype = ele.value("type").toString();
            if (eletype == "TX Voice Mem. Buttons" || eletype == tr("TX Voice Mem. Buttons"))
            {
                eletype = ScreenConfigElement::getRawScreenTypeString(sctTxVmButtons);
            }
            scele.type = ScreenConfigElement::getScreenType(eletype);
            if (scele.type == sctSplit)
            {
                QJsonArray rows = ele.value("rows").toArray();
                procRows(scele.rows, rows);
            }
            else if (scele.type == sctAux)
            {
                QString auxtype = ele.value("auxtype").toString();
                scele.auxType = AuxTypeOption::getAuxEntryType(auxtype);
            }
            else
            {
                scele.auxType = aeClock;
            }
            scrow.elements.push_back(scele);
        }
        elerows.push_back(scrow);
    }
}
void ScreenConfigFile::readFile(QString f, QWidget *parent)
{
    configs.clear();

    trace("Using default configuration");
    parseConfigString(defaultConfig);

    trace("Using default protectected configuration");
    parseConfigString(protectedConfig);

    bool retval = false;

    QFile jf(f);
    if (jf.open(QIODevice::ReadOnly))
    {
        QString s = jf.readAll();
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
            for (auto const &n: QASCONST(namearray))
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
        scele.insert("auxtype", AuxTypeOption::getRawAuxTypeString(e.auxType));
    }
    scrow.append(scele);
}
void ScreenConfigFile::getScreenConfig(const SCScreen &scb, QJsonObject &scr)
{
    QJsonArray scrows;
    for (auto &j: scb.baseElement->rows)
    {
       QJsonArray scrow;
       for (auto &k: j.elements)
       {
           writeTypetoRow(k, scrow);
       }
       scrows.append(scrow);
    }
    scr.insert("rows", scrows);
}

bool ScreenConfigFile::writeFile(QString f)
{
     QJsonDocument json;
     QJsonArray scarray;

     for(auto &i: configs )
     {
         QJsonObject sc;
         QJsonArray screens;
         QString name = i.name;
         SC &scb = i;
         sc.insert("name", name);

         if (i.baseElement->screens.count() == 1)
         {
             // for single screen, keep old format
             getScreenConfig(scb.baseElement->screens[0], sc);
         }
         else
         {
             bool ms = true;
             for (auto &screen:i.baseElement->screens)
             {
                 QJsonObject scr;
                 if (ms)
                 {
                    scr.insert("mainscreen", screen.name);
                    ms = false;
                 }
                 else
                 {
                     scr.insert("screen", screen.name);
                 }
                 getScreenConfig(screen, scr);

                 screens.append(scr);

             }
             sc.insert("screens", screens);
         }

         scarray.append(sc);
     }
     json.setArray(scarray);

     QByteArray s = json.toJson();

     QFile jf(f);
     if (!jf.open(QIODevice::WriteOnly | QIODevice::Truncate))
     {
         trace("Failed to open " + f );
         return false;
     }

     jf.write(s);

     jf.close();

     return true;
}

