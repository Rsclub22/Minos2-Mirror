/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rig Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2016 - 2024
//
//
//
//
/////////////////////////////////////////////////////////////////////////////



#ifndef RIGCAPABILITIES_H
#define RIGCAPABILITIES_H

#include <QString>

namespace RigCapConstants
{
    enum PortType {none, serial, network, usb, udp};


    // These are the PTT capabilities from hamlib. Most radios are RIG_PTT_NONE, RIG_PTT_RIG and RIG_PTT_RIG_MICDATA
    // we will use RIG_PTT_NONE to disable CAT PTT and allow Hardware PTT
    enum RigPttPortType {RIG_PTT_NONE = 0,       /*!< No PTT available */
                      RIG_PTT_RIG,            /*!< Legacy PTT (CAT PTT) */
                      RIG_PTT_SERIAL_DTR,     /*!< PTT control through serial DTR signal */
                      RIG_PTT_SERIAL_RTS,     /*!< PTT control through serial RTS signal */
                      RIG_PTT_PARALLEL,       /*!< PTT control through parallel port */
                      RIG_PTT_RIG_MICDATA,    /*!< Legacy PTT (CAT PTT), supports RIG_PTT_ON_MIC/RIG_PTT_ON_DATA */
                      RIG_PTT_CM108,          /*!< PTT control through CM108 GPIO pin */
                      RIG_PTT_GPIO,           /*!< PTT control through GPIO pin */
                      RIG_PTT_GPION          /*!< PTT control through inverted GPIO pin */
                     };

    enum PollData {pollDataOn = true, pollDataOff = false};
}



class RigCapabilities
{

public:


    RigCapabilities();

    ~RigCapabilities();


    void clear();

    void setPortType(RigCapConstants::PortType portType_);
    RigCapConstants::PortType getPortType() const;

    void setRigManufacturer(QString rigManufacturer_);
    QString getRigManufacturer()const;

    void setRigName(QString rigName_);
    QString getRigName()const;

    void setRigModelName(QString rigModelName_);
    QString getRigModelName()const;

    void setRigModelNumber(int rigModelNumber_);
    int getRigModelNumber()const;

    void setLibraryName(QString libraryName_);
    QString getLibraryName();

    void setLibraryVersion(QString libraryVersion_);
    QString getLibraryVersion();

    void setSupportGetSupBands(bool supportGetSupBands_);
    bool getSupportGetSupBands()const;

    void setSupportGetVfo(bool supportGetVfo_);
    bool getSupportGetVfo()const;

    void setSupportSetVfo(bool supportSetVfo_);
    bool getSupportSetVfo()const;

    void setSupportGetRit(bool supportGetRit_);
    bool getSupportGetRit()const;

    void setSupportSetRit(bool supportSetRit_);
    bool getSupportSetRit()const;

    void setSupportGetRitState(bool supportGetRitState_);
    bool getSupportGetRitState()const;

    void setSupportSetRitState(bool supportSetRitState_);
    bool getSupportSetRitState()const;

    void setSupportGetRitMax(bool supportGetRitMax_);
    bool getSupportGetRitMax()const;

    void setSupportSMeter(bool supportSMeter_);
    bool getSupportSMeter()const;

    void setSupportPttPortType(RigCapConstants::RigPttPortType supportPttPortType_);
    RigCapConstants::RigPttPortType  getSupportPttPortType();

    void setSupportGetPtt(bool supportGetPtt_);
    bool getSupportGetPtt()const;

    void setSupportSetPtt(bool supportSetPtt_);
    bool getSupportSetPtt()const;

    void setSupportGetVox(bool supportGetVox_);
    bool getSupportGetVox()const;

    void setSupportSetVox(bool supportSetVox_);
    bool getSupportSetVox()const;

    void setSupportVolume(bool supportVolume_);
    bool getSupportVolume()const;

    void setSupportAntSw(bool supportAntSw_);
    bool getSupportAntSw()const;

    void setSupportRigCtld(bool supportRigCtld_);
    bool getSupportRigCtld()const;

    void setSupportVoiceMemory(bool supportVoiceMemory_);
    bool getSupportVoiceMemory()const;

    void setStartVoiceMemoryNumber(int startVoiceMemoryNumber_);
    int getStartVoiceMemoryNumber()const;

    void setEndVoiceMemoryNumber(int endVoiceMemoryNumber_);
    int getEndVoiceMemoryNumber()const;

    void setSupportStopVoiceMemory(bool supportStopVoiceMemory_);
    bool getSupportStopVoiceMemory()const;

    void setSupportCwMemory(bool supportCwMemory_);
    bool getSupportCwMemory()const;

    void setStartCwMemoryNumber(int startCwMemoryNumber_);
    int getStartCwMemoryNumber()const;

    void setEndCwMemoryNumber(int endCwMemoryNumber_);
    int getEndCwMemoryNumber()const;

    void setSupportCwMemoryStop(bool supportCwMemoryStop_);
    bool getSupportCwMemoryStop()const;

    void setSupportCwMemoryWait(bool supportCwMemoryWait_);
    bool getSupportCwMemoryWait()const;

    void setPollData(bool pollData_);
    bool getPollData()const;





  RigCapabilities( const RigCapabilities &rigcap);
  RigCapabilities &operator= ( const RigCapabilities &rigcap);

private:

  RigCapConstants::PortType portType;
  QString rigManufacturer;
  QString rigName;
  QString rigModelName;       // combined manufacturer and rig names
  int rigModelNumber;
  QString libraryName;
  QString libraryVersion;
  bool supportGetSupBands;
  bool supportGetVfo;
  bool supportSetVfo;
  bool supportGetRit;
  bool supportSetRit;
  bool supportGetRitState;
  bool supportSetRitState;
  bool supportGetRitMax;
  bool supportSMeter;
  RigCapConstants::RigPttPortType supportPttPortType = RigCapConstants::RigPttPortType::RIG_PTT_NONE;
  bool supportGetPtt;
  bool supportSetPtt;
  bool supportGetVox;
  bool supportSetVox;
  bool supportVolume;
  bool supportAntSw;
  bool supportRigCtld;
  bool supportVoiceMemory;
  int startVoiceMemoryNumber;
  int endVoiceMemoryNumber;
  bool supportStopVoiceMemory;
  bool supportCwMemory;
  int startCwMemoryNumber;
  int endCwMemoryNumber;
  bool supportCwMemoryStop;
  bool supportCwMemoryWait;
  bool pollData;


};

#endif // RIGCAPABILITIES_H
