/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2016
//
//
/////////////////////////////////////////////////////////////////////////////




#ifndef ROTATORRPC_H
#define ROTATORRPC_H

#include "base_pch.h"
#include "RotatorCache.h"

class RotatorMainWindow;

class RotatorRpc : public QObject
{
    Q_OBJECT

    RotatorMainWindow *parent;
public:
    explicit RotatorRpc(RotatorMainWindow *parent);

    RotatorCache rotatorCache;

    void publishAntennaList(QString ants);

    void publishPresetList(QString presets);
signals:
    void setRotation(int direction, int angle);
    void selectAntennaFromLog(PubSubName);
    void setRotPreset(QString);

private slots:
    void on_notify(AnalysePubSubNotify an, const QString from );
    void on_routerCall( bool err, QSharedPointer<MinosRPCObj>mro, const QString from );
};

#endif // ROTATORRPC_H
