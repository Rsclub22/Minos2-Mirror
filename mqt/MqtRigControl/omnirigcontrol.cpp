/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Omnirig Rig Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2016 - 2020
//
//  Insperation for some of this code from the WSJT-X Project.
//
//
/////////////////////////////////////////////////////////////////////////////



#include <QObject>
#include <QString>
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

QString OmnirigControl::convertModeToQString(MODE mode)
{
    switch (mode)
      {
      case AM: return "AM";
      case CW: return "CW";
      case CW_R: return "CW_R";
      case USB: return "USB";
      case LSB: return "LSB";
      case FSK: return "FSK";
      case FSK_R: return "FSK_R";
      case DIG_L: return "DIG_L";
      case DIG_U: return "DIG_U";
      case FM: return "FM";
      case DIG_FM: return "DIG_FM";
      default: break;
      }
    return "USB";
}

OmniRig::RigParamX OmnirigControl::map_mode(QString mode)
{
    if (mode == "AM") return OmniRig::PM_AM;
    if (mode == "CW") return OmniRig::PM_CW_L;
    if (mode == "CW_R") return OmniRig::PM_CW_U;
    if (mode == "USB") return OmniRig::PM_SSB_U;
    if (mode == "LSB") return OmniRig::PM_SSB_L;
    if (mode == "FSK") return OmniRig::PM_DIG_L;
    if (mode == "FSK_R") return OmniRig::PM_DIG_U;
    if (mode == "DIG_L") return OmniRig::PM_DIG_L;
    if (mode == "DIG_U") return OmniRig::PM_DIG_U;
    if (mode == "FM") return OmniRig::PM_FM;
    if (mode == "DIG_FM") return OmniRig::PM_FM;
    else return OmniRig::PM_SSB_U;
}




OmnirigControl::OmnirigControl(RigNumber rig_number_, QObject *parent)  : RigBase(parent),
    rig_number(rig_number_)

{
    setRigConnected(false);
}

OmnirigControl::~OmnirigControl()
{

}



void OmnirigControl::onHandleCOMException (int code, QString source, QString desc, QString help)
{
    traceMsg(QString("COM/OLE error: %1 at %2: %3 (%4)").arg (QString::number(code)).arg(source). arg(desc). arg(help));
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
            setRigConnected(false);
            emit rigStatus(OMNIRIG_OFFLINE * -1, QString("Status"));


        }
        else if (status == OmniRig::ST_ONLINE)
        {
            setRigConnected(true);
            //emit rigStatus(OMNIRIG_ONLINE);

        }
        else if (status == OmniRig::ST_NOTCONFIGURED)
        {
            setRigConnected(false);
            emit rigStatus(OMNIRIG_NOTCONFIGURED * -1, QString("Status"));
        }
        else if (status == OmniRig::ST_DISABLED)
        {
            setRigConnected(false);
            emit rigStatus(OMNIRIG_DISABLED * -1, QString("Status"));
        }
        else if (status == OmniRig::ST_PORTBUSY)
        {
            setRigConnected(false);
            emit rigStatus(OMNIRIG_PORTBUSY * -1, QString("Status"));
        }
        else if (status == OmniRig::ST_NOTRESPONDING)
        {
            setRigConnected(false);
            emit rigStatus(OMNIRIG_NOTRESPONDING * -1, QString("Status"));
        }


    }

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

        if (params & OmniRig::PM_VFOAA)
        {
            traceMsg(QString("OmniRig params change: OmniRig VFOAA"));

        }
        if (params & OmniRig::PM_VFOAB)
         {
            traceMsg(QString("OmniRig params change: OmniRig VFOAB"));

        }
        if (params & OmniRig::PM_VFOBA)
        {
            traceMsg(QString("OmniRig params change: OmniRig VFOBA"));

        }
        if (params & OmniRig::PM_VFOBB)
        {
            traceMsg(QString("OmniRig params change: OmniRig VFOBB"));

        }
        if (params & OmniRig::PM_VFOA)
        {
            traceMsg(QString("OmniRig params change: OmniRig VFOA"));

        }
        if (params & OmniRig::PM_VFOB)
        {
            traceMsg(QString("OmniRig params change: OmniRig VFOB"));

        }
        if (params & OmniRig::PM_FREQ)
        {
            traceMsg(QString("OmniRig params change:  PM_FREQ"));
            emit newFreq();
        }
        if (params & OmniRig::PM_FREQA)
        {
            auto f = rig->FreqA ();
            traceMsg(QString("OmniRig params change: OmniRig FREQA = %1").arg(QString::number(f)));

        }
        if (params & OmniRig::PM_FREQB)
        {
            auto f = rig->FreqB ();
            traceMsg(QString("OmniRig params change: OmniRig FREQB = ").arg(QString::number(f)));

        }
        if (params & OmniRig::PM_PITCH)
        {
            traceMsg(QString("OmniRig params change: OmniRig PITCH"));
        }
        if (params & OmniRig::PM_RITOFFSET)
        {
            traceMsg(QString("OmniRig params change: OmniRig RITOFFSET"));
        }
        if (params & OmniRig::PM_RIT0)
        {
            traceMsg(QString("OmniRig params change: OmniRig RIT0"));
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
        }
        if (params & OmniRig::PM_RITOFF)
        {
            traceMsg(QString("OmniRig params change: OmniRig RITOFF"));
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

        }
        if (params & OmniRig::PM_TX)
        {
            traceMsg(QString("OmniRig params change: OmniRig TX"));

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

    (*rigsList)[OmniRigOneName] = RigCapabilities(
                RigCapConstants::PortType::none,
                "Afreet",
                "Omnirig",
                OmniRigOneName,
                id1,
                false,      // support getSupBands
                false,      // support get rit
                false,      // support set rit
                false,      // support get rit state
                false,      // support set rit state
                false,      // support s-meter
                false,      // support get Ptt
                false,      // support set Ptt
                false,      // support volume
                false,      // support antenna switch
                false,      // support rigctld
                false);     // polldata flag

    // note the library may support these features,
    // but Omnirig should be polled to verify the radio supports the feature



    (*rigsList)[OmniRigTwoName] = RigCapabilities(
                RigCapConstants::PortType::none,
                "Afreet",
                "Omnirig",
                OmniRigTwoName,
                id2,
                false,      // support getSupBands
                false,      // support get rit
                false,      // support set rit
                false,      // support get rit state
                false,      // support set rit state
                false,      // support s-meter
                false,      // support get Ptt
                false,      // support set Ptt
                false,      // support volume
                false,      // support antenna switch
                false,      // support rigctld
                false);     // polldata flag


}



int OmnirigControl::omnirigError(omnirigErrorCode errNum)
{
    return  errNum * -1;
}




int OmnirigControl::rigInit(scatParams &currentRadio, bool useRigCtld)
{
    Q_UNUSED(useRigCtld)

    trace(QString("Omnirig: Trying to start COM server"));

    omni_rig = new OmniRig::OmniRigX(this);

    if (omni_rig->isNull())
    {
        trace(QString("Failed to start Omnirig COM server"));
        return omnirigError(OMNIRIG_COM_FAILED_START);
    }
    else
    {
        trace(QString("Omnirig: COM server started"));

    }

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
    trace(QString("Omnirig: Software Version %1").arg(v));


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
            trace(QString("Omnirig: Rig %1 failed to initialise").arg(QString::number(rig_number)));
            return omnirigError(OMNIRIG_ONE_FAILED_INITIALISE);
        }
        else
        {
            trace(QString("Omnirig: Rig %1 initialised").arg(QString::number(rig_number)));
            return omnirigError(OMNIRIG_TWO_FAILED_INITIALISE);
        }
    }

    serPort = (new OmniRig::PortBits(rig->PortBits()));


    rig_type = rig->RigType ();
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



    // also allow time to get freq
    traceMsg(QString("Rig Init - try to get freq"));
    auto f = rig->GetRxFrequency();
    traceMsg(QString("Rig Init - first getFres = %1").arg(QString::number(f)));
    for (int i = 0; (f == 0) && (i < 10); i++)
    {
        traceMsg(QString("Rig Init getFreq i = %1").arg(i));
        f = rig->GetRxFrequency ();
        if (f != 0)
        {
            break;
        }
        QThread::msleep(500);
    }

    traceMsg(QString("Rig Init - freq after delay = %1").arg(QString::number(f)));

    if (f != 0)
    {
        setRigConnected(true);
        traceMsg(QString(" Rig init, get freq = %1").arg(QString::number(f)));
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
        freq = static_cast<Frequency>(rig->GetRxFrequency());
        traceMsg(QString("GetFrequency %1").arg(QString::number(freq)));
        return omnirigError(OMNIRIG_OK);
    }

    return omnirigError(OMNIRIG_OFFLINE);
}

int OmnirigControl::setFrequency(Frequency freq, VFO vfo)
{
    traceMsg(QString("Set Frequency = %1").arg(QString::number(freq)));
    Q_UNUSED(vfo)
    if (!rig || rig->isNull ())
    {
        return omnirigError(OMNIRIG_RIG_NULL);
    }


    if (rigConnected)
    {
        traceMsg(QString("SetFrequency = %1").arg(QString::number(freq)));
        rig->SetFreq(static_cast<int>(freq));
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
    traceMsg(QString("GetMode = %1").arg(convertModeToQString(mode)));

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
        traceMsg(QString("SetMode = %1").arg(convertModeToQString(mode)));
        OmniRig::RigParamX m = map_mode(mode);
        rig->SetMode(m);

        return omnirigError(OMNIRIG_OK);


}

bool OmnirigControl::supportVolControl(int rigNumber)
{
    // not supported
    return false;
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

bool OmnirigControl::supportSignalStrength(int modelNumber)
{
    Q_UNUSED(modelNumber)
    // not supported
    return false;
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
    QString v = QString::number(omni_rig->SoftwareVersion()).toLocal8Bit ();

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


QString OmnirigControl::getLibraryName()
{
    return QString("Omnirig");
}

int OmnirigControl::getRit(VFO vfo, ShortFreq &ritfreq)
{
    Q_UNUSED(vfo)
    int f = rig->RitOffset();
    ritfreq = static_cast<ShortFreq>(f);
    return omnirigError(OMNIRIG_OK);
}

int OmnirigControl::setRit(VFO vfo, ShortFreq ritfreq)
{
    Q_UNUSED(vfo)
    rig->SetRitOffset(static_cast<int>(ritfreq));
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

bool OmnirigControl::supportReadRit(int rigModelNumber)
{
    Q_UNUSED(rigModelNumber)
    return rig->IsParamReadable(OmniRig::PM_RIT0);

}

bool OmnirigControl::supportWriteRit(int rigModelNumber)
{
    Q_UNUSED(rigModelNumber)
    return rig->IsParamWriteable(OmniRig::PM_RIT0);
}

bool OmnirigControl::supportReadRitState(int rigModelNumber)
{
    Q_UNUSED(rigModelNumber)

    return rig->IsParamReadable(OmniRig::PM_RIT0);

}

bool OmnirigControl::supportWriteRitState(int rigModelNumber)
{
    Q_UNUSED(rigModelNumber)

    return rig->IsParamWriteable(OmniRig::PM_RIT0);
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
