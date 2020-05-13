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
#include "OmniWrapper.h"


enum omnirigErrorCode {OMNIRIG_OK = 0, OMNIRIG_NOT_SUPPORTED, OMNIRIG_COM_FAILED_START,
                       OMNIRIG_ONE_FAILED_INITIALISE, OMNIRIG_TWO_FAILED_INITIALISE,
                      OMNIRIG_OFFLINE, OMNIRIG_ONLINE, OMNIRIG_GETFREQ_FAIL, OMINIRIG_COM_EXCEPTION,
                      OMNIRIG_NOTCONFIGURED, OMNIRIG_DISABLED,
                      OMNIRIG_PORTBUSY, OMNIRIG_NOTRESPONDING, OMNIRIG_RIG_NULL};



class OmnirigControl : public RigBase
{
    Q_OBJECT

    static const char* omnirigErrorMsg[];
public:
    enum RigNumber {One = 1, Two};

    OmnirigControl(RigNumber rig_number_, QObject *parent = nullptr);
    virtual ~OmnirigControl();

    static void register_rigs(RigFactory::Rigs*, int, int);



    void setRigConnected(bool rigConnected_){rigConnected = rigConnected_;}
    bool getRigConnected(){return rigConnected;}

    int rigInit(scatParams &currentRadio, bool useRigCtld) override;
    int closeRig() override;

    int getFrequency(VFO vfo, Frequency &) override;
    int setFrequency(Frequency freq, VFO vfo) override;

    int getMode(VFO vfo, MODE &mode) override;
    int setMode(VFO vfo, MODE mode) override;

    int setVolume(VFO vfo, float val) override;
    int getVolume(VFO vfo, float *val) override;

    int getSignalStrength(VFO vfo, int *value) override;

    QString getRigLibVersion() override;
    QString getLibraryName() override;
    QString getErrorMsgText(int errorCode) override;

    int getRit(VFO vfo, ShortFreq &ritfreq) override;
    int setRit(VFO vfo, ShortFreq ritfreq) override;
    int setRitState(VFO vfo, bool state) override;
    int getRitState(VFO vfo, bool& state) override;

    static void setTraceCommsFlag(bool value);
    void setTraceComms(bool value) override;
    bool getTraceComms() override;



    OmniRig::RigParamX map_mode(QString mode);



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

    OmniRig::PortBits* serPort;

    OmniRig::RigStatusX status;
    bool rigConnected;

    static MODE map_mode (OmniRig::RigParamX param);
    static OmniRig::RigParamX map_mode (MODE mode);

    void traceMsg(QString msg);
    int omnirigError(omnirigErrorCode errNum);
    QString convertModeToQString(MODE mode);
    void traceCommsMsg(QString msg);
};

#endif // OMNIRIGCONTROL_H
