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
#include "BandList.h"
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


// hamlib conf token strings
const QString HAMLIB_RETRY = "retry";
const QString HAMLIB_TIMEOUT = "timeout";


const int MAX_RITFREQ = 9999;




class TransVertParams
{

public:

    QString transVertName;
    QString band;
    Frequency radioFreq;
    Frequency targetFreq;
    Frequency fLow;
    Frequency fHigh;
    Frequency transVertOffset;
    QString antSwitchNum = "0";
    QString transSwitchNum = "";


};




// for non Hamlib Radios
class SupportBands
{
public:

    SupportBands()
    {

        BandList::getBandList().loadAllBands(bands);

        foreach (auto &b, bands)
        {

            supportBands.insert(b.data()->name(), false);
        }
    }


    SupportBands& operator=(const SupportBands &sbd)
    {

        foreach (auto &b, bands)
        {
            supportBands.insert(b.data()->name(), sbd.getSupportBandFlag(b.data()->name()));
        }

        return *this;
    }

    bool operator==(const SupportBands &sbd)
    {
        bool state = true;

        for (QMap<QString, bool>::const_iterator i = sbd.supportBands.begin(); i != sbd.supportBands.end(); i++)
        {
            if (supportBands.value(i.key()) == i.value())
            {
                state = false;
                break;
            }
        }

        return state;
    }


    void clear()
    {
        foreach (auto &b, bands)
        {
            supportBands.insert(b.data()->name(), false);
        }
    }

    int count()
    {
        return supportBands.count();
    }

    bool getSupportBandFlag(const QString band) const
    {
        if (supportBands.contains(band))
        {
            return supportBands.value(band);

        }

        return false;
    }

    void setSupportBandFlag(QString band, bool state)
    {
        if (supportBands.contains(band))
        {

            supportBands.insert(band, state);
        }
    }

    QVector<QSharedPointer<BandInfo> > bands;
    QMap<QString, bool> supportBands;

};






// This was the hamlib catParams structure, other fields have been added
// to support other functions.

class scatParams
{

public:

  scatParams(){}

  bool operator==(const QSharedPointer<scatParams> radParams)
  {
      if (radioName == radParams->radioName &&
              radioNumber == radParams->radioNumber &&
              comport == radParams->comport &&
              rigMfg_Name == radParams->rigMfg_Name &&
              rigModelName == radParams->rigModelName &&
              rigModel == radParams->rigModel &&
              rigModelNumber == radParams->rigModelNumber &&
              pollInterval == radParams->pollInterval &&
              civAddress == radParams->civAddress &&
              baudrate == radParams->baudrate &&
              parity == radParams->parity &&
              stopbits == radParams->stopbits &&
              databits == radParams->databits &&
              handshake == radParams->handshake &&
              forceDtr == radParams->forceDtr &&
              forceRts == radParams->forceRts &&
              portType == radParams->portType &&
              advancedCommsFlag == radParams->advancedCommsFlag &&
              networkAdd == radParams->networkAdd &&
              networkPort == radParams->networkPort &&
              enablePTT == radParams->enablePTT &&
              pttSerialPort == radParams->pttSerialPort &&
              pttType == radParams->pttType &&
              rigCtldEnable == radParams->rigCtldEnable &&
              startMinosRigCtld == radParams->startMinosRigCtld &&
              rigCtldNetworkAdd == radParams->rigCtldNetworkAdd &&
              rigCtldNetworkPort == radParams->rigCtldNetworkPort &&
              mgmMode == radParams->mgmMode &&
              antSwitchAvail == radParams->antSwitchAvail &&
              ritSupported == radParams->ritSupported &&

              transVertEnable == radParams->transVertEnable &&
              volAvail == radParams->volAvail &&
              supportBands == radParams->supportBands &&     // for non hamlib radios
              compareStringList(transVertNames, radParams->transVertNames) &&
              numTransverters == radParams->numTransverters &&
              enableTransSwitch == radParams->enableTransSwitch &&
              enableLocTVSwMsg == radParams->enableLocTVSwMsg &&
              locTVSwComport == radParams->locTVSwComport &&
              compareStringList(radioSupBands, radParams->radioSupBands) &&
              compareStringList(radioTransSupBands, radParams->radioTransSupBands) &&
              transVertSettings == radParams->transVertSettings &&
              enableShowCatFeatures == radParams->enableShowCatFeatures &&
              ritEnable == radParams->ritEnable &&
              sMeterEnable == radParams->sMeterEnable &&
              volumeEnable == radParams->volumeEnable &&
              voiceMemEnable == radParams->voiceMemEnable &&
              cWMemEnable == radParams->cWMemEnable &&
              catEnable == radParams->catEnable)
      {
         return true;
      }

      return false;

  }


  bool compareStringList(QStringList &sl1, QStringList &sl2)
  {
      if (sl1.count() != sl2.count())
      {
          return false;
      }
      else
      {
          for (int i = 0; i <sl1.count(); i++)
          {
              if (sl1[i] != sl2[i])
              {
                  return false;
              }
          }
      }
      return true;
  }

  QSharedPointer<scatParams> operator=(const QSharedPointer<scatParams> srce)
  {
      radioName = srce->radioName;
      radioNumber = srce->radioNumber;
      comport = srce->comport;
      rigMfg_Name = srce->rigMfg_Name;
      rigModelName = srce->rigModelName;
      rigModel = srce->rigModel;       // used as key to select radio
      rigModelNumber = srce->rigModelNumber;
      pollInterval = srce->pollInterval;
      civAddress = srce->civAddress;
      baudrate = srce->baudrate;
      parity = srce->parity;
      stopbits = srce->stopbits;
      databits = srce->databits;
      handshake = srce->handshake;
      forceDtr = srce->forceDtr;
      forceRts = srce->forceRts;
      portType = srce->portType;
      advancedCommsFlag = srce->advancedCommsFlag;
      networkAdd = srce->networkAdd;
      networkPort = srce->networkPort;
      enablePTT  = srce->enablePTT;
      pttSerialPort = srce->enablePTT;
      pttType = srce->pttType;
      rigCtldEnable = srce->rigCtldEnable;
      startMinosRigCtld = srce->startMinosRigCtld;
      rigCtldNetworkAdd = srce->rigCtldNetworkAdd;
      rigCtldNetworkPort = srce->rigCtldNetworkPort;
      mgmMode = srce->mgmMode;
      antSwitchAvail = srce->antSwitchAvail;
      ritSupported = srce->ritSupported;

      transVertEnable  = srce->transVertEnable;
      volAvail = srce->volAvail;
      supportBands = srce->supportBands;        // for non hamlib radios
      transVertNames = srce->transVertNames;
      numTransverters = srce->numTransverters;
      enableTransSwitch = srce->enableTransSwitch;
      enableLocTVSwMsg = srce->enableLocTVSwMsg;
      locTVSwComport = srce->locTVSwComport;
      radioSupBands = srce->radioSupBands;  // bands supported by radio
      radioTransSupBands = srce->radioTransSupBands; // band supported by radio and transverters
      transVertSettings = srce->transVertSettings;


      // enable\Disable Cat features
      enableShowCatFeatures = srce->enableShowCatFeatures;
      ritEnable = srce->ritEnable;
      sMeterEnable = srce->sMeterEnable;
      volumeEnable = srce->volumeEnable;
      voiceMemEnable = srce->voiceMemEnable;
      cWMemEnable = srce->cWMemEnable;
      catEnable = srce->catEnable;

      return QSharedPointer<scatParams>(this);

  }


  QString radioName;    //Minos Radio Name
  QString radioNumber;  // Minos Radio Number
  QString comport; /**<  serial port device*/
  QString rigMfg_Name;
  QString rigModelName;
  QString rigModel;       // used as key to select radio
  int rigModelNumber = 1;
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
  bool advancedCommsFlag = false;
  QString networkAdd;
  QString networkPort;
  bool enablePTT  = false;
  QString pttSerialPort;
  int pttType;
  bool rigCtldEnable = false;
  bool startMinosRigCtld = true;
  QString rigCtldNetworkAdd;
  QString rigCtldNetworkPort;
  QString mgmMode = "USB";
  bool antSwitchAvail = false;
  bool ritSupported = false;

  bool transVertEnable  = false;
  bool volAvail = false;
  SupportBands supportBands;        // for non hamlib radios
  QStringList transVertNames;
  int numTransverters = 0;
  bool enableTransSwitch = false;
  bool enableLocTVSwMsg = false;
  QString locTVSwComport = "";
  QStringList radioSupBands;  // bands supported by radio
  QStringList radioTransSupBands; // band supported by radio and transverters
  QVector<TransVertParams*> transVertSettings;

  // enable\Disable Cat features
  bool enableShowCatFeatures = false;
  bool ritEnable = false;
  bool sMeterEnable = true;
  bool volumeEnable = true;
  bool voiceMemEnable = true;
  bool cWMemEnable = true;
  bool catEnable = true;


};


class rigcommon
{
public:
    rigcommon();
};




void fillPortsInfo(QComboBox* comportSel);
void getListOfComports(QStringList &listOfAvailComports);
bool isComportAvail(const QString comport);

#endif // RIGCOMMON_H
