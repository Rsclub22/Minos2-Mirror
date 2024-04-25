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
#include "QtUtils.h"
#include "BandList.h"
#include "rigcontrolcommonconstants.h"
#include "serialCommonData.h"

static const char blankString[] = QT_TRANSLATE_NOOP("SettingsDialog", "N/A");

enum MODE {UNK, CW, CW_R, USB, LSB, FSK, FSK_R, DIG_U, DIG_L, AM, FM, DIG_FM};

enum VFO {CURRENT_VFO, VFOA, VFOB};


QString vfoToStr(VFO curVfo);
VFO strToVfo(QString vfo);


const int MAX_RITFREQ = 9999;

auto constexpr OMNIRIG_NAME = "Omnirig";
auto constexpr OMINRIG_MFR_NAME = "Afreet";



class TransVertParams
{

public:

    QString transVertName;
    QString band;
    Frequency radioFreq;
    Frequency targetFreq;
    Frequency transVertOffset;
    QString antSwitchNum = "0";
    QString transSwitchNum = "";
    bool markForDeletion = false;

TransVertParams& operator=(const TransVertParams &tvp)
{

    transVertName = tvp.transVertName;
    band = tvp.band;
    radioFreq = tvp.radioFreq;
    targetFreq = tvp.targetFreq;
    transVertOffset = tvp.transVertOffset;
    antSwitchNum = tvp.antSwitchNum;
    transSwitchNum = tvp.transSwitchNum;
    markForDeletion = tvp.markForDeletion;



    return *this;
}

bool notEqual(const QSharedPointer<TransVertParams> tvp)
{
    if (transVertName != tvp->transVertName ||
        band != tvp->band ||
        radioFreq != tvp->radioFreq ||
        targetFreq != tvp->targetFreq ||
        transVertOffset != tvp->transVertOffset ||
        antSwitchNum != tvp->antSwitchNum ||
        transSwitchNum != tvp->transSwitchNum ||
        markForDeletion != tvp->markForDeletion)
    {
        return true;
    }

    return false;
}

};




// for non Hamlib Radios
class SupportBands
{
public:

    SupportBands()
    {

        BandList::getBandList().loadAllBands(bands);

        for (const auto &b: QASCONST(bands))
        {

            supportBands.insert(b->name(), false);
        }
    }


    SupportBands& operator=(const SupportBands &sbd)
    {

        for (const auto &b: QASCONST(bands))
        {
            supportBands.insert(b->name(), sbd.getSupportBandFlag(b->name()));
        }

        return *this;
    }

    bool operator==(const SupportBands &sbd)
    {
        return supportBands == sbd.supportBands;
    }

    bool operator!=(const SupportBands &sbd)
    {
        return supportBands != sbd.supportBands;
    }



    void clear()
    {
        for (const auto &b: QASCONST(bands))
        {
            supportBands.insert(b->name(), false);
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
    void clear();

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




class RadioNameChange
{
public:
    RadioNameChange(){};



    QString newName;
    QString oldName;

};



// This was the hamlib catParams structure, other fields have been added
// to support other functions.

class scatParams
{

public:

  scatParams(){}

  void clear();

  bool operator ==(const scatParams radParams);
  bool compareEqual(QSharedPointer<scatParams> radParams);
  bool compareNotEqual(QSharedPointer<scatParams> radParams);
  void scatParamsCopy(const QSharedPointer<scatParams> srce);

  bool compareStringList(QStringList &sl1, QStringList &sl2);
  bool transVertSettingsNotEqual(const QMap<QString, QSharedPointer<TransVertParams> > tvs2);




  QString radioName;    //Minos Radio Name
  QString previousRadioName;
  bool markForDeletion = false;
  QString radioNumber;  // Minos Radio Number
  QString comport;
  QString rigMfg_Name;
  QString rigModelName;
  QString rigModel;       // used as key to select radio
  int rigModelNumber = 1;
  QString pollInterval = RIG_DEFAULT_POLLINTERVAL;
  QString civAddress;
  int baudrate = 0; /**<  serial port baudrate*/
  serialCommonData::serialParityCodes parity = serialCommonData::serialParityCodes::PARITY_NONE;
  int stopbits = 0;
  int databits = 0;
  serialCommonData::s_handshakeCodes handshake = serialCommonData::s_handshakeCodes::HANDSHAKE_NONE;
  serialCommonData::s_forceLinesCodes forceDtr = serialCommonData::s_forceLinesCodes::FORCE_LINE_NONE;
  serialCommonData::s_forceLinesCodes forceRts = serialCommonData::s_forceLinesCodes::FORCE_LINE_NONE;
  int portType = 0;
  bool advancedCommsFlag = false;
  QString networkAdd;
  QString networkPort;
  bool enablePTT  = false;
  QString pttSerialPort;
  serialCommonData::PTTMethodCodes pttType = serialCommonData::PTTMethodCodes::PTT_METHOD_NONE;
  bool rigCtldEnable = false;
  bool startMinosRigCtld = true;
  QString rigCtldNetworkAdd;
  QString rigCtldNetworkPort;
  QString mgmMode = hamlibData::USB;
  QString rttyMode = hamlibData::LSB;
  QString pskMode = hamlibData::USB;
  bool antSwitchAvail = false;
  bool ritSupported = false;

  bool transVertEnable  = false;
  SupportBands supportBands;        // for non hamlib radios
  QMap<QString, QSharedPointer<TransVertParams> > transVertSettings;
  bool enableTransSwitch = false;
  bool enableLocTVSwMsg = false;
  QString locTVSwComport = "";
  QStringList radioSupBands;  // bands supported by radio
  QStringList radioTransSupBands; // band supported by radio and transverters

  EnableDisableCatFeature enableDisableCatFeature;
};


class rigcommon
{
public:
    rigcommon();
    static MODE convertQStringToMode(QString modeStr);
    static QString convertModeToQString(MODE m);
};

void fillPortsInfo(QComboBox* comportSel);
void getListOfComports(QStringList &listOfAvailComports);
bool isComportAvail(const QString comport);

#endif // RIGCOMMON_H
