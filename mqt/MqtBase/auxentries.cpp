#include "auxentries.h"
#include "QtUtils.h"

QVector <AuxTypeOption> AuxTypeOption::auxoptions = {
    {aeClock, QT_TR_NOOP("Clock"), QT_TR_NOOP("Clock")},
    {aeDXCC, QT_TR_NOOP("DXCC"), QT_TR_NOOP("DXCC")},
    {aeDistrict, QT_TR_NOOP("District"), QT_TR_NOOP("District")},
    {aeMemories, QT_TR_NOOP("Memories"), QT_TR_NOOP("Memories")},
    {aeLocatorMap, QT_TR_NOOP("Locator Map"), QT_TR_NOOP("Locator Map")},
    {aeLocatorTree, QT_TR_NOOP("Locator Tree"), QT_TR_NOOP("Locator Tree")},
    {aeStats, QT_TR_NOOP("Stats"), QT_TR_NOOP("Stats")},
    };
AuxTypeOption::AuxTypeOption(AuxEntryType t, const char *s, const char *hint):QObject(),type(t), s(s), hint(hint)
{

}

AuxTypeOption::AuxTypeOption(const AuxTypeOption &a):QObject()
{
    type = a.type;
    s = a.s;
    hint = a.hint;
}

AuxEntryType AuxTypeOption::getAuxEntryType(QString s)
{
    for(auto const &opt: QASCONST(auxoptions))
    {
        if (tr(opt.s) == s || (opt.s == s))
            return opt.type;
    }
    return aeClock;
}
const char * AuxTypeOption::getRawAuxTypeString(AuxEntryType t)
{
    for(auto const &opt: QASCONST(auxoptions))
    {
        if (opt.type == t)
            return opt.s;
    }
    return getRawAuxTypeString(aeClock);
}


QString AuxTypeOption::getTrAuxTypeString(AuxEntryType t)
{
    return tr(getRawAuxTypeString(t));
}

