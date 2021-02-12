/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rig Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2017
//
// Interprocess Control Logic
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2017
//
//
//
/////////////////////////////////////////////////////////////////////////////

#ifndef RIGCONTROLRPC_H
#define RIGCONTROLRPC_H

#include "RigCache.h"

#include "base_pch.h"

class RigControlMainWindow;

class RigControlRpc : public QObject
{
    Q_OBJECT

    RigControlMainWindow *parent;

public:
    explicit RigControlRpc(RigControlMainWindow *parent);

    RigCache rigCache;

    void publishRadioNames(QStringList radios);

signals:
    void setFreq(Frequency);
    void setBand(QString);
    void setMode(QString);
    void selectLoggerRadio(PubSubName, QString, Frequency, QString);
    void setRitFreq(ShortFreq);
    void setRitStatus(bool);
    void setVolume(int volLevel);
    void setVoiceMessageNum(QString msgNum);

private slots:
    void on_notify(bool err, QSharedPointer<MinosRPCObj> mro, const QString &from );
    void on_serverCall( bool err, QSharedPointer<MinosRPCObj>mro, const QString &from );

};

#endif // RIGCONTROLRPC_H
