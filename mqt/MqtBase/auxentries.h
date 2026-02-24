#ifndef AUXENTRIES_H
#define AUXENTRIES_H

#include <QVector>
#include <QString>
#include <QList>
#include <QObject>

enum AuxEntryType {
    aeClock,
    aeDXCC,
    aeDistrict,
    aeMemories,
    aeLocatorMap,
    aeLocatorTree,
    aeStats
};

class AuxTypeOption:QObject
{
    Q_OBJECT
public:
    static AuxEntryType getAuxEntryType(QString s);
    static QString getTrAuxTypeString(AuxEntryType t);
    static const char *getRawAuxTypeString(AuxEntryType t);

    static QVector <AuxTypeOption> auxoptions;

    AuxTypeOption(AuxEntryType t, const char *s, const char *hint);
    AuxTypeOption(const AuxTypeOption &a);

    AuxEntryType type;
    const char * s;
    const char * hint;
};
#endif // AUXENTRIES_H
