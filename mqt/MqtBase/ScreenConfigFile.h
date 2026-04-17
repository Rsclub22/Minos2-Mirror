#ifndef SCREENCONFIGFILE_H
#define SCREENCONFIGFILE_H
#include <QCoreApplication>
#include <QSharedPointer>
#include <QMap>
#include "auxentries.h"
#include "screenconfigtypes.h"
//#include "StackedInfoFrame.h"

class ScreenConfigScreen;

class SCElement;
class SCRow
{
public:
    QVector<SCElement> elements;
};

// or does ths derive from SC?
class SCScreen
{
public:
    bool mainScreen;
    QString name;
    QSharedPointer<SCElement> baseElement;
};

// we need to make the top level one of these...
// split/addabove(below) produce different splitter actions
// can we colour them differently?

// once we have a split element can we add to the right(left) of all of it?

class ScreenConfigElement;
class SCElement
{
public:
    SCType type = sctNone;
    AuxEntryType auxType = aeClock;
    int fontSize = 100;
    QVector<SCRow> rows;
    QVector<SCScreen> screens;
};

class SC
{
public:
    QString name;
    QSharedPointer<SCElement> baseElement;
};

class ScreenConfigFile
{
    Q_DECLARE_TR_FUNCTIONS(ScreenConfigFile)

    static ScreenConfigFile scf;
    static QString cfileName;

    bool loaded = false;

    static QString defaultConfig;
    static QString protectedConfig;

public:

    static ScreenConfigFile &getScreenConfigFile(QWidget *p)
    {
        if (!scf.loaded)
        {
            scf.loadFile(p);
        }
        return scf;
    }
    static void setFileName(QString c);
    ScreenConfigFile();
    ~ScreenConfigFile();

    static void setDefProt(QString d, QString prot);
    void loadFile(QWidget *parent);
    bool dumpFile();

    QMap <QString, SC> configs;
    
    void getScreenConfig(const SCScreen &scb, QJsonObject &scr);

private:
    void readFile(QString s, QWidget *parent);
    bool writeFile(QString s);
    void procScreens(QVector<SCScreen> &elescr, QJsonArray &screens);
    void procSingleScreen(SC &config, QJsonValue &base);
    void procRows(QVector<SCRow> &elerows, QJsonArray &rows);
    void writeTypetoRow(SCElement &e, QJsonArray &scrow);
    bool parseConfigString(QString s);
};

#endif // SCREENCONFIGFILE_H
