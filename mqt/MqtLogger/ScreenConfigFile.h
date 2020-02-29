#ifndef SCREENCONFIGFILE_H
#define SCREENCONFIGFILE_H
#include "base_pch.h"
#include "StackedInfoFrame.h"

enum SCType
{
    sctAux,
    sctLog,
    sctRigControl,
    sctRotControl,
    sctRotPresets,
    sctQSOEdit,
    sctNextQSODetails,
    sctThisMatch,
    sctOtherMatch,
    sctArchiveMatch,
    sctChat,
    sctCluster,
    sctWsjtx,
    sctBandmap,
	sctSplit,
    sctNone
};
class SCElement;
class SCRow
{
public:
    QVector<SCElement> elements;
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
    AuxEntries auxType = aeClock;
    QVector<SCRow> rows;
};

class SC
{
public:
    QString name;
    QSharedPointer<SCElement> baseElement;
};

class ScreenConfigFile
{
    Q_DECLARE_TR_FUNCTIONS(ScreenConfigFie)
public:

    ScreenConfigFile();
    ~ScreenConfigFile();
    void loadFile(bool getDefault, QWidget *parent);
    bool dumpFile();

    QMap <QString, SC> configs;
    
private:
    void readFile(QString s, bool getDefault, QWidget *parent);
    bool writeFile(QString s);
    void procRows(QVector<SCRow> &elerows, QJsonArray &rows);
    void writeTypetoRow(SCElement &e, QJsonArray &scrow);
    bool parseConfigString(QString s);
};

#endif // SCREENCONFIGFILE_H
