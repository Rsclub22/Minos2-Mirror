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


#ifndef HAMLIBRIGCONTROL_H
#define HAMLIBRIGCONTROL_H

//#include "hamlibCommon.h"
#include "rigbase.h"
#include "rigfactory.h"
#include <hamlib/rig.h>


bool model_Sort(const rig_caps *caps1,const rig_caps *caps2);
int rig_message_cb(enum rig_debug_level_e, rig_ptr_t, const char*, va_list);

int collect(const rig_caps *caps, void *);



enum serial_force_Lines_e {FORCE_LINE_NONE, FORCE_LINE_OFF, FORCE_LINE_ON};
const serial_force_Lines_e forceLinesCodes[] = {FORCE_LINE_NONE, FORCE_LINE_OFF, FORCE_LINE_ON};

const serial_parity_e parityCodes[] = {RIG_PARITY_NONE, RIG_PARITY_ODD, RIG_PARITY_EVEN, RIG_PARITY_MARK, RIG_PARITY_SPACE};
const serial_handshake_e handshakeCodes[] = { RIG_HANDSHAKE_NONE, RIG_HANDSHAKE_XONXOFF, RIG_HANDSHAKE_HARDWARE };






//class RigFactory;

class HamlibRigControl : public RigBase
{
    Q_OBJECT
    static const char* hamlibErrorMsg[];

public:
    HamlibRigControl(QObject *parent = nullptr);
    virtual ~HamlibRigControl();

    static void register_rigs(RigFactory::Rigs*);
    static bool checkFreqRange(int rigNumber, Frequency freq);

    int rigInit(scatParams &currentRadio, bool useRigCtld) override;
    int closeRig() override;

    void setRigConnected(bool rigConnected_) override;
    bool getRigConnected() override;

    int getFrequency(VFO vfo, Frequency& f) override;
    int setFrequency(Frequency frequency, VFO vfo) override;

    int getMode(VFO vfo, MODE& mode) override;
    int setMode(VFO vfo, MODE mode) override;

    bool supportVolControl(int rigNumber) override;
    int setVolume(VFO vfo, float val) override;
    int getVolume(VFO vfo, float *val) override;

    bool supportSignalStrength(int modelNumber) override;
    int getSignalStrength(VFO vfo, int *value) override;

    QString getRigLibVersion() override;
    QString getLibraryName() override;
    QString getErrorMsgText(int errorCode) override;

    bool supportReadRit(int rigNumber) override;
    bool supportWriteRit(int rigNumber) override;
    int getRit(VFO vfo, ShortFreq &ritfreq) override;
    int setRit(VFO vfo, ShortFreq ritfreq) override;

    bool supportWriteRitState(int rigNumber) override;
    bool supportReadRitState(int rigNumber) override;
    int setRitState(VFO vfo, bool state) override;
    int getRitState(VFO vfo, bool& state) override;
    int clearRit(VFO vfo) override;
    int getMaxRitFreq(int rigNumber) override;

    //int setConfigurationParameter(CONFIG_PARAM cfgparam, QString value) override;
    //int getConfigurationParameter(CONFIG_PARAM cfgparam, QString *value) override;


    static void setTraceCommsFlag(bool value);
    void setTraceComms(bool value) override;
    bool getTraceComms() override;

    int rig_message_cb(enum rig_debug_level_e debug_level, const char *fmt, va_list ap);





    int getVfo(VFO *vfo) override;
    int setVfo(VFO vfo) override;

    bool supportReadVfo(int rigNumber) override;
    bool supportWriteVfo(int rigNumber) override;

    QString convertVfoQStr(vfo_t vfo);



private:


    hamlib_port_t myport;
    RIG *my_rig = nullptr;

    rmode_t rmode;          // read radio mode
    pbwidth_t rwidth;        // read radio rx bw


    MODE mapMode(rmode_t m) const;
    rmode_t mapMode(MODE mode) const;
    rmode_t mapMode(QString mode) const;

    vfo_t hamlibVfoNames[3] = {RIG_VFO_CURR, RIG_VFO_A, RIG_VFO_B};

    serial_parity_e getSerialParityCode(int index){return parityCodes[index];}
    serial_handshake_e getSerialHandshakeCode(int index){return handshakeCodes[index];}
    serial_force_Lines_e getSerialForceLineCode(int index){return forceLinesCodes[index];}

    bool rigConnected;

    setting_t rigHasGetFunc(setting_t);
    static setting_t rigHasGetFunc(int rigNumber, setting_t level);

    setting_t rigHasSetFunc(setting_t);
    static setting_t rigHasSetFunc(int rigNumber, setting_t);


    setting_t rigHasGetLevel(setting_t level);
    static setting_t rigHasGetLevel(int rigNumber, setting_t level);
    setting_t rigHasSetLevel(setting_t level);
    static setting_t rigHasSetLevel(int rigNumber, setting_t level);
    int rigSetLevel(vfo_t vfo, setting_t level, value_t val);
    int rigGetLevel(vfo_t vfo, setting_t level, value_t *val);

    int getSignalStrength(vfo_t vfo, value_t *val);
    static rmode_t convertQStrRmode_t(QString mode);

    pbwidth_t passbandNarrow(rmode_t mode);
    pbwidth_t passbandNormal(rmode_t mode);
    pbwidth_t passbandWide(rmode_t mode);

    VFO convert_Vfo_t_To_VFO(vfo_t vfo);
    vfo_t convert_VFO_to_vfo_t(VFO vfo);

    token_t convertConfigParam(CONFIG_PARAM cfgparam);
};

#endif // HAMLIBRIGCONTROL_H
