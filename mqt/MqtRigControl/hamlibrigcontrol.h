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

#include "hamlibCommon.h"
#include "rigbase.h"
#include "rigfactory.h"
#include <hamlib/rig.h>


bool model_Sort(const rig_caps *caps1,const rig_caps *caps2);
int rig_message_cb(enum rig_debug_level_e, rig_ptr_t, const char*, va_list);

int collect(const rig_caps *caps, void *);








class RigFactory;

class HamlibRigControl : public RigBase
{
    Q_OBJECT
public:
    HamlibRigControl(QObject *parent = nullptr);
    virtual ~HamlibRigControl();

    static void register_rigs(RigFactory::Rigs*);

    int rigInit(scatParams &currentRadio, bool useRigCtld) override;
    int closeRig() override;

    int getFrequency(VFO vfo, Frequency& f) override;
    int setFrequency(Frequency frequency, VFO vfo) override;

    int getMode(VFO vfo, MODE& mode) override;
    int setMode(VFO vfo, MODE mode) override;
    QString convertModeQStr(MODE mode) override;
    MODE convertQStrMode(QString mode) override;


    QString getRigLibVersion() override;
    QString getErrorMsgText(int errorCode) override;

    static void setTraceCommsFlag(bool value);
    void setTraceComms(bool value) override;
    bool getTraceComms() override;

    int rig_message_cb(enum rig_debug_level_e debug_level, const char *fmt, va_list ap);

private:

    hamlib_port_t myport;
    RIG *my_rig = nullptr;

    rmode_t rmode;          // read radio mode
    pbwidth_t rwidth;        // read radio rx bw


    MODE mapMode(rmode_t m) const;
    rmode_t mapMode(MODE mode) const;

    vfo_t hamlibVfoNames[3] = {RIG_VFO_CURR, RIG_VFO_A, RIG_VFO_B};

    serial_parity_e getSerialParityCode(int index){return hamlibSerialData::parityCodes[index];}
    serial_handshake_e getSerialHandshakeCode(int index){return hamlibSerialData::handshakeCodes[index];}
    hamlibSerialData::serial_force_Lines_e getSerialForceLineCode(int index){return hamlibSerialData::forceLinesCodes[index];}



};

#endif // HAMLIBRIGCONTROL_H
