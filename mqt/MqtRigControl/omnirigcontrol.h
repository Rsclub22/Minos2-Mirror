/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Omnirig Rig Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2016 - 2020
//
//
//
//
/////////////////////////////////////////////////////////////////////////////

#ifndef OMNIRIGCONTROL_H
#define OMNIRIGCONTROL_H

#include "rigbase.h"
#include "rigcommon.h"
#include "rigfactory.h"
#include "omniRig.h"

class OmnirigControl : public RigBase
{
    Q_OBJECT
public:
    OmnirigControl(QObject *parent = nullptr);
    virtual ~OmnirigControl();

    static void register_rigs(RigFactory::Rigs*, int, int);

    enum RigNumber {One = 1, Two};


    int rigInit(scatParams &currentRadio, bool useRigCtld) override;
    int closeRig() override;

    int getFrequency(VFO vfo, Frequency &) override;
    int setFrequency(Frequency freq, VFO vfo) override;

    int getMode(VFO vfo, MODE &mode) override;
    int setMode(VFO vfo, MODE mode) override;
    QString convertModeQStr(MODE mode) override;
    MODE convertQStrMode(QString mode) override;

    int setVolume(VFO vfo, float val) override;
    int getVolume(VFO vfo, float *val) override;

    int getSignalStrength(VFO vfo, int *value) override;

    QString getRigLibVersion() override;
    QString getErrorMsgText(int errorCode) override;

    int getRit(VFO vfo, ShortFreq &ritfreq) override;
    int setRit(VFO vfo, ShortFreq ritfreq) override;
    int setRitState(VFO vfo, bool state) override;
    int getRitState(VFO vfo, bool& state) override;

    static void setTraceCommsFlag(bool value);
    void setTraceComms(bool value) override;
    bool getTraceComms() override;



private slots:
    void onHandleVisibleChange();
    void onHandleRigTypeChange(int);
    void onHandleStatusChange(int);
    void onHandleParamsChange(int, int);
    void onHandleCOMException(int, QString, QString, QString);
    void onHandleCustomReply(int, const QVariant &, const QVariant &);


private:

    RigNumber rig_number;

    int omnirigStart ();

    OmniRig::OmniRigX* omni_rig;
    OmniRig::RigX* rig;
    QString rig_type;
    int readable_params;
    int writable_params;

    static MODE map_mode (OmniRig::RigParamX param);
    static OmniRig::RigParamX map_mode (MODE mode);

};

#endif // OMNIRIGCONTROL_H
