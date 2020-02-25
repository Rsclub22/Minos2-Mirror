/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Omnirig Rig Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2016 - 2020
//
//
//
//
/////////////////////////////////////////////////////////////////////////////



#include <QObject>
#include <QString>
#include "omnirigcontrol.h"


const char* OmnirigControl::omnirigErrorMsg[] =  {QT_TR_NOOP("No Error, operation completed sucessfully"),
                                                QT_TR_NOOP("Omnirig Com Failed to start"),
                                                QT_TR_NOOP("Omnirig rig One failed to initialise"),
                                                QT_TR_NOOP("Omnirig rig Two failed to initialise")
                                                };




namespace
{
  auto constexpr OmniRigOneName = "OmniRig Rig 1";
  auto constexpr OmniRigTwoName = "OmniRig Rig 2";
}

auto OmnirigControl::map_mode (OmniRig::RigParamX param) -> MODE
{
  if (param & OmniRig::PM_CW_U)
    {
      return CW_R;
    }
  else if (param & OmniRig::PM_CW_L)
    {
      return CW;
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
  trace(QString("OmniRigControl unrecognized mode"));
  //throw_qstring (tr ("OmniRig: unrecognized mode"));
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
  return OmniRig::PM_SSB_U; // quieten compiler grumble
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
    rig_number(rig_number_),
    rigConnected(false)
{

}

OmnirigControl::~OmnirigControl()
{

}



void OmnirigControl::onHandleCOMException (int code, QString source, QString desc, QString help)
{
    qDebug() << QString("OmniRig COM/OLE error: %1 at %2: %3 (%4)").arg (QString::number(code)).arg(source). arg(desc). arg(help);
}

void OmnirigControl::onHandleVisibleChange()
{
    if (!omni_rig || omni_rig->isNull ())
        return;
    qDebug() << QString("OmniRig visibility change: visibility = %1").arg(omni_rig->DialogVisible ());
}




void OmnirigControl::onHandleRigTypeChange(int rigNumber)
{
    if (!omni_rig || omni_rig->isNull())
        return;
    qDebug() << QString("OmniRig rig type change: rig = $1").arg(rigNumber);

}


void OmnirigControl::onHandleStatusChange(int)
{
    if (!omni_rig || omni_rig->isNull())
        return;
    qDebug() << QString("OmniRig status change for rig %1").arg (rig_number).toLocal8Bit ();

}


void OmnirigControl::onHandleParamsChange(int rigNumber, int params)
{
    if (!omni_rig || omni_rig->isNull ())
        return;
    traceMsg(QString("OmniRig params change: params = 0x%1 for rig %2")
          .arg (params, 8, 16, QChar ('0'))
          .arg (rig_number).toLocal8Bit ());
         // << "state before:" << state ());
    if (rigNumber == rig_number)
    {
        if (!rig || rig->isNull ())
            return;
        //      starting_ = false;
        //TransceiverState old_state {state ()};
        //auto need_frequency = false;

        if (params & OmniRig::PM_VFOAA)
        {
            qDebug() << QString("OmniRig VFOAA");
            //update_split (false);
            //reversed_ = false;
            //update_rx_frequency (rig_->FreqA ());
            //update_other_frequency (rig_->FreqB ());
        }
        if (params & OmniRig::PM_VFOAB)
         {
            qDebug() << QString("OmniRig VFOAB");
            //update_split (true);
            //reversed_ = false;
            //update_rx_frequency (rig_->FreqA ());
            //update_other_frequency (rig_->FreqB ());
        }
        if (params & OmniRig::PM_VFOBA)
        {
            qDebug() << QString("OmniRig VFOBA");
            //update_split (true);
            //reversed_ = true;
            //update_other_frequency (rig_->FreqA ());
            //update_rx_frequency (rig_->FreqB ());
        }
        if (params & OmniRig::PM_VFOBB)
        {
            qDebug() << QString("OmniRig VFOBB");
            //update_split (false);
            //reversed_ = true;
            //update_other_frequency (rig_->FreqA ());
            //update_rx_frequency (rig_->FreqB ());
        }
        if (params & OmniRig::PM_VFOA)
        {
            qDebug() << QString("OmniRig VFOA");
            //reversed_ = false;
            //need_frequency = true;
        }
        if (params & OmniRig::PM_VFOB)
        {
            qDebug() << QString("OmniRig VFOB");
            //reversed_ = true;
            //need_frequency = true;
        }
        if (params & OmniRig::PM_FREQ)
        {
            qDebug() << QString("OmniRig FREQ");
            //need_frequency = true;
        }
        if (params & OmniRig::PM_FREQA)
        {
            auto f = rig->FreqA ();
            qDebug() << QString("OmniRig FREQA = %1").arg(QString::number(f));
            //if (reversed_)
            //  {
            //    update_other_frequency (f);
            //  }
            //else
            //  {
            //    update_rx_frequency (f);
            //  }
        }
        if (params & OmniRig::PM_FREQB)
        {
            auto f = rig->FreqB ();
            qDebug() << QString("OmniRig FREQB = ").arg(QString::number(f));
            //if (reversed_)
            //  {
            //    update_rx_frequency (f);
            //  }
            //else
            //  {
            //    update_other_frequency (f);
            //  }
        }
/*        if (need_frequency)
          {
            if (readable_params_ & OmniRig::PM_FREQA)
              {
                auto f = rig_->FreqA ();
                if (f)
                  {
                    TRACE_CAT ("OmniRigTransceiver", "FREQA = " << f);
                    if (reversed_)
                      {
                        update_other_frequency (f);
                      }
                    else
                      {
                        update_rx_frequency (f);
                      }
                  }
              }
            if (readable_params_ & OmniRig::PM_FREQB)
              {
                auto f = rig_->FreqB ();
                if (f)
                  {
                    TRACE_CAT ("OmniRigTransceiver", "FREQB = " << f);
                    if (reversed_)
                      {
                        update_rx_frequency (f);
                      }
                    else
                      {
                        update_other_frequency (f);
                      }
                  }
              }
            if (readable_params_ & OmniRig::PM_FREQ && !state ().ptt ())
              {
                auto f = rig_->Freq ();
                if (f)
                  {
                    TRACE_CAT ("OmniRigTransceiver", "FREQ = " << f);
                    update_rx_frequency (f);
                  }
              }
          }
*/      if (params & OmniRig::PM_PITCH)
        {
            qDebug() << QString("OmniRig PITCH");
        }
        if (params & OmniRig::PM_RITOFFSET)
        {
            qDebug() << QString("OmniRig RITOFFSET");
        }
        if (params & OmniRig::PM_RIT0)
        {
            qDebug() << QString("OmniRig RIT0");
        }
        if (params & OmniRig::PM_VFOEQUAL)
        {
            //auto f = readable_params_ & OmniRig::PM_FREQA ? rig->FreqA () : rig->Freq ();
            //auto m = map_mode (rig->Mode ());
            //TRACE_CAT ("OmniRigTransceiver", QString {"VFOEQUAL f=%1 m=%2"}.arg (f).arg (m));
            //update_rx_frequency (f);
            //update_other_frequency (f);
            //update_mode (m);
        }
        if (params & OmniRig::PM_VFOSWAP)
        {
            qDebug() << QString("OmniRig VFOSWAP");
            //auto f = state ().tx_frequency ();
            //update_other_frequency (state ().frequency ());
            //update_rx_frequency (f);
            //update_mode (map_mode (rig_->Mode ()));
        }
        if (params & OmniRig::PM_SPLITON)
        {
            qDebug() << QString("OmniRig SPLITON");
            //update_split (true);
        }
        if (params & OmniRig::PM_SPLITOFF)
        {
            qDebug() << QString("OmniRig SPLITOFF");
            //update_split (false);
        }
        if (params & OmniRig::PM_RITON)
        {
            qDebug() << QString("OmniRig RITON");
        }
        if (params & OmniRig::PM_RITOFF)
        {
            qDebug() << QString("OmniRig RITOFF");
        }
        if (params & OmniRig::PM_XITON)
        {
            qDebug() << QString("OmniRig XITON");
        }
        if (params & OmniRig::PM_XITOFF)
        {
            qDebug() << QString("OmniRig XITOFF");
        }
        if (params & OmniRig::PM_RX)
        {
            qDebug() << QString("OmniRig RX");
            //update_PTT (false);
        }
        if (params & OmniRig::PM_TX)
        {
            qDebug() << QString("OmniRig TX");
            //update_PTT ();
        }
        if (params & OmniRig::PM_CW_U)
        {
            qDebug() << QString("OmniRig CW-R");
            //update_mode (CW_R);
        }
        if (params & OmniRig::PM_CW_L)
        {
            qDebug() << QString("OmniRig CW");
            //update_mode (CW);
        }
        if (params & OmniRig::PM_SSB_U)
        {
            qDebug() << QString("OmniRig USB");
            //update_mode (USB);
        }
        if (params & OmniRig::PM_SSB_L)
        {
            qDebug() << QString("OmniRig LSB");
            //update_mode (LSB);
        }
        if (params & OmniRig::PM_DIG_U)
        {
            qDebug() << QString("OmniRig DATA-U");
            //update_mode (DIG_U);
        }
        if (params & OmniRig::PM_DIG_L)
        {
            qDebug() << QString("OmniRig DATA-L");
            //update_mode (DIG_L);
        }
        if (params & OmniRig::PM_AM)
        {
            qDebug() << QString("OmniRig AM");
            //update_mode (AM);
        }
        if (params & OmniRig::PM_FM)
        {
            qDebug() << QString("OmniRig FM");
            //update_mode (FM);
        }

        //if (old_state != state () || send_update_signal_)
        //  {
        //    update_complete ();
        //    send_update_signal_ = false;
        //  }
        //TRACE_CAT ("OmniRigTransceiver", "OmniRig params change: state after:" << state ());
    }
    //Q_EMIT notified ();

}


void OmnirigControl::onHandleCustomReply(int, QVariant const&, QVariant const&)
{

}


void OmnirigControl::register_rigs(RigFactory::Rigs* rigsList, int id1, int id2)
{
    (*rigsList)[OmniRigOneName] = RigCapabilities(
                id1,
                RigCapConstants::PortType::none,
                "Afreet",
                "Omnirig",
                OmniRigOneName,
                false,
                false,
                false,
                false,
                false,
                false,
                false,
                false,
                false,
                false,
                false);




    (*rigsList)[OmniRigTwoName] = RigCapabilities(
                id2,
                RigCapConstants::PortType::none,
                "Afreet",
                "Omnirig",
                OmniRigTwoName,
                false,
                false,
                false,
                false,
                false,
                false,
                false,
                false,
                false,
                false,
                false);

}



int OmnirigControl::omnirigError(omnirigErrorCode errNum)
{
    return  errNum * -1;
}

int OmnirigControl::rigInit(scatParams &currentRadio, bool useRigCtld)
{
    Q_UNUSED(useRigCtld)

    traceMsg(QString("Omnirig starting"));

    omni_rig = new OmniRig::OmniRigX(this);

    if (omni_rig->isNull())
    {
        traceMsg(QString("Failed to start Omnirig COM server"));
        return omnirigError(OMNIRIG_COM_FAILED_START);
    }
    else
    {
        traceMsg(QString("Omnirig COM server started"));

    }

    // COM/OLE exceptions get signaled
    connect (&*omni_rig, SIGNAL (exception (int, QString, QString, QString)), this, SLOT (handle_COM_exception (int, QString, QString, QString)));

    // IOmniRigXEvent interface signals
    connect (&*omni_rig, SIGNAL (VisibleChange ()), this, SLOT (onHandleVisibleChange()));
    connect (&*omni_rig, SIGNAL (RigTypeChange (int)), this, SLOT (onHandleRigTypeChange(int)));
    connect (&*omni_rig, SIGNAL (StatusChange (int)), this, SLOT (onHandleStatusChange(int)));
    connect (&*omni_rig, SIGNAL (ParamsChange (int, int)), this, SLOT (onHandleParamsChange(int, int)));
    connect (&*omni_rig
             , SIGNAL (CustomReply (int, QVariant const&, QVariant const&))
             , this, SLOT (handle_custom_reply (int, QVariant const&, QVariant const&)));

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


    rig_type = rig->RigType ();
    readable_params = rig->ReadableParams ();
    writable_params = rig->WriteableParams ();

    traceMsg(QString("Rig initial rig type: %1 readable params = 0x%2 writable params = 0x%3 for rig %4")
                .arg (rig_type)
                .arg (readable_params, 8, 16, QChar ('0'))
                .arg (writable_params, 8, 16, QChar ('0'))
                .arg (rig_number).toLocal8Bit ());


    if (OmniRig::ST_ONLINE == rig->Status())
    {
        setRigConnected(true);
        traceMsg(QString("Rig %1 online").arg(QString::number(rig_number)));
    }
    else
    {
        setRigConnected(false);
        traceMsg(QString("Rig %1 offline").arg(QString::number(rig_number)));
    }

    return omnirigError(OMNIRIG_OK);

    //rig->SetFreq(144300000);
    //QThread::msleep (500);
    //qDebug() << QString("Frequency = %1").arg(QString::number(rig->GetRxFrequency()));

}

int OmnirigControl::closeRig()
{

}

int OmnirigControl::getFrequency(VFO vfo, Frequency &)
{

}

int OmnirigControl::setFrequency(Frequency freq, VFO vfo)
{

}

int OmnirigControl::getMode(VFO vfo, MODE &mode)
{

}

int OmnirigControl::setMode(VFO vfo, MODE mode)
{

}



int OmnirigControl::setVolume(VFO vfo, float val)
{

}

int OmnirigControl::getVolume(VFO vfo, float *val)
{

}

int OmnirigControl::getSignalStrength(VFO vfo, int *value)
{

}

QString OmnirigControl::getRigLibVersion()
{

}

QString OmnirigControl::getErrorMsgText(int errorCode)
{

}

int OmnirigControl::getRit(VFO vfo, ShortFreq &ritfreq)
{

}

int OmnirigControl::setRit(VFO vfo, ShortFreq ritfreq)
{

}

int OmnirigControl::setRitState(VFO vfo, bool state)
{

}

int OmnirigControl::getRitState(VFO vfo, bool& state)
{

}

void OmnirigControl::setTraceCommsFlag(bool value)
{

}

void OmnirigControl::setTraceComms(bool value)
{

}

bool OmnirigControl::getTraceComms()
{

}


void OmnirigControl::traceMsg(QString msg)
{
    trace(QString("Omnirig: %1").arg(msg));
}

