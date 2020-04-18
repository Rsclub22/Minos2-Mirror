/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2020
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
#include "rotatorfactory.h"
#include "rotatorbase.h"
//#include "hamlibCommon.h"

#include <hamlib/rotator.h>
#include <hamlib/rig.h>         // for debug

bool model_Sort(const rot_caps *caps1,const rot_caps *caps2);
int rig_message_cb(enum rig_debug_level_e, rig_ptr_t, const char*, va_list);

int collect(const rot_caps *caps, void *);



enum serial_force_Lines_e {FORCE_LINE_NONE, FORCE_LINE_OFF, FORCE_LINE_ON};
const serial_force_Lines_e forceLinesCodes[] = {FORCE_LINE_NONE, FORCE_LINE_OFF, FORCE_LINE_ON};

const serial_parity_e parityCodes[] = {RIG_PARITY_NONE, RIG_PARITY_ODD, RIG_PARITY_EVEN, RIG_PARITY_MARK, RIG_PARITY_SPACE};
const serial_handshake_e handshakeCodes[] = { RIG_HANDSHAKE_NONE, RIG_HANDSHAKE_XONXOFF, RIG_HANDSHAKE_HARDWARE };





class HamlibRotControl: public RotatorBase
{
    Q_OBJECT

    static const char* hamlibErrorMsg[];

public:
    explicit HamlibRotControl(QObject *parent = nullptr);
    virtual ~HamlibRotControl();

    static void register_rotators(RotatorFactory::Rotators *);


    int getModelNumber(int idx);
    int getRotatorModelIndex();
    //void getRotatorList();
    //bool getRotatorList(QComboBox *cb);
    const char * getMfg_Name(int idx);
    const char * getModel_Name(int idx);

    int rotInit(srotParams &currentAntenna) override;
    int closeRotator() override;

    int request_bearing() override;
    int rotate_to_bearing(const int bearing) override;
    int rotateCClockwise(const int speed) override;
    int rotateClockwise(const int speed)  override;
    int stop_rotation() override;

    void set_rotatorSpeed(int speed)  override;
    int get_rotatorSpeed()  override;

    QStringList getErrorMsgList();
    QString getErrorMsgText(int errorCode) override;
    QString getLibraryName() override;
    QString getRotLibVersion() override;

    static void setTraceCommsFlag(bool value);
    void setTraceComms(bool value) override;
    bool getTraceComms() override;

    int rig_message_cb(enum rig_debug_level_e debug_level, const char *fmt, va_list ap);



    //int getSupportCwCcwCmd(int rotNumber, bool *flag);
    //int getMaxMinRotation(int rotNumber, int *maxRot, int *minRot);
    int getPortType(int rotNumber, rig_port_e *portType);

    int getModelInfo(QString rotModel, int *rotModelNumber, QString *rotMfgName, QString *rotModelName);


private:



signals:
   //void bearing_updated(int);
   //void request_bearingError(int);
   void debug_protocol(QString);


private:
    hamlib_port_t myport;
    ROT *my_rot = nullptr;            // handle to rig instance)
    azimuth_t rot_azimuth;  // azimuth from rotator
    elevation_t rot_elevation; // not used

    int rot_speed = 100;
    //bool rotControlEnabled;
    //bool serialConnected;
    //void errorMessage(int errorCode,QString command);
    //bool rotatorlistLoaded=false;
    //srotParams curRotParams;   remove
    //int serialP;


    serial_parity_e getSerialParityCode(int index){return parityCodes[index];}
    serial_handshake_e getSerialHandshakeCode(int index){return handshakeCodes[index];}
    serial_force_Lines_e getSerialForceLineCode(int index){return forceLinesCodes[index];}



    //int retcode;		/* generic return code from functions */
    //int exitcode;




};

#endif // HAMLIBROTCONTROL_H
