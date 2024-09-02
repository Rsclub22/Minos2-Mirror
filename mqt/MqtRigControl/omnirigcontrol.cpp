/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Omnirig Rig Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2016 - 2023
//
//  Insperation for some of this code from the WSJT-X Project.
//
//
/////////////////////////////////////////////////////////////////////////////



#include <QObject>
#include <QString>
#include <QTimer>
#include <QThread>
#include "BandList.h"
#include "MTrace.h"

#if defined (WIN32)
#include "omnirigcontrol.h"
#endif

const char* OmnirigControl::omnirigErrorMsg[] =  {QT_TR_NOOP("No Error, operation completed sucessfully"),
                                                QT_TR_NOOP("Radio Function not supported"),
                                                QT_TR_NOOP("Omnirig Com Failed to start"),
                                                QT_TR_NOOP("Omnirig rig One failed to initialise"),
                                                QT_TR_NOOP("Omnirig rig Two failed to initialise"),
                                                QT_TR_NOOP("Omnirig rig is offline"),
                                                QT_TR_NOOP("Omnirig radio online"),
                                                QT_TR_NOOP("Omnirig get freq fail"),
                                                QT_TR_NOOP("Omnirig COM Exception\nCheck Rigcontrol tracelog"),
                                                QT_TR_NOOP("Omnirig not configured"),
                                                QT_TR_NOOP("Omnirig disabled"),
                                                QT_TR_NOOP("Omnirig port busy"),
                                                QT_TR_NOOP("Omnirig not responding"),
                                                QT_TR_NOOP("Omnirig rig pointer null")

                                                };


bool omnirigTraceComms = false;

namespace
{
  auto constexpr OmniRigOneName = "OmniRig Rig 1";
  auto constexpr OmniRigTwoName = "OmniRig Rig 2";
}

auto OmnirigControl::map_mode (OmniRig::RigParamX param) -> MODE
{
  if (param & OmniRig::PM_CW_U)
  {
      return CW;
  }
  else if (param & OmniRig::PM_CW_L)
  {
      return CW_R;
  }
  else if (param & OmniRig::PM_SSB_U)
  {
      return USB;
  }
  else if (param & OmniRig::PM_SSB_L)
  {
      return LSB;
  }
  else if (param & OmniRig::PM_DIG_U)
  {
      return DIG_U;
  }
  else if (param & OmniRig::PM_DIG_L)
  {
      return DIG_L;
  }
  else if (param & OmniRig::PM_AM)
  {
      return AM;
  }
  else if (param & OmniRig::PM_FM)
  {
      return FM;
  }
  trace(QString("Omnirig: unrecognized mode"));

  return UNK;
}

OmniRig::RigParamX OmnirigControl::map_mode (MODE mode)
{
  switch (mode)
  {
    case AM: return OmniRig::PM_AM;
    case CW: return OmniRig::PM_CW_L;
    case CW_R: return OmniRig::PM_CW_U;
    case USB: return OmniRig::PM_SSB_U;
    case LSB: return OmniRig::PM_SSB_L;
    case FSK: return OmniRig::PM_DIG_L;
    case FSK_R: return OmniRig::PM_DIG_U;
    case DIG_L: return OmniRig::PM_DIG_L;
    case DIG_U: return OmniRig::PM_DIG_U;
    case FM: return OmniRig::PM_FM;
    case DIG_FM: return OmniRig::PM_FM;
    default: break;
  }
  return OmniRig::PM_SSB_U;
}

OmniRig::RigParamX OmnirigControl::map_mode(QString mode)
{
    if (mode == hamlibData::AM) return OmniRig::PM_AM;
    if (mode == hamlibData::CW) return OmniRig::PM_CW_L;
    if (mode == hamlibData::CWR) return OmniRig::PM_CW_U;
    if (mode == hamlibData::USB) return OmniRig::PM_SSB_U;
    if (mode == hamlibData::LSB) return OmniRig::PM_SSB_L;
    if (mode == hamlibData::RTTY) return OmniRig::PM_DIG_L;
    if (mode == hamlibData::RTTYR) return OmniRig::PM_DIG_U;
    if (mode == hamlibData::PKTLSB) return OmniRig::PM_DIG_L;
    if (mode == hamlibData::PKTUSB) return OmniRig::PM_DIG_U;
    if (mode == hamlibData::FM) return OmniRig::PM_FM;
    if (mode == hamlibData::PKTFM) return OmniRig::PM_FM;
    else return OmniRig::PM_SSB_U;
}




OmnirigControl::OmnirigControl(RigNumber rig_number_, QObject *parent)  : RigBase(parent),
    rig_number(rig_number_),
    reversedVFO(false)

{
    setRigConnected(false);
}

OmnirigControl::~OmnirigControl()
{

}



void OmnirigControl::onHandleCOMException (int code, QString source, QString desc, QString help)
{
    traceMsg(QString("COM/OLE error: %1 at %2: %3 (%4)").arg (QString::number(code), source, desc, help));
    emit rigStatus(OMINIRIG_COM_EXCEPTION * -1, QString("COM Exception"));
}

void OmnirigControl::onHandleVisibleChange()
{
    if (!omni_rig || omni_rig->isNull ())
        return;
    traceMsg(QString(" visibility change: visibility = %1").arg(omni_rig->DialogVisible ()));
}




void OmnirigControl::onHandleRigTypeChange(int rigNumber)
{
    if (!omni_rig || omni_rig->isNull())
        return;
    traceMsg(QString("OmniRigTransceiver rig type change: rig = %1").arg(rigNumber));



}


void OmnirigControl::onHandleStatusChange(int rigNumber)
{
    if (!omni_rig || omni_rig->isNull())
        return;
    if (rig_number == rigNumber)
    {
        if (!rig || rig->isNull ())
            return;

        status = rig->Status();
        traceMsg(QString("Rig %1 Status Change: new status = %2").arg(rigNumber).arg(rig->StatusStr()));
        if (status != OmniRig::ST_ONLINE)
        {
            if (!offlineTimer->isActive())  // to cover for rogue offlines
            {
                offlineTimer->start(10000);
            }




        }
        else if (status == OmniRig::ST_ONLINE)
        {

            offlineTimer->stop();

            setRigConnected(true);
            //emit rigStatus(OMNIRIG_ONLINE);

        }


        else if (status == OmniRig::ST_NOTCONFIGURED)
        {
            traceMsg(QString("OmniRig::ST_NOTCONFIGURED"));
            //setRigConnected(false);
            //emit rigStatus(OMNIRIG_NOTCONFIGURED * -1, QString("Status"));
        }
        else if (status == OmniRig::ST_DISABLED)
        {
            traceMsg(QString("OmniRig::ST_DISABLED"));
            //setRigConnected(false);
            //emit rigStatus(OMNIRIG_DISABLED * -1, QString("Status"));
        }
        else if (status == OmniRig::ST_PORTBUSY)
        {
            //setRigConnected(false);
            //emit rigStatus(OMNIRIG_PORTBUSY * -1, QString("Status"));
            traceMsg(QString("Rig %1 port is busy").arg(rigNumber));
        }
        else if (status == OmniRig::ST_NOTRESPONDING)
        {
            //setRigConnected(false);
            //emit rigStatus(OMNIRIG_NOTRESPONDING * -1, QString("Status"));
            traceMsg(QString("Rig %1 is not responding").arg(rigNumber));
        }


    }

}


void OmnirigControl::onOffLineTimeout()
{

    offlineTimer->stop();
    setRigConnected(false);
    emit rigStatus(OMNIRIG_OFFLINE * -1, QString("Status"));



}

void OmnirigControl::onHandleParamsChange(int rigNumber, int params)
{
    if (!omni_rig || omni_rig->isNull ())
        return;
    traceMsg(QString("OmniRig params change: params = 0x%1 for rig %2")
          .arg (params, 8, 16, QChar ('0'))
          .arg (rig_number).toLocal8Bit ());



    if (rigNumber == rig_number)
    {
        if (!rig || rig->isNull ())
            return;

        auto need_frequency = false;



        if ((params & OmniRig::PM_VFOAA) && curVfo != VFO::CURRENT_VFO)
        {
            traceMsg(QString("OmniRig params change: OmniRig VFOAA"));
            reversedVFO = false;
            need_frequency = true;
            curVfo = VFO::VFOA;
            emit newVfo(vfoToStr(curVfo));


        }
        if (params & OmniRig::PM_VFOAB)
         {
            traceMsg(QString("OmniRig params change: OmniRig VFOAB"));
            reversedVFO = false;
        }
        if (params & OmniRig::PM_VFOBA)
        {
            traceMsg(QString("OmniRig params change: OmniRig VFOBA"));
            reversedVFO = true;

        }
        if ((params & OmniRig::PM_VFOBB) && curVfo != VFO::CURRENT_VFO )
        {
            traceMsg(QString("OmniRig params change: OmniRig VFOBB"));
            reversedVFO = true;
            need_frequency = true;
            curVfo = VFO::VFOB;
            emit newVfo(vfoToStr(curVfo));


        }
        if ((params & OmniRig::PM_VFOA) && curVfo != VFO::CURRENT_VFO)
        {
            traceMsg(QString("OmniRig params change: OmniRig VFOA"));
            reversedVFO = false;
            need_frequency = true;
            curVfo = VFO::VFOA;
            emit newVfo(vfoToStr(curVfo));

        }
        if ((params & OmniRig::PM_VFOB) && curVfo != VFO::CURRENT_VFO)
        {
            traceMsg(QString("OmniRig params change: OmniRig VFOB"));
            reversedVFO = true;
            need_frequency = true;
            curVfo = VFO::VFOB;
            emit newVfo(vfoToStr(curVfo));


        }
        if (params & OmniRig::PM_FREQ)
        {
            auto f = rig->Freq();

            emit newRxFreq(Frequency(f));
            traceMsg(QString("OmniRig params change: OmniRig PM_FREQ = %1").arg(QString::number(f)));

        }
        if ((params & OmniRig::PM_FREQA) && !reversedVFO)
        {
            auto f = rig->FreqA ();
            emit newRxFreq(Frequency(f));
            traceMsg(QString("OmniRig params change: OmniRig PM_FREQA = %1").arg(QString::number(f)));

        }
        if ((params & OmniRig::PM_FREQB) && reversedVFO)
        {
            auto f = rig->FreqB ();
            emit newRxFreq(Frequency(f));
            traceMsg(QString("OmniRig params change: OmniRig PM_FREQB = %1").arg(QString::number(f)));

        }
        if (need_frequency)
        {
            if ((readable_params & OmniRig::PM_FREQA) && !reversedVFO )
            {
                auto f = rig->FreqA();
                if (f)
                {
                    traceMsg(QString("OmniRig Need Freq, PM_FREQA = %1").arg(f));
                    emit newRxFreq(Frequency(f));
                }
            }
            else if ((readable_params & OmniRig::PM_FREQB) && reversedVFO)
            {
                auto f = rig->FreqB();
                if (f)
                {
                    traceMsg(QString("OmniRig Need Freq, PM_FREQB = %1").arg(f));
                    emit newRxFreq(Frequency(f));
                }
            }
            else if (readable_params & OmniRig::PM_FREQ)
            {
                auto f = rig->Freq();
                if (f)
                {
                    traceMsg(QString("OmniRig Need Freq, PM_FREQ = %1").arg(f));
                    emit newRxFreq(Frequency(f));
                }
            }
        }
        if (params & OmniRig::PM_PITCH)
        {
            traceMsg(QString("OmniRig params change: OmniRig PITCH"));
        }
        if (params & OmniRig::PM_RITOFFSET)
        {
            traceMsg(QString("OmniRig params change: OmniRig RITOFFSET"));
            //auto f = rig->RitOffset();
            emit ritOffset();
        }
        if (params & OmniRig::PM_RIT0)
        {
            traceMsg(QString("OmniRig params change: OmniRig RIT0"));
            emit rit0();
        }
        if (params & OmniRig::PM_VFOEQUAL)
        {
            traceMsg(QString("OmniRig params change: OmniRig PM_VFOEQUAL"));
        }
        if (params & OmniRig::PM_VFOSWAP)
        {
            traceMsg(QString("OmniRig params change: OmniRig VFOSWAP"));

        }
        if (params & OmniRig::PM_SPLITON)
        {
            traceMsg(QString("OmniRig params change: OmniRig SPLITON"));

        }
        if (params & OmniRig::PM_SPLITOFF)
        {
            traceMsg(QString("OmniRig params change: OmniRig SPLITOFF"));

        }
        if (params & OmniRig::PM_RITON)
        {
            traceMsg(QString("OmniRig params change: OmniRig RITON"));
            emit ritOn();
        }
        if (params & OmniRig::PM_RITOFF)
        {
            traceMsg(QString("OmniRig params change: OmniRig RITOFF"));
            emit ritOff();
        }
        if (params & OmniRig::PM_XITON)
        {
            traceMsg(QString("OmniRig params change: OmniRig XITON"));
        }
        if (params & OmniRig::PM_XITOFF)
        {
            traceMsg(QString("OmniRig params change: OmniRig XITOFF"));
        }
        if (params & OmniRig::PM_RX)
        {
            traceMsg(QString("OmniRig params change: OmniRig RX"));
            emit pttState(false);

        }
        if (params & OmniRig::PM_TX)
        {
            traceMsg(QString("OmniRig params change: OmniRig TX"));
            emit pttState(true);

        }
        if (params & OmniRig::PM_CW_U)
        {
            traceMsg(QString("Parameter change - mode PM_CW_U"));
            emit newMode();

        }
        if (params & OmniRig::PM_CW_L)
        {
           traceMsg(QString("Parameter change - mode PM_CW_L"));
           emit newMode();
        }
        if (params & OmniRig::PM_SSB_U)
        {
            traceMsg(QString("Parameter change - mode PM_SSB_U"));
            emit newMode();
        }
        if (params & OmniRig::PM_SSB_L)
        {
            traceMsg(QString("Parameter change - mode PM_SSB_L"));
            emit newMode();
        }
        if (params & OmniRig::PM_DIG_U)
        {
            traceMsg(QString("Parameter change - mode PM_DIG_U"));
            emit newMode();
        }
        if (params & OmniRig::PM_DIG_L)
        {
            traceMsg(QString("Parameter change - mode PM_DIG_L"));
            emit newMode();
        }
        if (params & OmniRig::PM_AM)
        {
            traceMsg(QString("Parameter change - mode PM_AM"));
            emit newMode();
        }
        if (params & OmniRig::PM_FM)
        {
            traceMsg(QString("Parameter change - mode PM_FM"));
            emit newMode();
        }


    }


}


void OmnirigControl::onHandleCustomReply(int, QVariant const&, QVariant const&)
{

}


void OmnirigControl::register_rigs(RigFactory::Rigs* rigsList, int id1, int id2)
{

    RigCapabilities rigCap;

    rigCap.setPortType(RigCapConstants::PortType::none);
    rigCap.setRigManufacturer(OMINRIG_MFR_NAME);
    rigCap.setRigName(OMNIRIG_NAME);
    rigCap.setRigModelName(OmniRigOneName);
    rigCap.setRigModelNumber(id1);
    rigCap.setLibraryName("Omnirig");
    rigCap.setLibraryVersion(OmnirigControl::getRigLibVersion());
    rigCap.setSupportGetSupBands(false);
    rigCap.setSupportGetVfo(false);
    rigCap.setSupportSetVfo(false);
    rigCap.setSupportGetRit(false);
    rigCap.setSupportSetRit(false);
    rigCap.setSupportGetRitState(false);
    rigCap.setSupportSetRitState(false);
    rigCap.setSupportGetRitMax(false);
    rigCap.setSupportSMeter(false);
    rigCap.setSupportPttPortType(RigCapConstants::RigPttPortType::RIG_PTT_RIG);
    rigCap.setSupportGetPtt(true);
    rigCap.setSupportSetPtt(true);
    rigCap.setSupportGetVox(false);
    rigCap.setSupportSetVox(false);
    rigCap.setSupportVolume(false);
    rigCap.setSupportAntSw(false);
    rigCap.setSupportRigCtld(true);
    rigCap.setSupportVoiceMemory(false);
    rigCap.setStartVoiceMemoryNumber(0);
    rigCap.setEndVoiceMemoryNumber(0);
    rigCap.setSupportStopVoiceMemory(false);
    rigCap.setStartCwMemoryNumber(0);
    rigCap.setEndCwMemoryNumber(0);
    rigCap.setSupportCwMemory(false);
    rigCap.setSupportCwMemoryStop(false);
    rigCap.setSupportCwMemoryWait(false);
    rigCap.setPollData(false);



    (*rigsList)[OmniRigOneName] = rigCap;



    rigCap.setRigModelName(OmniRigTwoName);
    rigCap.setRigModelNumber(id2);

    (*rigsList)[OmniRigTwoName] = rigCap;




}



int OmnirigControl::omnirigError(omnirigErrorCode errNum)
{
    return  errNum * -1;
}




int OmnirigControl::rigInit(scatParams &currentRadio, bool useRigCtld)
{
    Q_UNUSED(useRigCtld)

    traceMsg(QString("Trying to start COM server"));

    omni_rig = new OmniRig::OmniRigX(this);

    if (omni_rig->isNull())
    {
        traceMsg(QString("Failed to start COM server"));
        return omnirigError(OMNIRIG_COM_FAILED_START);
    }
    else
    {
        traceMsg(QString("COM server started"));

    }

    offlineTimer =  new QTimer(this);
    offlineTimer->setSingleShot (true);
    connect (offlineTimer, SIGNAL(timeout()), this,  SLOT(onOffLineTimeout()));


    // COM/OLE exceptions get signaled
    connect (&*omni_rig, SIGNAL (exception (int, QString, QString, QString)), this, SLOT (onHandleCOMException (int, QString, QString, QString)));

    // IOmniRigXEvent interface signals
    connect (&*omni_rig, SIGNAL (VisibleChange ()), this, SLOT (onHandleVisibleChange()));
    connect (&*omni_rig, SIGNAL (RigTypeChange (int)), this, SLOT (onHandleRigTypeChange(int)));
    connect (&*omni_rig, SIGNAL (StatusChange (int)), this, SLOT (onHandleStatusChange(int)));
    connect (&*omni_rig, SIGNAL (ParamsChange (int, int)), this, SLOT (onHandleParamsChange(int, int)));
    connect (&*omni_rig
             , SIGNAL (CustomReply (int, QVariant const&, QVariant const&))
             , this, SLOT (onHandleCustomReply (int, QVariant const&, QVariant const&)));

    QString v = QString::number(omni_rig->SoftwareVersion()).toLocal8Bit ();
    traceMsg(QString("Software Version %1").arg(v));


    switch (rig_number)
    {
      case One:
        rig =  new OmniRig::RigX (omni_rig->Rig1 ());
        break;
      case Two:
        rig = new OmniRig::RigX (omni_rig->Rig2 ());
        break;
    }

    if (rig->isNull())
    {
        if (rig_number == One)
        {
            traceMsg(QString("Rig %1 failed to initialise").arg(QString::number(rig_number)));
            return omnirigError(OMNIRIG_ONE_FAILED_INITIALISE);
        }
        else
        {
            traceMsg(QString("Rig %1 initialised").arg(QString::number(rig_number)));
            return omnirigError(OMNIRIG_TWO_FAILED_INITIALISE);
        }
    }

    serPort = (new OmniRig::PortBits(rig->PortBits()));


    rig_type = rig->RigType ();
    traceMsg(QString("Opening Rig %1").arg(rig_type));
    currentRadio.rigModelName = rig_type;
    readable_params = rig->ReadableParams ();
    writable_params = rig->WriteableParams ();

    trace(QString("Omnirig: Rig initial rig type: %1 readable params = 0x%2 writable params = 0x%3 for rig %4")
                .arg (rig_type)
                .arg (readable_params, 8, 16, QChar ('0'))
                .arg (writable_params, 8, 16, QChar ('0'))
                .arg (rig_number).toLocal8Bit ());

    // wait to come on-line
    bool ok = false;
    for (int i = 0; i < 10; i++)
    {
        if (OmniRig::ST_ONLINE == rig->Status ())
        {
            ok = true;
            break;
        }

        QThread::msleep(500);
    }

    if (ok)
    {
        setRigConnected(true);
        trace(QString("Omnirig: Rig %1 online").arg(QString::number(rig_number)));
    }
    else
    {
        setRigConnected(false);
        trace(QString("Omnirig: Rig %1 offline").arg(QString::number(rig_number)));
        return omnirigError(OMNIRIG_OFFLINE);

    }




    if ((readable_params & (OmniRig::PM_VFOA | OmniRig::PM_VFOAA | OmniRig::PM_VFOB | OmniRig::PM_VFOBB)) && !rig_type.contains("IC"))
    {
        traceMsg(QString("Able to read VFO state"));
        auto vfo = rig->Vfo();
        if (vfo == OmniRig::PM_VFOA || vfo == OmniRig::PM_VFOAA)
        {
            curVfo = VFO::VFOA;
            reversedVFO = false;

        }
        else if (vfo == OmniRig::PM_VFOB || vfo == OmniRig::PM_VFOBB)
        {
            curVfo = VFO::VFOB;
            reversedVFO = true;

        }
    }
    else
    {
        traceMsg(QString("Not able to read VFO state or Icom"));

        if ((writable_params & (OmniRig::PM_VFOA | OmniRig::PM_VFOB)) == (OmniRig::PM_VFOA | OmniRig::PM_VFOB))
        {
            // start with VFO A (probably MAIN) on rigs that we
            // can't query VFO but can set explicitly
            rig->SetVfo (OmniRig::PM_VFOA);

        }
        else if ((writable_params & (OmniRig::PM_VFOAA | OmniRig::PM_VFOBB)) == (OmniRig::PM_VFOAA | OmniRig::PM_VFOBB))
        {

            rig->SetVfo(OmniRig::PM_VFOAA);

        }

        curVfo = VFO::CURRENT_VFO;

    }

    traceMsg(QString("Vfo set to %1").arg(vfoToStr(curVfo)));
    emit newVfo(vfoToStr(curVfo));



    // also allow time to get freq
    traceMsg(QString("Rig Init - try to get freq"));
    //auto f = rig->GetRxFrequency();

    Frequency f;
    getFrequency(CURRENT_VFO, f);
    traceMsg(QString("Rig Init - first getFreq = %1").arg(f.traceStr()));
    for (int i = 0; (f.isClear()) && (i < 10); i++)
    {
        traceMsg(QString("Rig Init getFreq i = %1").arg(i));
        getFrequency(CURRENT_VFO, f);
        traceMsg(QString("Rig Init getRxFreq = %1").arg(f.traceStr()));
        if (!f.isClear())
        {
            break;
        }
        QThread::msleep(500);
    }

    traceMsg(QString("Rig Init - freq after delay = %1").arg(f.traceStr()));

    if (!BandList::getBandList().checkValidBand(f))
    {
        f.clear();      // received freq not valid
        traceMsg(QString("Rig Init - freq not in valid band"));
    }
    else
    {
        traceMsg(QString("Rig Init - freq in valid band"));
    }


    if (!f.isClear())
    {
        setRigConnected(true);
        traceMsg(QString(" Rig init, get freq = %1").arg(f.traceStr()));
    }
    else
    {
        setRigConnected(false);
        traceMsg(QString(" Rig init, get freq failed"));
        return omnirigError(OMNIRIG_GETFREQ_FAIL);
    }

    traceMsg(QString(" Rig init: connected = %1").arg(getRigConnected() ? "yes" : "no"));
    return omnirigError(OMNIRIG_OK);

}



int OmnirigControl::closeRig()
{
    traceMsg(QString(" Closing Rig"));

    QThread::msleep(200);       // leave time for pending commands

    if (serPort !=nullptr)
    {
        traceMsg(QString(" Closing Serial Port"));
        serPort->Unlock();     // release serial port
        serPort->clear();
        serPort = nullptr;
    }
    if (omni_rig)
    {
        traceMsg(QString(" Closing COM Component"));

        if (rig)
        {
            rig->clear();
            rig = nullptr;
        }
        omni_rig->clear();
    }

    setRigConnected(false);

    QThread::msleep(500); // allow time to close serial and com component
    return OMNIRIG_OK;
}

int OmnirigControl::getFrequency(VFO vfo, Frequency &freq)
{
    traceMsg(QString("Get Frequency"));
    Q_UNUSED(vfo)
    if (!rig || rig->isNull ())
    {
        return omnirigError(OMNIRIG_RIG_NULL);
    }

    if (rigConnected)
    {

        if (/*OmniRig::PM_UNKNOWN == rig->Vfo() && */(readable_params & OmniRig::PM_FREQ) || curVfo == VFO::CURRENT_VFO)
        {
            auto f = rig->Freq();
            traceMsg(QString("Current VFO freq = %1").arg(f));
            if (f)
            {
                traceMsg(QString("GetFrequency, PM_FREQ = %1").arg(f));
                freq = Frequency(f);

            }
        }
        else if ((readable_params & OmniRig::PM_FREQA) && !reversedVFO )
        {
            auto f = rig->FreqA();
            traceMsg(QString("VFOA freq = %1").arg(f));
            if (f)
            {
                traceMsg(QString("GetFrequency, PM_FREQA = %1").arg(f));
                freq = Frequency(f);
            }
        }
        else if ((readable_params & OmniRig::PM_FREQB) && reversedVFO)
        {
            auto f = rig->FreqB();
            traceMsg(QString("VFOB freq = %1").arg(f));
            if (f)
            {
                traceMsg(QString("GetFrequency, PM_FREQB = %1").arg(f));
                freq = Frequency(f);
            }
        }



        return omnirigError(OMNIRIG_OK);
    }

    return omnirigError(OMNIRIG_OFFLINE);
}

int OmnirigControl::setFrequency(const Frequency &freq, VFO vfo)
{
    traceMsg(QString("Set Frequency = %1").arg(freq.traceStr()));
    Q_UNUSED(vfo)

    if (!rig || rig->isNull ())
    {
        return omnirigError(OMNIRIG_RIG_NULL);
    }


    if (rigConnected)
    {
        traceMsg(QString("SetFrequency = %1").arg(QString::number(static_cast<qint64>(freq))));
        //rig->SetFreq(static_cast<int>(freq));

        int f = qint64(freq);
        if (OmniRig::PM_FREQ & writable_params)
        {
            rig->SetFreq(f);
        }
        else if (reversedVFO && (OmniRig::PM_FREQB & writable_params))
        {
            rig->SetFreqB(f);
        }
        else if (!reversedVFO && (OmniRig::PM_FREQA & writable_params))
        {
            rig->SetFreqA (f);
        }

        return omnirigError(OMNIRIG_OK);
    }

    return omnirigError(OMNIRIG_OFFLINE);


}

int OmnirigControl::getMode(VFO vfo, MODE &mode)
{
    traceMsg(QString("Get mode"));
    Q_UNUSED(vfo)
    if (!rig || rig->isNull ())
    {
        return omnirigError(OMNIRIG_RIG_NULL);
    }

    if (!rigConnected)
    {
        return omnirigError(OMNIRIG_OFFLINE);

    }

    mode = map_mode(rig->Mode());
    traceMsg(QString("GetMode = %1").arg(rigcommon::convertModeToQString(mode)));

    return omnirigError(OMNIRIG_OK);


}

int OmnirigControl::setMode(VFO vfo, MODE mode)
{
    traceMsg(QString("SetMode "));
    Q_UNUSED(vfo)
    if (!rig || rig->isNull ())
    {
        return omnirigError(OMNIRIG_RIG_NULL);
    }

    if (!rigConnected)
    {
        return omnirigError(OMNIRIG_OFFLINE);

    }
        traceMsg(QString("SetMode = %1").arg(rigcommon::convertModeToQString(mode)));
        OmniRig::RigParamX m = map_mode(mode);
        rig->SetMode(m);

        return omnirigError(OMNIRIG_OK);


}

bool OmnirigControl::modeSupported(MODE /*mode*/, Frequency /*f*/)
{
    return true;
}



int OmnirigControl::setVfo(VFO vfo)
{
    if (vfo == VFO::VFOA)
    {
        rig->SetVfo (OmniRig::PM_VFOA);
    }
    else if (vfo == VFO::VFOB)
    {
        rig->SetVfo (OmniRig::PM_VFOB);
    }

    return  omnirigError(OMNIRIG_OK);

}



int OmnirigControl::getVfo(VFO *vfo)
{

    *vfo = curVfo;

    return omnirigError(OMNIRIG_OK);

}



int OmnirigControl::setVolume(VFO vfo, float val)
{
    traceMsg(QString(" SetVolume = %1 ").arg(QString::number(val)));
    Q_UNUSED(vfo)
    Q_UNUSED(val)
    return omnirigError(OMNIRIG_NOT_SUPPORTED);
}




int OmnirigControl::getVolume(VFO vfo, float *val)
{
    traceMsg(QString(" GetVolume"));
    Q_UNUSED(vfo)
    Q_UNUSED(val)
    return omnirigError(OMNIRIG_NOT_SUPPORTED);
}


int OmnirigControl::getSignalStrength(VFO vfo, int *value)
{
    traceMsg(QString(" GetSignal Strength"));
    Q_UNUSED(vfo)
    Q_UNUSED(value)
    return omnirigError(OMNIRIG_NOT_SUPPORTED);
}

QString OmnirigControl::getRigLibVersion()
{
    trace("get Omnirig Version Number");

    OmniRig::OmniRigX* omni_rig = new OmniRig::OmniRigX();

    if (omni_rig->isNull())
    {

        return QString("lib ver com failed to start");
    }


    QString v = QString::number(omni_rig->SoftwareVersion()).toLocal8Bit ();

    if (omni_rig)
    {
        omni_rig->clear();

    }

    return QString("Omnirig V%1").arg(v);
}

QString OmnirigControl::getErrorMsgText(int errorCode)
{
    if (errorCode > static_cast<int>(sizeof(omnirigErrorMsg)/sizeof(const char *)))
    {
        return tr("Omnirig Errorcode too large!");
    }
    return tr(omnirigErrorMsg[errorCode]);
}




int OmnirigControl::getRit(VFO vfo, ShortFreq &ritfreq)
{
    Q_UNUSED(vfo)
    int f = rig->RitOffset();
    ritfreq = ShortFreq(f);
    return omnirigError(OMNIRIG_OK);
}

int OmnirigControl::setRit(VFO vfo, const ShortFreq &ritfreq)
{
    Q_UNUSED(vfo)
    rig->SetRitOffset(ritfreq);
    return omnirigError(OMNIRIG_OK);
}


int OmnirigControl::setRitState(VFO vfo, bool state)
{
    Q_UNUSED(vfo)
    if (state)
    {
        rig->SetRit(OmniRig::PM_RITON);
    }
    else
    {
        rig->SetRit(OmniRig::PM_RITOFF);
    }

    return omnirigError(OMNIRIG_OK);

}

int OmnirigControl::getRitState(VFO vfo, bool& state)
{
    Q_UNUSED(vfo)
    OmniRig::RigParamX s = rig->Rit();
    if (s == OmniRig::PM_RITON)
    {
        state = true;
    }
    else if (s == OmniRig::PM_RITOFF)
    {
        state = false;
    }

    return omnirigError(OMNIRIG_OK);

}

int OmnirigControl::clearRit(VFO vfo)
{
    Q_UNUSED(vfo)
    rig->ClearRit();
    return omnirigError(OMNIRIG_OK);
}





int OmnirigControl::getMaxRitFreq(int rigModelNumber)
{
    Q_UNUSED(rigModelNumber);

    return 9990;
}

int OmnirigControl::setConfigurationParameter(QString cfgparam, QString value)
{
    Q_UNUSED(cfgparam)
    Q_UNUSED(value)
    int retCode = Rig_OK;
    return retCode;
}
int OmnirigControl::getConfigurationParameter(QString cfgparam, QString *value)
{
    Q_UNUSED(cfgparam)
    Q_UNUSED(value)
    int retCode = Rig_OK;
    return retCode;
}

int OmnirigControl::sendVoiceMessage(VFO vfo, int vmNum)
{
    Q_UNUSED(vmNum)
    Q_UNUSED(vfo)
    return omnirigError(OMNIRIG_NOT_SUPPORTED);
}



int OmnirigControl::stop_voice_mem(VFO vfo)
{

    Q_UNUSED(vfo)
    return omnirigError(OMNIRIG_NOT_SUPPORTED);
}




int OmnirigControl::sendMorse(VFO vfo, QString msg)
{
    Q_UNUSED(vfo)
    Q_UNUSED(msg)
    return omnirigError(OMNIRIG_NOT_SUPPORTED);
}
int OmnirigControl::stopMorse(VFO vfo)
{
    Q_UNUSED(vfo)
    return omnirigError(OMNIRIG_NOT_SUPPORTED);
}
//int OmnirigControl::waitMorsePtt(VFO vfo)
//{
//    Q_UNUSED(vfo)
//    return omnirigError(OMNIRIG_NOT_SUPPORTED);
//}
int OmnirigControl::waitMorse(VFO vfo)
{
    Q_UNUSED(vfo)
    return omnirigError(OMNIRIG_NOT_SUPPORTED);
}




int OmnirigControl::getPttStatus(VFO vfo, bool &state)
{
    Q_UNUSED(vfo)
    Q_UNUSED(state)
    return omnirigError(OMNIRIG_NOT_SUPPORTED);
}

int OmnirigControl::setPtt(VFO vfo, bool state)
{
    Q_UNUSED(vfo)

    rig->SetTx (state ? OmniRig::PM_TX : OmniRig::PM_RX);
    return omnirigError(OMNIRIG_OK);
}




int OmnirigControl::setVoxState(VFO vfo, bool state)
{
    Q_UNUSED(vfo)
    Q_UNUSED(state)
    return omnirigError(OMNIRIG_NOT_SUPPORTED);
}



int OmnirigControl::getVoxState(VFO vfo, bool &state)
{
    Q_UNUSED(vfo)
    Q_UNUSED(state)
    return omnirigError(OMNIRIG_NOT_SUPPORTED);
}

void OmnirigControl::setTraceCommsFlag(bool value)
{
    omnirigTraceComms = value;
}

void OmnirigControl::setTraceComms(bool value)
{
    omnirigTraceComms = value;
}

bool OmnirigControl::getTraceComms()
{
    return omnirigTraceComms;
}

void OmnirigControl::setRigConnected(bool rigConnected_)
{
    trace(QString("Omnirig: setRigConnected = %1").arg(rigConnected_ ? "true" : "false"));
    rigConnected = rigConnected_;
}

bool OmnirigControl::getRigConnected()
{
    trace(QString("Omnirig: getRigConnected = %1").arg(rigConnected ? "true" : "false"));
    return rigConnected;
}


void OmnirigControl::traceCommsMsg(QString msg)
{
    if (omnirigTraceComms)
    {
       traceMsg(msg);
    }

}

void OmnirigControl::traceMsg(QString msg)
{

       trace(QString("Omnirig: %1").arg(msg));


}
