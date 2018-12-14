#ifndef ROTCONTROL_H
#define ROTCONTROL_H

#include <QObject>
#include <QComboBox>
#include <QStringList>

#include <hamlib/rig.h>

bool model_Sort(const rig_caps *caps1,const rig_caps *caps2);


namespace serialData
{
const serial_parity_e parityCodes[] = {RIG_PARITY_NONE, RIG_PARITY_ODD, RIG_PARITY_EVEN, RIG_PARITY_MARK, RIG_PARITY_SPACE};
const serial_handshake_e handshakeCodes[] = { RIG_HANDSHAKE_NONE, RIG_HANDSHAKE_XONXOFF, RIG_HANDSHAKE_HARDWARE };
const QStringList parityStr = (QStringList() << "None" << "Odd" << "Even" << "Mark" << "Space");
const QStringList handshakeStr = (QStringList() << "None" << "XON/XOFF" << "CTS/RTS");
const QStringList baudrateStr = (QStringList() << "" << "1200" << "2400" << "4800" << "9600" << "19200" << "38400");
const QStringList databitsStr = (QStringList() << "" << "7" << "8" );
const QStringList stopbitsStr = (QStringList()<< "" << "1" << "2");


const QStringList hamlibErrorMsg = {"No Error, operation completed sucessfully",
                                "Invalid parameter", "Invalid configuration",
                                "Memory shortage", "Function not implemented",
                                "Communication timed out", "IO error, including open failed",
                                "Internal Hamlib error", "Protocol error",
                                "Command rejected by the rig", "Command performed, but arg truncated",
                                "Function not available", "VFO not targetable",
                                "Error talking on the bus", "Collision on the bus",
                                "NULL RIG handle or any invalid pointer parameter in get arg",
                                "Invalid VFO", "RIG_EDOM"};


}



struct scatParams
{
//  QString configLabel;
  QString radioName;
  QString comport; /**<  serial port device*/
  QString radioMfg_Name;
  QString radioModel;
  QString radioModelName;
  int radioModelNumber;
  QString civAddress;
  int baudrate; /**<  serial port baudrate*/
  int parity;
  int stopbits;
  int databits;
  int handshake;
  bool enableCAT;
  bool enableSerialPTT;
  QString pttSerialPort;
  bool activeRTS;
  bool activeDTR;
  bool nactiveRTS;
  bool nactiveDTR;
  ptt_type_t pttType;
//  bool enableXMLRPC;
//  int XMLRPCPort;
  double txOnDelay;

};

class RigControl: public QObject
{
  Q_OBJECT
public:
  RigControl();
  ~RigControl();
  int init(scatParams currentRadio);
  bool enabled() {return rigControlEnabled;}
  int getFrequency();
  bool setFrequency(double frequency);
  bool getMode(QString &mode);
  bool setMode(QString mode);
  int getModelNumber(int idx);
  int getRigModelIndex();
  void getRigList();
  bool getRigList(QComboBox *cb);
  const char * getMfg_Name(int idx);
  const char * getModel_Name(int idx);
  void disable();
  int closeRig();
  scatParams* params() {return &catParams;}
  void activatePTT(bool b);
  double getTxDelay() {return catParams.txOnDelay;}
  int rawCommand(QByteArray ba);
//  QString initError;
  serial_parity_e getSerialParityCode(int index);
  serial_handshake_e getSerialHandshakeCode(int index);
  QStringList getParityCodeNames();
  QStringList getHandShakeNames();
  QStringList getBaudRateNames();
  QStringList getDataBitsNames();
  QStringList getStopBitsNames();
  void set_serialConnected(bool connectFlag);
  bool get_serialConnected();
  QStringList gethamlibErrorMsg();
  QString gethamlibErrorMsg(int errorCode);

signals:
  void frequency_updated(double);

private:
  hamlib_port_t myport;
  RIG *my_rig;            // handle to rig (nstance)
  freq_t frequency;            // frequency
  rmode_t rmode;          // radio mode of operation
  pbwidth_t width;
  vfo_t vfo;              // vfo selection
  int strength;           // S-Meter level
  int retcode;            // generic return code from functions
  rig_model_t myrig_model;
  bool rigControlEnabled;
  bool serialConnected;
  void errorMessage(int errorCode,QString command);
  void getRadioList();
  scatParams catParams;
  int serialP;
  bool setPTT(bool On);
  double lastFrequency;
  QStringList xmlModes;



};




#endif // ROTCONTROL_H
