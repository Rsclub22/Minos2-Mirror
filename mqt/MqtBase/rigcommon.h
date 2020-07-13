/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2018
//
//
//
/////////////////////////////////////////////////////////////////////////////

#ifndef RIGCOMMON_H
#define RIGCOMMON_H

#include <QComboBox>
#include "rigcontrolcommonconstants.h"

static const char blankString[] = QT_TRANSLATE_NOOP("SettingsDialog", "N/A");

enum MODE {UNK, CW, CW_R, USB, LSB, FSK, FSK_R, DIG_U, DIG_L, AM, FM, DIG_FM};

// radio modes

const QString AM_STR = "AM";             // Amplitude Modulation
const QString CW_STR =  "CW";            //  normal sideband
const QString USB_STR = "USB";           // Upper Side Band
const QString LSB_STR = "LSB";           // Lower Side Band
const QString RTTY_STR = "RTTY";         //  Radio Teletype
const QString FM_STR = "FM";             // "narrow" band FM
const QString CW_R_STR = "CW_R";            //  "reverse" sideband
const QString RTTY_R_STR = "RTTYR";       // RTTY "reverse" sideband
const QString PKTLSB_STR = "PKTLSB";     // Packet/Digital LSB mode (dedicated port)
const QString PKTUSB_STR = "PKTUSB";     // Packet/Digital USB mode (dedicated port)
const QString PKTFM_FM = "PKTFM";       // Packet/Digital FM mode (dedicated port)

const QString MGM_STR = "MGM";           // MGM generically

const QStringList supModeList = { CW_STR, USB_STR, FM_STR, MGM_STR};

enum VFO {CURRENT_VFO, VFOA, VFOB};


QString vfoToStr(VFO curVfo);
VFO strToVfo(QString vfo);

using  Frequency = quint64;
using ShortFreq = qint32;

// These are used in hamlib
// post_write_delay disables ACK/NAK in FT817
enum CONFIG_PARAM {post_write_delay};


class TransVertParams
{

public:

    QString transVertName;
    QString band;
    QString radioFreqStr;
    Frequency radioFreq = 0;
    QString targetFreqStr;
    Frequency targetFreq = 0;
    Frequency fLow;
    Frequency fHigh;
    QString transVertOffsetStr;
    Frequency transVertOffset = 0;
    QString antSwitchNum = "0";
    QString transSwitchNum = "";


};


// This was the hamlib catParams structure, other fields have been added
// to support other functions.

class scatParams
{

public:

  static void copyRig(scatParams* srce, scatParams &dest)
  {

      dest.radioName = srce->radioName;
      dest.radioNumber = srce->radioNumber;
      dest.comport = srce->comport;
      dest.rigMfg_Name = srce->rigMfg_Name;
      dest.rigModel = srce->rigModel;
      dest.rigModelName = srce->rigModelName;
      dest.rigModelNumber = srce->rigModelNumber;
      dest.pollInterval = srce->pollInterval;
      dest.civAddress = srce->civAddress;
      dest.baudrate = srce->baudrate;
      dest.parity = srce->parity;
      dest.stopbits = srce->stopbits;
      dest.databits = srce->databits;
      dest.handshake = srce->handshake;
      dest.forceDtr = srce->forceDtr;
      dest.forceRts = srce->forceRts;
      dest.portType = srce->portType;
      dest.advancedCommsFlag = srce->advancedCommsFlag;
      dest.networkAdd = srce->networkAdd;
      dest.networkPort = srce->networkPort;
      dest.enableCAT = srce->enableCAT;
      dest.enableSerialPTT = srce->enableSerialPTT;
      dest.pttSerialPort = srce->pttSerialPort;
      dest.rigCtldEnable = srce->rigCtldEnable;
      dest.startMinosRigCtld = srce->startMinosRigCtld;
      dest.rigCtldNetworkAdd = srce->rigCtldNetworkAdd;
      dest.rigCtldNetworkPort = srce->rigCtldNetworkPort;
      dest.support50MHz = srce->support50MHz;
      dest.support144MHz = srce->support144MHz;
      dest.support432MHz = srce->support432MHz;
      dest.support1296MHz = srce->support1296MHz;
      dest.mgmMode = srce->mgmMode;
      dest.pttType = srce->pttType;
      dest.antSwitchAvail = srce->antSwitchAvail;
      dest.ritSupported = srce->ritSupported;
      dest.ritEnable = srce->ritEnable;
      dest.radioSupBands = srce->radioSupBands;
      dest.radioTransSupBands = srce->radioTransSupBands;
      dest.transVertEnable = srce->transVertEnable;
      dest.enableTransSwitch = srce->enableTransSwitch;
      dest.enableLocTVSwMsg = srce->enableLocTVSwMsg;
      dest.locTVSwComport = srce->locTVSwComport;

      dest.transVertNames.clear();
      if (srce->transVertNames.count() > 0)
      {
          for (int i = 0; i < srce->transVertNames.count(); i++)
          {
              dest.transVertNames.append(srce->transVertNames[i]);
          }
      }
      dest.numTransverters = srce->numTransverters;
      dest.transVertSettings.clear();
      if (srce->numTransverters > 0)
      {
          for (int i = 0; i <srce->numTransverters; i++)
          {
              dest.transVertSettings.append(srce->transVertSettings[i]);
          }
      }

  }


  QString radioName;    //Minos Radio Name
  QString radioNumber;  // Minos Radio Number
  QString comport; /**<  serial port device*/
  QString rigMfg_Name;
  QString rigModelName;
  QString rigModel;       // used as key to select radio
  int rigModelNumber = 0;
  QString pollInterval = RIG_DEFAULT_POLLINTERVAL;
  QString civAddress;
  int baudrate = 0; /**<  serial port baudrate*/
  int parity = 0;
  int stopbits = 0;
  int databits = 0;
  int handshake = 0;
  int forceDtr = 0;
  int forceRts = 0;
  int portType = 0;
  bool advancedCommsFlag;
  QString networkAdd;
  QString networkPort;
  bool enableCAT = false;
  bool enableSerialPTT  = false;
  QString pttSerialPort;
  bool rigCtldEnable = false;
  bool startMinosRigCtld = true;
  QString rigCtldNetworkAdd;
  QString rigCtldNetworkPort;
  QString mgmMode = "USB";
  int pttType;
  bool antSwitchAvail = false;
  bool ritSupported = false;
  bool ritEnable = false;
  bool transVertEnable  = false;
  bool volAvail = false;
  bool support50MHz = false;        // for non hamlib radios
  bool support70MHz = false;
  bool support144MHz = false;
  bool support432MHz = false;
  bool support1296MHz = false;
  QStringList transVertNames;
  int numTransverters = 0;
  bool enableTransSwitch = false;
  bool enableLocTVSwMsg = false;
  QString locTVSwComport = "";
  QStringList radioSupBands;  // bands supported by radio
  QStringList radioTransSupBands; // band supported by radio and transverters
  QVector<TransVertParams*> transVertSettings;


};


class rigcommon
{
public:
    rigcommon();
};




void fillPortsInfo(QComboBox* comportSel);


#endif // RIGCOMMON_H
