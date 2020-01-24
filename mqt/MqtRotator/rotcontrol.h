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

namespace serialData
{
const serial_parity_e parityCodes[] = {RIG_PARITY_NONE, RIG_PARITY_ODD, RIG_PARITY_EVEN, RIG_PARITY_MARK, RIG_PARITY_SPACE};
const serial_handshake_e handshakeCodes[] = { RIG_HANDSHAKE_NONE, RIG_HANDSHAKE_XONXOFF, RIG_HANDSHAKE_HARDWARE };
const QStringList parityStr = (QStringList() << "None" << "Odd" << "Even" << "Mark" << "Space");
const QStringList handshakeStr = (QStringList() << "None" << "XON/XOFF" << "CTS/RTS");
const QStringList forceLinesStr = QStringList() << "" << "High" << "Low";
const QStringList baudrateStr = (QStringList() << "" << "1200" << "2400" << "4800" << "9600" << "19200" << "38400" << "57600" << "115200");
const QStringList databitsStr = (QStringList() << "" << "7" << "8" );
const QStringList stopbitsStr = (QStringList()<< "" << "1" << "2");

}





class srotParams
{
public:


  static void copyRot(srotParams* srce, srotParams &dest)
  {

      dest.antennaName = srce->antennaName;
      dest.antennaNumber = srce->antennaNumber;
      dest.configLabel = srce->configLabel;
      dest.comport = srce->comport;
      dest.rotatorModel = srce->rotatorModel;
      dest.rotatorManufacturer = srce->rotatorManufacturer;
      dest.rotatorModelName = srce->rotatorModelName;
      dest.rotatorModelNumber = srce->rotatorModelNumber;
      dest.pollInterval = srce->pollInterval;
      dest.min_azimuth = srce->min_azimuth;
      dest.max_azimuth = srce->max_azimuth;
      dest.endStopType = srce->endStopType;
      dest.rotatorCWEndStop = srce->rotatorCWEndStop;
      dest.rotatorCCWEndStop = srce->rotatorCCWEndStop;
      dest.rotType = srce->rotType;
      dest.min_elevation = srce->min_elevation;
      dest.max_elevation = srce->max_elevation;
      dest.southStopType = srce->southStopType;
      dest.overRunFlag = srce->overRunFlag;
      dest.supportCwCcwCmd = srce->supportCwCcwCmd;
      dest.simCwCcwCmd = srce->simCwCcwCmd;
      dest.antennaOffset = srce->antennaOffset;
      dest.moving = srce->moving;
      dest.portType = srce->portType;
      dest.networkAdd = srce->networkAdd;
      dest.networkPort = srce->networkPort;
      dest.maxBaudRate = srce->maxBaudRate;
      dest.minBaudRate = srce->minBaudRate;
      dest.baudrate = srce->baudrate;
      dest.parity = srce->parity;
      dest.stopbits = srce->stopbits;
      dest.databits = srce->databits;
      dest.handshake = srce->handshake;
      dest.enableRot = srce->enableRot;
      dest.activeRTS = srce->activeRTS;
      dest.activeDTR = srce->activeDTR;
      dest.nactiveRTS = srce->nactiveRTS;
      dest.nactiveDTR = srce->nactiveDTR;

  }


  QString antennaName;
  QString antennaNumber;
  QString configLabel;
  QString comport;
  QString rotatorModel;
  QString rotatorManufacturer;
  QString rotatorModelName;
  int rotatorModelNumber = 0;
  int rotatorCWEndStop = COMPASS_MIN0;  // actual rotator endstops
  int rotatorCCWEndStop= COMPASS_MAX360;
  endStop rotType = ROT_0_360;          // actual rotator type

  QString pollInterval = ROT_DEFAULT_POLLINTERVAL;

  int min_azimuth = 0;                   // working endstops
  int max_azimuth = 0;
  endStop endStopType = ROT_0_360;      //working endstop type

  int min_elevation = 0.0;
  int max_elevation = 0.0;
  southStop southStopType = S_STOPOFF;
  bool overRunFlag = false;
  bool supportCwCcwCmd = true;
  bool simCwCcwCmd = false;
  int antennaOffset = 0;
  bool moving = false;
  rig_port_e portType = RIG_PORT_NONE;
  QString networkAdd;
  QString networkPort;
  int maxBaudRate = 0;
  int minBaudRate = 0;
  int baudrate = 0;
  int parity = 0;
  int stopbits = 0;
  int databits = 0;
  int handshake = 0;
  bool enableRot = false;
  bool activeRTS = false;
  bool activeDTR = false;
  bool nactiveRTS = false;
  bool nactiveDTR = false;

};




class RotControl: public QObject
{
    Q_OBJECT

    static const char * hamlibErrorMsg[];
public:
    explicit RotControl(QObject *parent = nullptr);
    ~RotControl();
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
    azimuth_t getRotatorAzimuth();
    int request_bearing();
    int rotate_to_bearing(int bearing);
    int rotateCClockwise(int speed);
    int rotateClockwise(int speed);
    int stop_rotation();
    serial_parity_e getSerialParityCode(int index);
    serial_handshake_e getSerialHandshakeCode(int index);
    QStringList getParityCodeNames();
    QStringList getHandShakeNames();
    QStringList getBaudRateNames();
    QStringList getDataBitsNames();
    QStringList getStopBitsNames();
//    QStringList gethamlibErrorMsg();
    QString gethamlibErrorMsg(int errorCode);
    QString gethamlibVersion();
//    QString initError;

    //azimuth_t getMaxAzimuth();
    //azimuth_t getMinAzimuth();
    //elevation_t getMaxElevation();
    //elevation_t getMinElevation();

    int calcSouthBearing(int rotatorBearing);

    int rig_message_cb(enum rig_debug_level_e debug_level, const char *fmt, va_list ap);


    int getMaxBaudRate();
    int getMinBaudRate();
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
    elevation_t rot_elevation; // not used

    int rot_speed = 100;
    bool rotControlEnabled;
    bool serialConnected;
    void errorMessage(int errorCode,QString command);
    bool rotatorlistLoaded=false;
    //srotParams curRotParams;   remove
    int serialP;

    bool traceComms = false;



    int retcode;		/* generic return code from functions */
    int exitcode;



};

#endif // ROTCONTROL_H
