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






#ifndef HAMLIBROTCONTROL_H
#define HAMLIBROTCONTROL_H

#include <QObject>
#include <QComboBox>
#include <QStringList>

#include "rotatorcommon.h"
#include "rotcontrol.h"


#include <hamlib/rotator.h>
#include <hamlib/rig.h>         // for debug

bool model_Sort(const rot_caps *caps1,const rot_caps *caps2);
int rig_message_cb(enum rig_debug_level_e, rig_ptr_t, const char*, va_list);



const QStringList errorMsgTxt = {"No Error, operation completed sucessfully",
                                "Invalid parameter",
                                "Invalid configuration",
                                "Memory shortage", "Function not implemented",
                                "Communication timed out",
                                "IO error, including open failed",
                                "Internal Hamlib error",
                                "Protocol error",
                                "Command rejected by the rig",
                                "Command performed, but arg truncated",
                                "Function not available",
                                "VFO not targetable",
                                "Error talking on the bus",
                                "Collision on the bus",
                                "NULL RIG handle or any invalid pointer parameter in get arg",
                                "Invalid VFO",
                                "RIG_EDOM",
                                "Hamlib Init Fail"};









class HamlibRotControl: public RotControl
{
    Q_OBJECT

public:
    explicit HamlibRotControl(QObject *parent);
    ~HamlibRotControl();

    int init(srotParams &currentAntenna);
    int closeRotator();
    int getModelNumber(int idx);
    int getRotatorModelIndex();
    void getRotatorList();
    bool getRotatorList(QComboBox *cb);
    const char * getMfg_Name(int idx);
    const char * getModel_Name(int idx);
    void set_rotatorSpeed(int speed);
    int get_rotatorSpeed();
    void set_serialConnected(bool connectFlag);
    bool get_serialConnected();
    int getRotatorAzimuth();
    int request_bearing();
    int rotate_to_bearing(int bearing);
    int rotateCClockwise(int speed);
    int rotateClockwise(int speed);
    int stop_rotation();
    serialData::serial_parity getSerialParityCode(int index);
    serialData::serial_handshake getSerialHandshakeCode(int index);
    QStringList getParityCodeNames();
    QStringList getHandShakeNames();
    QStringList getBaudRateNames();
    QStringList getDataBitsNames();
    QStringList getStopBitsNames();
    QStringList getErrorMsgList();
    QString getErrorMsgText(int errorCode);
    QString gethamlibVersion();


    int rig_message_cb(enum rig_debug_level_e debug_level, const char *fmt, va_list ap);



    int getSupportCwCcwCmd(int rotNumber, bool *flag);
    int getMaxMinRotation(int rotNumber, int *maxRot, int *minRot);
    int getPortType(int rotNumber, rig_port_e *portType);

    int getModelInfo(QString rotModel, int *rotModelNumber, QString *rotMfgName, QString *rotModelName);

    void enableTraceComms(bool state);

signals:
   void bearing_updated(int);
   void request_bearingError(int);
   void debug_protocol(QString);


private:
    hamlib_port_t myport;
    ROT *my_rot = nullptr;            // handle to rig instance)
    azimuth_t rot_azimuth;  // azimuth from rotator
    //elevation_t rot_elevation; // not used

    int rot_speed = 100;
    //bool rotControlEnabled;
    bool serialConnected;
    //void errorMessage(int errorCode,QString command);
    bool rotatorlistLoaded=false;
    //srotParams curRotParams;   remove
    //int serialP;

    bool traceComms = false;



    //int retcode;		/* generic return code from functions */
    //int exitcode;



};

#endif // HAMLIBROTCONTROL_H
