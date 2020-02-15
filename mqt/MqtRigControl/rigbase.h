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

#ifndef RIGBASE_H
#define RIGBASE_H

#include <QObject>
#include "rigcommon.h"

enum MODE {UNK, CW, CW_R, USB, LSB, FSK, FSK_R, DIG_U, DIG_L, AM, FM, DIG_FM};
enum VFO {CURRENT_VFO, VFOA, VFOB};
using  Frequency = quint64;





class RigBase : public QObject
{
    Q_OBJECT
public:
    explicit RigBase(QObject *parent = nullptr);

    void setRigConnected(bool rigConnected_){rigConnected = rigConnected_;}
    bool getRigConnected(){return rigConnected;}

    virtual void setTraceComms(bool value) = 0;
    virtual bool getTraceComms() = 0;

    virtual int rigInit(scatParams &currentRadio, bool useRigCtld) = 0;
    virtual int getFrequency(VFO vfo, Frequency*) = 0;
    virtual int setFrequency(Frequency freq, VFO vfo ) = 0;

    virtual int getMode(VFO vfo, MODE* mode) = 0;
    virtual int setMode(VFO vfo, MODE mode) = 0;

    virtual QString getRotLibVersion() = 0;


signals:


private:

    bool rigConnected;

};

#endif // RIGBASE_H
