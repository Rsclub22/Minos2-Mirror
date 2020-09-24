/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rig Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2016 - 2020
//
//
//
//
/////////////////////////////////////////////////////////////////////////////
#ifndef RIGFACTORY_H
#define RIGFACTORY_H

#include <QObject>
#include "rigcapabilities.h"
#include "rigbase.h"


enum RigId          // supported non-hamlib rig interfaces
{
    NonHamlibBaseId = 10000,
    HamlibRigCtld,
    OmniRigOneId,
    OmniRigTwoId
};



class RigFactory : public QObject
{
    Q_OBJECT
public:

    typedef QMap<QString, RigCapabilities> Rigs;

    explicit RigFactory(bool tracecommFlag, QObject *parent = nullptr);
    ~RigFactory();

    RigBase* createRigs(int rigId);
    Rigs* supported_rigs();
    bool checkForBands(int rigNumber, const Frequency &freq);

    void populateComboRigList(QComboBox* comBox);


signals:

private:
    Rigs rigsList;
    bool checkOmniRigInstalled();


};

#endif // RIGFACTORY_H
