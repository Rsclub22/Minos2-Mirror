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

#include "rigbase.h"
#include "rigfactory.h"
#include <hamlib/rig.h>


bool model_Sort(const rig_caps *caps1,const rig_caps *caps2);
int rig_message_cb(enum rig_debug_level_e, rig_ptr_t, const char*, va_list);

int collect(const rig_caps *caps, void *);







class HamlibRigControl : public RigBase
{
    Q_OBJECT
public:
    HamlibRigControl(QObject *parent = nullptr);
    virtual ~HamlibRigControl();

    static void register_rigs(RigFactory::Rigs *);

    int rigInit(scatParams &currentRadio, bool useRigCtld);
    int getFrequency(VFO vfo, Frequency*);
    int setFrequency(Frequency frequency, VFO vfo);

    int getMode(VFO vfo, MODE *mode);
    int setMode(VFO vfo, MODE mode);



    QString getRotLibVersion() override;

    static void setTraceCommsFlag(bool value);
    void setTraceComms(bool value) override;
    bool getTraceComms() override;

    int rig_message_cb(enum rig_debug_level_e debug_level, const char *fmt, va_list ap);



};

#endif // HAMLIBRIGCONTROL_H
