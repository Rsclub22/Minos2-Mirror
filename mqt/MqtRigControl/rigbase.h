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





class RigBase : public QObject
{
    Q_OBJECT
public:
    explicit RigBase(QObject *parent = nullptr);

    virtual void setRigConnected(bool rigConnected_) = 0;
    virtual bool getRigConnected() = 0;

    virtual void setTraceComms(bool value) = 0;
    virtual bool getTraceComms() = 0;

    virtual int rigInit(scatParams &currentRadio, bool useRigCtld) = 0;
    virtual int closeRig() = 0;

    virtual int getFrequency(VFO vfo, Frequency&) = 0;
    virtual int setFrequency(Frequency freq, VFO vfo ) = 0;

    virtual int getMode(VFO vfo, MODE& mode) = 0;
    virtual int setMode(VFO vfo, MODE mode) = 0;
    //virtual QString convertModeQStr(MODE mode) = 0;
    //virtual MODE convertQStrMode(QString mode) = 0;

    virtual bool supportVolControl(int rigNumber) = 0;
    virtual int setVolume(VFO vfo, float val) = 0;
    virtual int getVolume(VFO vfo, float *val) = 0;

    virtual bool supportReadRit(int rigModelNumber) = 0;
    virtual bool supportWriteRit(int rigModelNumber) = 0;
    virtual int getRit(VFO vfo, ShortFreq &ritfreq) = 0;
    virtual int setRit(VFO vfo, ShortFreq ritfreq) = 0;
    virtual int getMaxRitFreq(int rigNumber) = 0;

    virtual bool supportReadRitState(int rigModelNumber) = 0;
    virtual bool supportWriteRitState(int rigModelNumber) = 0;
    virtual int setRitState(VFO vfo, bool state) = 0;
    virtual int getRitState(VFO vfo, bool& state) = 0;

    virtual int clearRit(VFO vfo) = 0;

    virtual bool supportReadVfo(int rigModelNumber) = 0;
    virtual bool supportWriteVfo(int rigModelNumber) = 0;

    virtual int setVfo(VFO vfo) = 0;
    virtual int getVfo(VFO *vfo) = 0;


    virtual bool supportSignalStrength(int rigModelNumber) = 0;
    virtual int getSignalStrength(VFO vfo, int *value) = 0;

    virtual int setConfigurationParameter(int rigNumber, QString cfgparam, QString value) = 0;
    virtual int getConfigurationParameter(int rigNumber, QString cfgparam, QString *value) = 0;


    virtual QString getRigLibVersion() = 0;
    virtual QString getLibraryName() = 0;
    virtual QString getErrorMsgText(int errorCode) = 0;

    //virtual bool checkFreqRange(int rigNumber, Frequency freq, MODE mode) = 0;




signals:

    // no polling signals
    void newRxFreq(quint64 f);
    void newMode();
    void newVfo(QString);
    void ritOn();
    void ritOff();
    void ritOffset();
    void rit0();
    //void comError(QString, QString, QString, QString);
    void rigStatus(int, QString);


private:

    //bool rigConnected;

};

#endif // RIGBASE_H
