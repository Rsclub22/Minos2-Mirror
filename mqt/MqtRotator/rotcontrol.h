/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2016
//
// Hamlib library
//
//
/////////////////////////////////////////////////////////////////////////////






#ifndef ROTCONTROL_H
#define ROTCONTROL_H

#include <QObject>
#include <QComboBox>
#include <QStringList>

#include "rotatorcommon.h"

#include <hamlib/rotator.h>
#include <hamlib/rig.h>         // for debug

bool model_Sort(const rot_caps *caps1,const rot_caps *caps2);
int rig_message_cb(enum rig_debug_level_e, rig_ptr_t, const char*, va_list);




class RotControl: public QObject
{

    Q_OBJECT

public:

    virtual ~RotControl() = default;
    virtual int init(srotParams &currentAntenna) = 0;
    virtual int closeRotator() = 0;

    virtual void getRotatorList() = 0;
    virtual bool getRotatorList(QComboBox *cb) = 0;
    //const char * getMfg_Name(int idx);
    //const char * getModel_Name(int idx);
    virtual void set_rotatorSpeed(int speed) = 0;
    virtual int get_rotatorSpeed() = 0;
    virtual void set_serialConnected(bool connectFlag) = 0;
    virtual bool get_serialConnected() = 0;
    virtual int getRotatorAzimuth() = 0;
    virtual int request_bearing() = 0;
    virtual int rotate_to_bearing(int bearing) = 0;
    virtual int rotateCClockwise(int speed) = 0;
    virtual int rotateClockwise(int speed) = 0;
    virtual int stop_rotation() = 0;
    virtual serialData::serial_parity getSerialParityCode(int index) = 0;
    virtual serialData::serial_handshake getSerialHandshakeCode(int index) = 0;
    virtual QStringList getParityCodeNames() = 0;
    virtual QStringList getHandShakeNames() = 0;
    virtual QStringList getBaudRateNames() = 0;
    virtual QStringList getDataBitsNames() = 0;
    virtual QStringList getStopBitsNames() = 0;
    virtual QStringList gethamlibErrorMsg() = 0;
    virtual QString gethamlibErrorMsg(int errorCode) = 0;
    virtual QString gethamlibVersion() = 0;


    virtual int getSupportCwCcwCmd(int rotNumber, bool *flag) = 0;
    virtual int getMaxMinRotation(int rotNumber, int *maxRot, int *minRot) = 0;
    virtual int getPortType(int rotNumber, rig_port_e *portType) = 0;

    virtual int getModelInfo(QString rotModel, int *rotModelNumber, QString *rotMfgName, QString *rotModelName) = 0;

    virtual void enableTraceComms(bool state) = 0;

/*
signals:
   void bearing_updated(int);
   void request_bearingError(int);
   void debug_protocol(QString);


private:
    hamlib_port_t myport;
    ROT *my_rot = nullptr;            // handle to rig instance)
    azimuth_t rot_azimuth;  // azimuth from rotator
    elevation_t rot_elevation; // not used

    int rot_speed = 100;
    bool rotControlEnabled;
    bool serialConnected;
    void errorMessage(int errorCode,QString command);
    bool rotatorlistLoaded=false;
    //srotParams curRotParams;   remove
    int serialP;

    bool traceComms = false;



    int retcode;		// generic return code from functions
    int exitcode;

*/

};

#endif // ROTCONTROL_H
