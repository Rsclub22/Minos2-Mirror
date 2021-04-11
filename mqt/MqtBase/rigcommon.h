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
            if (supportBands.value(i.key()) != i.value())
            {
                state = false;
                break;
            }
        }

        return state;
    }

    bool operator!=(const SupportBands &sbd)
    {
        bool state = false;

        for (QMap<QString, bool>::const_iterator i = sbd.supportBands.begin(); i != sbd.supportBands.end(); i++)
        {
            if (supportBands.value(i.key()) != i.value())
            {
                state = true;
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

class EnableDisableCatFeature
{
public:
    EnableDisableCatFeature(){};

    bool operator ==(const EnableDisableCatFeature &edcf);
    bool operator !=(const EnableDisableCatFeature &edcf);
    EnableDisableCatFeature& operator =(const EnableDisableCatFeature &edcf);

    bool enableDisplay = false;
    bool ritEnable = false;
    bool sMeterEnable = true;
    bool volumeEnable = true;
    bool voiceMemEnable = true;
    bool cWMemEnable = true;
    bool catEnable = true;

};




// This was the hamlib catParams structure, other fields have been added
// to support other functions.

class scatParams
{

public:

  scatParams(){}

  bool operator ==(const scatParams radParams);
  bool compareEqual(QSharedPointer<scatParams> radParams);
  bool compareNotEqual(QSharedPointer<scatParams> radParams);
  void scatParamsCopy(const QSharedPointer<scatParams> srce);

  bool compareStringList(QStringList &sl1, QStringList &sl2);






  QString radioName;    //Minos Radio Name
  QString radioNumber;  // Minos Radio Number
  QString comport;
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
  int pttType = 0;
  bool rigCtldEnable = false;
  bool startMinosRigCtld = true;
  QString rigCtldNetworkAdd;
  QString rigCtldNetworkPort;
  QString mgmMode = "USB";
  bool antSwitchAvail = false;
  bool ritSupported = false;

  bool transVertEnable  = false;
  SupportBands supportBands;        // for non hamlib radios
  QStringList transVertNames;
  int numTransverters = 0;
  bool enableTransSwitch = false;
  bool enableLocTVSwMsg = false;
  QString locTVSwComport = "";
  QStringList radioSupBands;  // bands supported by radio
  QStringList radioTransSupBands; // band supported by radio and transverters
  QVector<TransVertParams*> transVertSettings;

  EnableDisableCatFeature enableDisableCatFeature;





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
