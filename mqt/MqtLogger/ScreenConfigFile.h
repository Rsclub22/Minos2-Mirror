#ifndef SCREENCONFIGFILE_H
#define SCREENCONFIGFILE_H
#include "base_pch.h"

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
    sctNone
};
class SCElement
{
public:
    SCType type;
};

class SCRow
{
public:
    QVector<SCElement> elements;
};

class SC
{
public:
    QString name;
    QVector<SCRow> rows;
};
class ScreenConfigFile
{
public:
    static const QString defaultLayoutName;

    ScreenConfigFile();
    bool loadFile();
    bool dumpFile();

    QMap <QString, SC> configs;
private:
    bool readFile(QString s);
    bool writeFile(QString s);
};

#endif // SCREENCONFIGFILE_H
