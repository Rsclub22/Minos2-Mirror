/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2016 - 2020
//
//
//
//
/////////////////////////////////////////////////////////////////////////////

#ifndef ROTATORFACTORY_H
#define ROTATORFACTORY_H

#include <QObject>
#include "rotatorcommon.h"
#include "rotcapabilities.h"
#include "rotatorbase.h"



enum RotId      // supported non-hamlib rotator interfaces
{
    NonHamlibBaseId = 9899,
    PSTRotatorId
};


const QString HAMLIB_API = "hamlib";
const QString PSTROTATOR_API = "pstRotator";


class RotatorFactory : public QObject
{
    Q_OBJECT
public:

    typedef   QMap<QString, RotCapabilities> Rotators;


    explicit RotatorFactory(bool tracecommFlag, QObject *parent = nullptr);
    ~RotatorFactory();



    RotatorBase* createRotator(int rotatorId);
    Rotators* supported_rotators();

signals:

private:
    Rotators rotatorsList;



};



#endif // ROTATORFACTORY_H
