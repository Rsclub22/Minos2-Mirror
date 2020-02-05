/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2016 - 2020
//
//
// PSTRotator
//
/////////////////////////////////////////////////////////////////////////////


#ifndef PSTROTCONTROL_H
#define PSTROTCONTROL_H

#include <QObject>
#include <QUdpSocket>
#include <QNetworkDatagram>
#include <QDebug>
#include "rotatorcommon.h"
#include "rotatorfactory.h"
#include "rotatorbase.h"


class PstRotControl : public RotatorBase
{
    Q_OBJECT
public:
    explicit PstRotControl(QObject *parent = nullptr);
    virtual ~PstRotControl();

    static void register_rotators(RotatorFactory *rotators, int rotatorId);
    int rotInit(srotParams &selectedAntenna) override;
    int request_bearing() override;
    int rotate_to_bearing(const int bearing) override;
    int stop_rotation() override;

    // these rotate methods are unused
    int rotateCClockwise(const int speed) override;
    int rotateClockwise(const int speed) override;

    void set_rotatorSpeed(int speed)  override;
    int get_rotatorSpeed()  override;

    int closeRotator() override;

    void enableTraceComms(bool state ) override;
    QString getRotLibVersion() override;

    void setPstNetAddress(QString address);
    void setPstPortAddress(QString port);



signals:
    //void pstBearing(int);

public slots:

    void processPendingReportDatagrams();

private:

    void sendCommandToPstRotator(const QString msg);
    void closeSockets();

    QString pstNetAddress;
    QHostAddress pstAddress;
    QUdpSocket* pstCommandSocket;
    unsigned short pstCommandPortNumber;


    QUdpSocket* pstReportSocket;
    unsigned short pstReportPortNumber;

    QString bearing;
    bool traceComms;

    int rot_speed = 100;



};

#endif // PSTROTCONTROL_H
