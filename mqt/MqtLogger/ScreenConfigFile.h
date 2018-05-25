#ifndef SCREENCONFIGFILE_H
#define SCREENCONFIGFILE_H
#include "base_pch.h"

enum SCType
{
    sctNone,
    sctAux,
    sctLog,
    sctRigControl,
    sctRotControl,
    sctQSOEdit,
    sctThisMatch,
    sctOtherMatch,
    sctArchiveMatch,
    sctChat
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
class ScreenConfigFile
{
public:
    ScreenConfigFile();

    QVector <SCRow> rows;
};

#endif // SCREENCONFIGFILE_H
