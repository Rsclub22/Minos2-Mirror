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





enum pstErrorCode {PST_OK = 0, BIND_FAILURE = -1, DATAGRAM_WRITE_ERROR = -2, REQ_BEARING_CMD_TIMEOUT = -3, ROT_TO_BEARING_CMD_TIMEOUT = -4, STOP_CMD_TIMEOUT = -5};

enum pstCmdSent {NO_CMDSENT = 0, REQ_BEARING_CMDSENT, ROT_TO_BEARING_CMDSENT, STOP_ROTATION_CMDSENT};

const pstErrorCode cmdSentErrorCodes[] = {PST_OK, REQ_BEARING_CMD_TIMEOUT, ROT_TO_BEARING_CMD_TIMEOUT, STOP_CMD_TIMEOUT};
const QString cmdSentString[] = {"Request Bearing Command", "Rotate To Bearing Command", "Stop Command"};
const int timeoutDur = 2000;


class PstRotControl : public RotatorBase
{
    Q_OBJECT

    static const char* pstRotatorErrorMsg[];
public:
    explicit PstRotControl(QObject *parent = nullptr);
    virtual ~PstRotControl();

    static void register_rotators(RotatorFactory::Rotators *rotatorsList, int rotatorId);
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


    void setTraceComms(bool value) override;
    bool getTraceComms() override;

    QString getRotLibVersion() override;
    QString getErrorMsgText(int errorCode) override;
    QString getLibraryName() override;

    void setPstNetAddress(QString address);
    void setPstPortAddress(QString port);



signals:
    //void pstBearing(int);

public slots:

    void processPendingReportDatagrams();

private slots:
    void onCommsTimeout();
private:

    int sendCommandToPstRotator(const QString msg);
    void closeSockets();

    QString pstNetAddress;
    QHostAddress pstAddress;
    QUdpSocket* pstCommandSocket;
    unsigned short pstCommandPortNumber;


    QUdpSocket* pstReportSocket;
    unsigned short pstReportPortNumber;

    QTimer *commsTimeoutTimer;
    QString bearing;
    bool traceCommsFlag;

    pstCmdSent cmdSent;


    int rot_speed = 100;



    void traceMsg(QString msg);
    void traceCommsMsg(QString msg);

};

#endif // PSTROTCONTROL_H
