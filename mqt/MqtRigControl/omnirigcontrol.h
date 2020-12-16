
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

    void setRigConnected(bool rigConnected_) override;
    bool getRigConnected() override;

    int rigInit(scatParams &currentRadio, bool useRigCtld) override;
    int closeRig() override;

    int getFrequency(VFO vfo, Frequency &) override;
    int setFrequency(const Frequency &freq, VFO vfo) override;

    int getMode(VFO vfo, MODE &mode) override;
    int setMode(VFO vfo, MODE mode) override;

    bool supportVolControl(int rigNumber) override;
    int setVolume(VFO vfo, float val) override;
    int getVolume(VFO vfo, float *val) override;

    bool supportSignalStrength(int modelNumber) override;
    int getSignalStrength(VFO vfo, int *value) override;

    QString getRigLibVersion() override;
    QString getLibraryName() override;
    QString getErrorMsgText(int errorCode) override;

    bool supportReadRit(int rigModelNumber) override;
    bool supportWriteRit(int rigModelNumber) override;
    int getRit(VFO vfo, ShortFreq &ritfreq) override;
    int setRit(VFO vfo, const ShortFreq &ritfreq) override;
    int getMaxRitFreq(int rigNumber) override;

    bool supportReadRitState(int rigModelNumber) override;
    bool supportWriteRitState(int rigModelNumber) override;
    int setRitState(VFO vfo, bool state) override;
    int getRitState(VFO vfo, bool& state) override;
    int clearRit(VFO vfo) override;

    static void setTraceCommsFlag(bool value);
    void setTraceComms(bool value) override;
    bool getTraceComms() override;

    bool supportReadVfo(int rigNumber) override;
    bool supportWriteVfo(int rigNumber) override;
    int setVfo(VFO vfo) override;
    int getVfo(VFO *vfo) override;

    int setConfigurationParameter(QString cfgparam, QString value) override;
    int getConfigurationParameter(QString cfgparam, QString *value) override;

    int sendVoiceMessage(VFO vfo, int vmNum) override;
    bool supportVoiceMemory() override;

    int sendMorse(VFO vfo, QString msg) override;
    int stopMorse(VFO vfo) override;
    //int waitMorsePtt(VFO vfo) override;
    int waitMorse(VFO vfo) override;
    bool supportCwMemory() override;

    int getPttStatus(VFO vfo, bool &state) override;
    int setPtt(VFO vfo, bool state) override;




#if defined (WIN32)
    OmniRig::RigParamX map_mode(QString mode);

#endif




private slots:

    void onHandleVisibleChange();
    void onHandleRigTypeChange(int);
    void onHandleStatusChange(int);
    void onHandleParamsChange(int, int);
    void onHandleCOMException(int, QString, QString, QString);
    void onHandleCustomReply(int, const QVariant &, const QVariant &);


    void onOffLineTimeout();
private:


    RigNumber rig_number;

    int omnirigStart ();

    OmniRig::OmniRigX* omni_rig;
    OmniRig::RigX* rig;
    QString rig_type;
    int readable_params;
    int writable_params;

    VFO curVfo;
    bool reversedVFO;

    OmniRig::PortBits* serPort;

    OmniRig::RigStatusX status;
    bool rigConnected;
    QTimer* offlineTimer;

    static MODE map_mode (OmniRig::RigParamX param);
    static OmniRig::RigParamX map_mode (MODE mode);

    void traceMsg(QString msg);
    int omnirigError(omnirigErrorCode errNum);
    QString convertModeToQString(MODE mode);
    void traceCommsMsg(QString msg);
    bool supportReadRit();
    bool supportWriteRit();



};

#endif // OMNIRIGCONTROL_H
