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


#ifndef ROTATORBASE_H
#define ROTATORBASE_H

#include <QObject>
#include "rotatorcommon.h"



class RotatorBase : public QObject
{
    Q_OBJECT
public:
    explicit RotatorBase(QObject *parent = nullptr);

    void setRotAzimuth(int rot_azimuth_){rot_azimuth = rot_azimuth_;}
    int getRotAzimuth(){return rot_azimuth;}

    virtual void setRotConnected(bool rotConnected) = 0;
    virtual bool getRotConnected() = 0;

    virtual void setTraceComms(bool value) = 0;
    virtual bool getTraceComms() = 0;

    virtual int rotInit(srotParams &selectedAntenna) = 0;

    virtual int request_bearing() = 0;
    virtual int rotate_to_bearing(const int bearing) = 0;
    virtual int rotateCClockwise(const int speed) = 0;
    virtual int rotateClockwise(const int speed) = 0;
    virtual int stop_rotation() = 0;
    virtual int closeRotator() = 0;
    virtual void set_rotatorSpeed(int speed) = 0;
    virtual int get_rotatorSpeed() = 0;

    virtual QString getRotLibVersion() = 0;
    virtual QString getLibraryName() = 0;
    virtual QString getErrorMsgText(int errorCode) = 0;


//protected:


signals:
   void bearing_updated(int);
   void sentCommandError(int, QString);
   void traceCommsMsg(QString);

private:

    int rot_azimuth;
    bool rotConnected;


};

#endif // ROTATORBASE_H
