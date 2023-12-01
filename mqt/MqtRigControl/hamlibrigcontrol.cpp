/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Hamlib Rig Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2016 - 2023
//
//
//
//
/////////////////////////////////////////////////////////////////////////////



#include "serialCommonData.h"
#include "hamlibrigcontrol.h"
#include "minosNetUtils.h"
#include "MTrace.h"

#ifndef HAMLIB_FILPATHLEN
#define HAMLIB_FILPATHLEN FILPATHLEN
#endif
const char* HamlibRigControl::hamlibErrorMsg[] =  {QT_TR_NOOP("No Error, operation completed sucessfully"),
                                                   QT_TR_NOOP("Invalid parameter"),
                                                   QT_TR_NOOP("Invalid configuration"),
                                                   QT_TR_NOOP("Memory shortage"),
                                                   QT_TR_NOOP("Function not implemented"),
                                                   QT_TR_NOOP("Communication timed out"),
                                                   QT_TR_NOOP("IO error, including open failed"),
                                                   QT_TR_NOOP("Internal Hamlib error"),
                                                   QT_TR_NOOP("Protocol error"),
                                                   QT_TR_NOOP("Command rejected by the rig"),
                                                   QT_TR_NOOP("Command performed, but arg truncated"),
                                                   QT_TR_NOOP("Function not available"),
                                                   QT_TR_NOOP("VFO not targetable"),
                                                   QT_TR_NOOP("Error talking on the bus"),
                                                   QT_TR_NOOP("Collision on the bus"),
                                                   QT_TR_NOOP("NULL RIG handle or any invalid pointer parameter in get arg"),
                                                   QT_TR_NOOP("Invalid VFO"),
                                                   QT_TR_NOOP("RIG_EDOM")};



bool hamlibTraceComms = false;

int debug_callback (enum rig_debug_level_e level, rig_ptr_t /* arg */, char const * format, va_list ap)
{
  Q_UNUSED(level)

  static char constexpr fmt[] = "Hamlib: ";
  QString message = QString::vasprintf (format, ap).trimmed ();

  if (hamlibTraceComms)
  {
      trace(QString("%1 %2").arg(fmt, message));
  }


  return 0;
}

const int RIGCTLD_MODEL_NUMBER = 2;



int register_callback(rig_model_t rig_model, void *callback_data)
{

    RigFactory::Rigs* rigsList = reinterpret_cast<RigFactory::Rigs*> (callback_data);


    RIG *myRig;
    myRig = rig_init(rig_model);
    bool myRigOk = (myRig) ? true : false;



    QString manufacturerName = QString::fromLatin1 (rig_get_caps_cptr (rig_model, RIG_CAPS_MFG_NAME_CPTR)).trimmed ();
    QString modelName = QString::fromLatin1 (rig_get_caps_cptr (rig_model, RIG_CAPS_MODEL_NAME_CPTR)).trimmed ();

    QString key = QString("%1 %2").arg(manufacturerName, modelName);

    auto port_type = RigCapConstants::PortType::none;

    switch(rig_get_caps_int(rig_model, RIG_CAPS_PORT_TYPE))
    {
        case RIG_PORT_SERIAL:
            port_type = RigCapConstants::PortType::serial;
        break;

        case RIG_PORT_NETWORK:
            port_type = RigCapConstants::PortType::network;
        break;

        case RIG_PORT_USB:
            port_type = RigCapConstants::PortType::usb;
        break;
        default:
        {}
    }


    bool supportGetVfo = rig_get_function_ptr(rig_model, RIG_FUNCTION_GET_VFO) ? true:false;
    bool supportSetVfo = rig_get_function_ptr(rig_model, RIG_FUNCTION_SET_VFO) ? true:false;

    bool supportGetRit = rig_get_function_ptr(rig_model, RIG_FUNCTION_GET_RIT) ? true:false;
    bool supportSetRit = rig_get_function_ptr(rig_model, RIG_FUNCTION_SET_RIT) ? true:false;

    bool supportGetRitState = false;
    bool supportSetRitState = false;

    if (myRigOk)
    {

        supportGetRitState = rig_has_get_func(myRig, RIG_FUNC_RIT)  ? true:false;
        supportSetRitState = rig_has_set_func(myRig, RIG_FUNC_RIT)  ? true:false;
    }


    bool supportSMeter = (rig_get_caps_int(rig_model, RIG_CAPS_HAS_GET_LEVEL) & RIG_LEVEL_STRENGTH) == RIG_LEVEL_STRENGTH;

    // hamlib 4.5 - Dummy need to enable PTT to use PTT, so disable for now
    bool supportGetPtt = false;
    bool supportSetPtt = false;

    auto supportPttPortType = RigCapConstants::PttPortType::RIG_PTT_NONE;

    switch (rig_get_caps_int(rig_model, RIG_CAPS_PTT_TYPE))
    {
        case RIG_PTT_NONE:
            supportPttPortType = RigCapConstants::PttPortType::RIG_PTT_NONE;
        break;

        case RIG_PTT_RIG:       // CAT PTT
            supportPttPortType = RigCapConstants::PttPortType::RIG_PTT_RIG;
        break;

        case RIG_PTT_RIG_MICDATA:  // CAT PTT
            supportPttPortType = RigCapConstants::PttPortType::RIG_PTT_RIG_MICDATA;
        break;
        default:
        {}

    }

    if (!key.contains("Hamlib Dummy"))
    {
        supportGetPtt = rig_get_function_ptr(rig_model, RIG_FUNCTION_GET_PTT) ? true:false;
        supportSetPtt = rig_get_function_ptr(rig_model, RIG_FUNCTION_SET_PTT) ? true:false;
    }

    bool supportGetVox = rig_has_get_func(myRig, RIG_FUNC_VOX) ? true:false;
    bool supportSetVox = rig_has_get_func(myRig, RIG_FUNC_VOX) ? true:false;


    bool supportVolume = ((rig_get_caps_int (rig_model, RIG_CAPS_HAS_GET_LEVEL) & RIG_LEVEL_AF) == RIG_LEVEL_AF) &&
                ((rig_get_caps_int (rig_model, RIG_CAPS_HAS_SET_LEVEL) & RIG_LEVEL_AF) == RIG_LEVEL_AF);

    // support Antenna Switch
    bool supportAntSw = (rig_get_function_ptr(rig_model, RIG_FUNCTION_GET_ANT) && rig_get_function_ptr(rig_model, RIG_FUNCTION_SET_ANT)) ? true:false;

    bool supportVoiceMem = rig_get_function_ptr(rig_model, RIG_FUNCTION_SEND_VOICE_MEM) ? true:false;
    bool supportStopVoiceMem = rig_get_function_ptr(rig_model, RIG_FUNCTION_STOP_VOICE_MEM) ? true:false;

    //RIG_MTYPE_VOICE,		/*!< Stored Voice Message */
    //RIG_MTYPE_MORSE

    int startVoiceMemoryNumber = 0;
    int endVoiceMemoryNumber = 0;

    if (supportVoiceMem)
    {
        HamlibRigControl::getNumberVoiceCWMemoryChannels(myRig, startVoiceMemoryNumber, endVoiceMemoryNumber, RIG_MTYPE_VOICE);
    }



    bool supportCwMem = rig_get_function_ptr(rig_model, RIG_FUNCTION_SEND_MORSE) ? true:false;
    bool supportCwMemStop = rig_get_function_ptr(rig_model, RIG_FUNCTION_STOP_MORSE) ? true:false;
    bool supportCwMemWait = rig_get_function_ptr(rig_model, RIG_FUNCTION_SEND_MORSE) ? true:false;

    int startCwMemoryNumber = 0;
    int endCwMemoryNumber = 0;
    if (supportCwMem)
    {
        HamlibRigControl::getNumberVoiceCWMemoryChannels(myRig, startCwMemoryNumber, endCwMemoryNumber, RIG_MTYPE_MORSE);

    }


    // below to get a parameter dump for development... edit required parameters...
    //trace(QString("%1\t%2\t%3\t%4\tportType = %5\tpttPortType = %6").arg(manufacturerName + " " + modelName).arg(rig_model).arg(supportCwMem ? "True" : "False").arg(supportVoiceMem ? "True" : "False").arg(port_type).arg(supportPttPortType));

    (*rigsList)[key] = RigCapabilities(port_type,
                                       manufacturerName,
                                       modelName,
                                       key,
                                       rig_model,
                                       true,                // radio supports lookup supported bands
                                       supportGetVfo,
                                       supportSetVfo,
                                       supportGetRit,       // radio supports get rit
                                       supportSetRit,       // radio supports set rit
                                       supportGetRitState,  // radio supports get rit state
                                       supportSetRitState,  // radio supports set rit state
                                       true,                // radio supports get rit max Khz
                                       supportSMeter,       // radio supports s-meter
                                       supportPttPortType,  // we only support CAT and when NONE Serial HW
                                       supportGetPtt,       // radio supports get Ptt
                                       supportSetPtt,       // radio supports set Ptt
                                       supportGetVox,       // radio supports get Vox State
                                       supportSetVox,       // radio supports set Vox State
                                       supportVolume,       // radio supports volume
                                       supportAntSw,        // radio supports antenna switch
                                       true,                // radio supports RigCtld
                                       supportVoiceMem,        // radio supports Voice Memory
                                       startVoiceMemoryNumber,
                                       endVoiceMemoryNumber,
                                       startCwMemoryNumber,
                                       endCwMemoryNumber,
                                       supportStopVoiceMem,     // radio supports stop Voice Memmory
                                       supportCwMem,            // radio supports Cw Memory
                                       supportCwMemStop,
                                       supportCwMemWait,

                                       true);    // radio poll data

    return 1;

}




HamlibRigControl::HamlibRigControl(QObject *parent) : RigBase(parent)
{
    setRigConnected(false);
}


HamlibRigControl::~HamlibRigControl()
{


}







void HamlibRigControl::register_rigs(RigFactory::Rigs* rigsList)
{
    rig_set_debug_callback (debug_callback, nullptr);

    rig_load_all_backends();
    rig_list_foreach_model(register_callback, rigsList);


}


int HamlibRigControl::rigInit(scatParams &currentRadio, bool useRigCtld)
{
    int retcode;

#if defined Q_OS_WIN32
    QString comport = "\\\\.\\";
#elif defined Q_OS_LINUX
    QString comport = "/dev/";
#elif defined Q_OS_MAC
    QString comport = "/dev/";
#endif

    if (useRigCtld)
    {
        if (currentRadio.rigCtldNetworkAdd.isEmpty() || isHostLocal(currentRadio.rigCtldNetworkAdd))
        {
            currentRadio.rigCtldNetworkAdd = RIGCTLD_LOCAL_HOST_ADDRESS;
        }
        if (currentRadio.rigCtldNetworkPort.isEmpty())
        {
            currentRadio.rigCtldNetworkPort = RIGCTLD_DEFAULT_PORT_ADDRESS;
        }
        my_rig = rig_init(RIGCTLD_MODEL_NUMBER);

    }
    else
    {
        my_rig = rig_init(currentRadio.rigModelNumber);
    }


    if (!my_rig)
    {
        return  -14;
    }



    // load cat params
    if (useRigCtld)
    {
        strncpy(my_rig->state.rigport.pathname, QString(currentRadio.rigCtldNetworkAdd + ":" + currentRadio.rigCtldNetworkPort).toLatin1().data(), HAMLIB_FILPATHLEN - 1);
    }
    else
    {

        if (rig_port_e(currentRadio.portType) == RIG_PORT_SERIAL)
        {
            comport.append(currentRadio.comport);
            strncpy(my_rig->state.rigport.pathname, comport.toLatin1().data(), HAMLIB_FILPATHLEN - 1);
            my_rig->state.rigport.parm.serial.rate = currentRadio.baudrate;
            my_rig->state.rigport.parm.serial.data_bits = currentRadio.databits;
            my_rig->state.rigport.parm.serial.stop_bits = currentRadio.stopbits;
            my_rig->state.rigport.parm.serial.parity = getSerialParityCode(currentRadio.parity);
            my_rig->state.rigport.parm.serial.handshake = getSerialHandshakeCode(currentRadio.handshake);



            if (my_rig->state.rigport.parm.serial.handshake != RIG_HANDSHAKE_HARDWARE)
            {

                if (currentRadio.forceRts)
                {
                    my_rig->state.rigport.parm.serial.rts_state = RIG_SIGNAL_ON;
                }
                else
                {

                    my_rig->state.rigport.parm.serial.rts_state = RIG_SIGNAL_UNSET;
                }
            }

        }
        else if (rig_port_e(currentRadio.portType) == RIG_PORT_NETWORK || rig_port_e(currentRadio.portType) == RIG_PORT_UDP_NETWORK)
        {
            QString netAdd;
            if (currentRadio.networkAdd.isEmpty() || isHostLocal(currentRadio.networkAdd))
            {
                netAdd = RIGCTLD_LOCAL_HOST_ADDRESS;
            }
            else
            {
                netAdd = currentRadio.networkAdd;
            }
            strncpy(my_rig->state.rigport.pathname, QString(netAdd + ":" + currentRadio.networkPort).toLatin1().data(), HAMLIB_FILPATHLEN - 1);
        }
        else if (rig_port_e(currentRadio.portType) == RIG_PORT_NONE)
        {
            strncpy(my_rig->state.rigport.pathname, QString("").toLatin1().data(), HAMLIB_FILPATHLEN - 1);
        }



        if (currentRadio.enablePTT)
        {

            serialCommonData::PTTMethodCodes pttType = static_cast<serialCommonData::PTTMethodCodes>(currentRadio.pttType);

            if (pttType != serialCommonData::PTT_METHOD_CAT && pttType != serialCommonData::PTT_METHOD_NONE)
            {
                if (!currentRadio.pttSerialPort.isEmpty())
                {
#if defined (WIN32)

                setConfigurationParameter("ptt_pathname", ("\\\\.\\" + currentRadio.pttSerialPort).toLatin1 ().data ());

#else
                setConfigurationParameter("ptt_pathname", currentRadio.pttSerialPort.toLatin1().data());
#endif

                }

                if (pttType == serialCommonData::PTT_METHOD_DTR)
                {
                   setConfigurationParameter("ptt_type", "DTR");

                }
                else if (pttType == serialCommonData::PTT_METHOD_RTS)
                {

                    setConfigurationParameter("ptt_type", "RTS");
                }

                setConfigurationParameter("ptt_share", "1");


            }
        }


    }



    if(QString(my_rig->caps->mfg_name) == "Icom")
    {
        if(!currentRadio.civAddress.isEmpty())
        {
            rig_set_conf(my_rig, rig_token_lookup(my_rig, "civaddr"),currentRadio.civAddress.toLatin1());
        }
    }


    retcode = rig_open(my_rig);
    if (retcode >= 0)
    {
        setRigConnected(true);

    }
    else
    {
        setRigConnected(false);
    }

    return retcode;

}


int HamlibRigControl::closeRig()
{

    int retcode;
    if (!my_rig)
    {
        return  -14;
    }

    rig_close(my_rig);

    retcode = rig_cleanup(my_rig);
    setRigConnected(false);

    return retcode;

}

/* ---------------------- Freq ------------------------------------ */


int HamlibRigControl::getFrequency(VFO vfo, Frequency &frequency)
{
    freq_t f;
    int retCode = rig_get_freq(my_rig, hamlibVfoNames[vfo], &f);
    if (retCode >= RIG_OK)
    {
        frequency = Frequency(f);
    }

    return retCode;
}


int HamlibRigControl::setFrequency(const Frequency &frequency, VFO vfo)
{
    return (rig_set_freq(my_rig, hamlibVfoNames[vfo], frequency));
}

/*

bool RigControl::checkFreqValid(freq_t freq, rmode_t mode)
{

    const freq_range_t* freq_range = rig_get_range(my_rig->caps->tx_range_list1, freq, mode);
    return (freq_range != nullptr)? true:false;

}
*/

/* ---------------------- Freq Range ---------------------------------*/

bool HamlibRigControl::checkFreqRange(int rigNumber, const Frequency &freq)
{
    RIG *myRig = rig_init(rigNumber);
    const freq_range_t* freq_range = HamlibRigControl::getFreqRange(myRig, freq);
    return (freq_range != nullptr)? true:false;
}

const freq_range_t* HamlibRigControl::getFreqRange(RIG *myRig, const Frequency &freq)
{
    if (myRig)
    {
        rmode_t mode = convertQStrRmode_t(hamlibData::USB);

        const freq_range_t* freq_range = nullptr;
        qint64 f = freq;
#if defined(RIG_MODEL_IC9700)
        if (myRig->caps->rig_model == RIG_MODEL_IC9700)
        {
             freq_range = rig_get_range(myRig->caps->tx_range_list2, f, mode);
        }
        else
#endif
#if defined(RIG_MODEL_TS2000)
        if (myRig->caps->rig_model == RIG_MODEL_TS2000)
        {
             freq_range = rig_get_range(myRig->caps->tx_range_list2, f, mode);
        }
        else
#endif
#if defined(RIG_MODEL_FTDX10)
        if (myRig->caps->rig_model == RIG_MODEL_FTDX10)
        {
             freq_range = rig_get_range(myRig->caps->tx_range_list2, f, mode);
        }
        else

#endif
#if defined(RIG_MODEL_FTDX101D)
        if (myRig->caps->rig_model == RIG_MODEL_FTDX101D)
        {
             freq_range = rig_get_range(myRig->caps->tx_range_list2, f, mode);
        }
        else

#endif
#if defined(RIG_MODEL_FTDX101MP)
        if (myRig->caps->rig_model == RIG_MODEL_FTDX101MP)
        {
             freq_range = rig_get_range(myRig->caps->tx_range_list2, f, mode);
        }
        else

#endif

        {
            freq_range = rig_get_range(myRig->caps->tx_range_list1, f, mode);
        }

        return freq_range;
    }


    return nullptr;
}



/* ---------------------- Mode ------------------------------------ */


int HamlibRigControl::getMode(VFO vfo, MODE& mode)
{
    int retCode =  rig_get_mode(my_rig, hamlibVfoNames[vfo], &rmode, &rwidth);
    if (retCode >= RIG_OK)
    {
        mode = mapMode(rmode);
    }

    return retCode;

}

int HamlibRigControl::setMode(VFO vfo, MODE mode)
{
    //this should be passband no change!
    // we need to use passband, so set it to normal
    //pbwidth_t pb = passbandNormal(mapMode(mode));
    pbwidth_t pb = RIG_PASSBAND_NOCHANGE;
    return rig_set_mode(my_rig, hamlibVfoNames[vfo], mapMode(mode), pb);
}

bool HamlibRigControl::modeSupported(MODE mode, Frequency f)
{
    const freq_range_t  *fr = getFreqRange(my_rig, f);
    rmode_t rm = mapMode(mode);

    bool supported = fr && ((fr->modes & rm) != 0);
    return supported;
}


// Hamlib conversion



rmode_t HamlibRigControl::convertQStrRmode_t(QString mode)
{
    return rig_parse_mode(mode.toLatin1());
}

MODE HamlibRigControl::mapMode (rmode_t m) const
{
  switch (m)
    {
    case RIG_MODE_AM:
    case RIG_MODE_SAM:
    case RIG_MODE_AMS:
    case RIG_MODE_DSB:
      return AM;

    case RIG_MODE_CW:
      return CW;

    case RIG_MODE_CWR:
      return CW_R;

    case RIG_MODE_USB:
    case RIG_MODE_ECSSUSB:
    case RIG_MODE_SAH:
    case RIG_MODE_FAX:
      return USB;

    case RIG_MODE_LSB:
    case RIG_MODE_ECSSLSB:
    case RIG_MODE_SAL:
      return LSB;

    case RIG_MODE_RTTY:
      return FSK;

    case RIG_MODE_RTTYR:
      return FSK_R;

    case RIG_MODE_PKTLSB:
      return DIG_L;

    case RIG_MODE_PKTUSB:
      return DIG_U;

    case RIG_MODE_FM:
    case RIG_MODE_WFM:
      return FM;

    case RIG_MODE_PKTFM:
      return DIG_FM;

    default:
      return UNK;
    }
}

rmode_t HamlibRigControl::mapMode (MODE mode) const
{
  switch (mode)
    {
    case AM: return RIG_MODE_AM;
    case CW: return RIG_MODE_CW;
    case CW_R: return RIG_MODE_CWR;
    case USB: return RIG_MODE_USB;
    case LSB: return RIG_MODE_LSB;
    case FSK: return RIG_MODE_RTTY;
    case FSK_R: return RIG_MODE_RTTYR;
    case DIG_L: return RIG_MODE_PKTLSB;
    case DIG_U: return RIG_MODE_PKTUSB;
    case FM: return RIG_MODE_FM;
    case DIG_FM: return RIG_MODE_PKTFM;
    default: break;
    }
  return RIG_MODE_USB;	// quieten compiler grumble
}

rmode_t HamlibRigControl::mapMode(QString mode) const
{
    if (mode == hamlibData::AM) return RIG_MODE_AM;
    if (mode == hamlibData::CW) return RIG_MODE_CW;
    if (mode == hamlibData::CWR) return RIG_MODE_CWR;
    if (mode == hamlibData::USB) return RIG_MODE_USB;
    if (mode == hamlibData::LSB) return RIG_MODE_LSB;
    if (mode == hamlibData::RTTY) return RIG_MODE_RTTY;
    if (mode == hamlibData::RTTYR) return RIG_MODE_RTTYR;
    if (mode == hamlibData::PKTLSB) return RIG_MODE_PKTLSB;
    if (mode == hamlibData::PKTUSB) return RIG_MODE_PKTUSB;
    if (mode == hamlibData::FM) return RIG_MODE_FM;
    if (mode == hamlibData::PKTFM) return RIG_MODE_PKTFM;
    else return RIG_MODE_USB;


}


/* ---------------------- VFO ------------------------------------ */
// Note not all radios support reading the VFO


bool HamlibRigControl::supportReadVfo(int rigNumber)
{
    RIG *myRig;
    myRig = rig_init(rigNumber);
    if (myRig)
    {
        if (myRig->caps->get_vfo == nullptr)
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    return false;
}


bool HamlibRigControl::supportWriteVfo(int rigNumber)
{
    RIG *myRig;
    myRig = rig_init(rigNumber);
    if (myRig)
    {
        if (myRig->caps->set_vfo == nullptr)
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    return false;
}

int HamlibRigControl::getVfo(VFO *vfo)
{
    int retCode;
    vfo_t hamlibVfo;
    retCode = rig_get_vfo(my_rig, &hamlibVfo);
    *vfo = convert_Vfo_t_To_VFO(hamlibVfo);
    return retCode;
}



int HamlibRigControl::setVfo(VFO vfo)
{
    return rig_set_vfo(my_rig, convert_VFO_to_vfo_t(vfo));
}




QString HamlibRigControl::convertVfoQStr(vfo_t vfo)
{
    return QString::fromLatin1(rig_strvfo(vfo));
}

VFO HamlibRigControl::convert_Vfo_t_To_VFO(vfo_t vfo)
{
    if (vfo == RIG_VFO_A)
    {
        return VFO::VFOA;
    }
    else if (vfo == RIG_VFO_B)
    {
        return VFO::VFOB;
    }
    else if (vfo == RIG_VFO_CURR)
    {
        return VFO::CURRENT_VFO;
    }

    return VFO::CURRENT_VFO;
}

vfo_t HamlibRigControl::convert_VFO_to_vfo_t(VFO vfo)
{
    if (vfo == VFO::VFOA)
    {
        return RIG_VFO_A;
    }
    else if (vfo == VFO::VFOB)
    {
        return RIG_VFO_B;
    }
    else if (vfo == VFO::CURRENT_VFO)
    {
        return RIG_VFO_CURR;
    }

    return RIG_VFO_CURR;
}


/*************** RIT ********************************/






int HamlibRigControl::getRit(VFO vfo, ShortFreq &ritfreq)
{
    shortfreq_t freq;
    int retCode = rig_get_rit(my_rig, hamlibVfoNames[vfo], &freq);
    if (retCode == RIG_OK)
    {
        ritfreq = ShortFreq(freq);
    }

    return retCode;
}

int HamlibRigControl::setRit(VFO vfo, const ShortFreq &ritfreq)
{
    return rig_set_rit(my_rig, hamlibVfoNames[vfo], qint32(ritfreq));
}

int HamlibRigControl::clearRit(VFO vfo)
{
    return rig_set_rit(my_rig, hamlibVfoNames[vfo], 0);
}


bool HamlibRigControl::supportReadRit(int rigNumber)
{
    RIG *myRig;
    myRig = rig_init(rigNumber);
    if (myRig)
    {
        if (myRig->caps->get_rit == nullptr)
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    return false;
}


bool HamlibRigControl::supportWriteRit(int rigNumber)
{
    RIG *myRig;
    myRig = rig_init(rigNumber);
    if (myRig)
    {
        if (myRig->caps->set_rit == nullptr)
        {
            return false;

        }
        else
        {
            return true;

        }
    }
    return false;
}




int HamlibRigControl::setRitState(VFO vfo, bool state)
{
    return rig_set_func(my_rig, hamlibVfoNames[vfo], RIG_FUNC_RIT, state);
}




int HamlibRigControl::getRitState(VFO vfo, bool& state)
{
    int status = 0;
    int retCode = RIG_OK;
    retCode = rig_get_func(my_rig, hamlibVfoNames[vfo], RIG_FUNC_RIT, &status);
    state = status ? true : false;
    return retCode;
}



bool HamlibRigControl::supportWriteRitState(int rigNumber)
{
    RIG *myRig;
    myRig = rig_init(rigNumber);
    setting_t state =  rig_has_set_func(myRig, RIG_FUNC_RIT);
    if (state & RIG_FUNC_RIT)
    {
        return true;
    }

    return false;

}




bool HamlibRigControl::supportReadRitState(int rigNumber)
{
    RIG *myRig;
    myRig = rig_init(rigNumber);
    setting_t state = rig_has_get_func(myRig, RIG_FUNC_RIT);
    if (state & RIG_FUNC_RIT)
    {
        return true;
    }

    return false;
}

int HamlibRigControl::getMaxRitFreq(int rigNumber)
{
    RIG *myRig;
    myRig = rig_init(rigNumber);
    return static_cast<int>(myRig->caps->max_rit);
}

/*************** PTT Control  ********************************/


int  HamlibRigControl::getPttStatus(VFO vfo, bool& state)
{
    ptt_t pttStatus;
    int retCode = rig_get_ptt(my_rig, hamlibVfoNames[vfo], &pttStatus);

    if (pttStatus == RIG_PTT_ON )
    {
        state = true;
    }
    else if (pttStatus == RIG_PTT_OFF)
    {
        state = false;
    }

    return retCode;
}


int HamlibRigControl::setPtt(VFO vfo, bool state)
{

    ptt_t pttState;
    if (state)
    {
        pttState = RIG_PTT_ON;
    }
    else
    {
        pttState = RIG_PTT_OFF;
    }
    int retcode = rig_set_ptt(my_rig, hamlibVfoNames[vfo], pttState);

    return retcode;

}

bool HamlibRigControl::supportGetPtt(int rigNumber)
{
    RIG *myRig;
    myRig = rig_init(rigNumber);
    return myRig->caps->get_ptt ? true:false;
}

bool HamlibRigControl::supportSetPtt(int rigNumber)
{
    RIG *myRig;
    myRig = rig_init(rigNumber);
    return myRig->caps->set_ptt ? true:false;
}

/************* VOX *********************/



bool HamlibRigControl::supportSetVox(int rigNumber)
{
    RIG *myRig;
    myRig= rig_init(rigNumber);
    setting_t state = rig_has_set_func(myRig, RIG_FUNC_VOX);
    if (state & RIG_FUNC_VOX)
    {
        return true;
    }

    return false;
}

bool HamlibRigControl::supportGetVoX(int rigNumber)
{
    RIG *myRig;
    myRig= rig_init(rigNumber);
    setting_t state = rig_has_get_func(myRig, RIG_FUNC_VOX);
    if (state & RIG_FUNC_VOX)
    {
        return true;
    }

    return false;
}

int HamlibRigControl::setVoxState(VFO vfo, bool state)
{
    return rig_set_func(my_rig, hamlibVfoNames[vfo], RIG_FUNC_VOX, state);
}

int HamlibRigControl::getVoxState(VFO vfo, bool &state)
{
    int status = 0;
    int retCode = RIG_OK;
    retCode = rig_get_func(my_rig, hamlibVfoNames[vfo], RIG_FUNC_VOX, &status);
    state = status ? true : false;
    return retCode;
}




/*************** Passband ********************************/





pbwidth_t HamlibRigControl::passbandNarrow(rmode_t mode)
{
    return rig_passband_narrow(my_rig, mode);
}

pbwidth_t HamlibRigControl::passbandNormal(rmode_t mode)
{
    return rig_passband_normal(my_rig, mode);
}

pbwidth_t HamlibRigControl::passbandWide(rmode_t mode)
{
    return rig_passband_wide(my_rig, mode);
}

/*************** Volume Level Control  ********************************/


bool HamlibRigControl::supportVolControl(int rigNumber)
{
    //if (rigNumber == 237)   // if rig is TS590SG ignore volume as it has a bug...
    //{
    //    return false;
    //}

    if ((rigHasGetLevel(rigNumber, RIG_LEVEL_AF) == RIG_LEVEL_AF) && (rigHasSetLevel(rigNumber, RIG_LEVEL_AF) == RIG_LEVEL_AF))
    {
        return true;
    }
    else
    {

        return false;
    }
}


bool HamlibRigControl::supportVolume(int rigNumber)
{
    //if (rigNumber == 237)   // if rig is TS590SG ignore volume as it has a bug...
    //{
    //    return false;
    //}

    if ((rigHasGetLevel(rigNumber, RIG_LEVEL_AF) == RIG_LEVEL_AF) && (rigHasSetLevel(rigNumber, RIG_LEVEL_AF) == RIG_LEVEL_AF))
    {
        return true;
    }
    else
    {

        return false;
    }
}

int HamlibRigControl::setVolume(VFO vfo, float val)
{
    value_t value;
    value.f = val;
    return rigSetLevel(hamlibVfoNames[vfo], RIG_LEVEL_AF, value);
}

int HamlibRigControl::getVolume(VFO vfo, float *val)
{
    value_t value;
    int retCode = RIG_OK;
    retCode = rigGetLevel(hamlibVfoNames[vfo], RIG_LEVEL_AF, &value);
    if (retCode == RIG_OK)
    {
        *val = value.f;
    }
    return retCode;
}


/*************** Signal Strength Level Control  ********************************/


bool HamlibRigControl::supportSignalStrength(int modelNumber)
{

    return rigHasGetLevel(modelNumber, RIG_LEVEL_STRENGTH);
}

/*
bool HamlibRigControl::supportSMeter(int modelNumber)
{
    return rigHasGetLevel(modelNumber, RIG_LEVEL_STRENGTH);
}
*/
int HamlibRigControl::getSignalStrength(VFO vfo, int *value)
{
    int retCode = RIG_OK;
    value_t val;
    retCode = rigGetLevel(hamlibVfoNames[vfo], RIG_LEVEL_STRENGTH, &val);
    if (retCode == RIG_OK)
    {
        *value = val.i;
    }

    return retCode;
}

/*************** Voice Memory Control  ********************************/


int HamlibRigControl::sendVoiceMessage(VFO vfo, int vmNum)
{

    return rig_send_voice_mem(my_rig, hamlibVfoNames[vfo], vmNum);

}

bool HamlibRigControl::supportVoiceMemory(int rigNumber)
{
    RIG *myRig;
    myRig = rig_init(rigNumber);
    if (myRig)
    {
        return my_rig->caps->send_voice_mem ? true:false;
    }

    return false;

}

// this is to support stop on TS890S. It does not use voice message number 0 to stop the message
// but sends a seperate parameter 1 to start message, 0 to stop message. hamlib remembers the message
// number.

int HamlibRigControl::stop_voice_mem(VFO vfo)
{
    return rig_stop_voice_mem(my_rig, hamlibVfoNames[vfo]);
}

bool HamlibRigControl::supportStopVoiceMem(int rigNumber)
{
    RIG *myRig;
    myRig = rig_init(rigNumber);
    if (myRig)
    {
        return my_rig->caps->stop_voice_mem ? true:false;
    }

    return false;

}


/*************** CW Memory Control  ********************************/

int HamlibRigControl::sendMorse(VFO vfo, QString msg)
{
    return rig_send_morse(my_rig, hamlibVfoNames[vfo], msg.toLocal8Bit());
}
int HamlibRigControl::stopMorse(VFO vfo)
{
    return rig_stop_morse(my_rig, hamlibVfoNames[vfo]);
}
//int HamlibRigControl::waitMorsePtt(VFO vfo)
//{

//    return wait_morse_ptt(my_rig, hamlibVfoNames[vfo]);
//}
int HamlibRigControl::waitMorse(VFO vfo)
{

    return rig_wait_morse(my_rig, hamlibVfoNames[vfo]);
}

bool HamlibRigControl::supportSendMorse(int rigNumber)
{

    RIG *myRig;
    myRig = rig_init(rigNumber);
    if (myRig)
    {
        return my_rig->caps->send_morse ? true:false;

    }

    return false;

}

bool HamlibRigControl::supportStopMorse(int rigNumber)
{

    RIG *myRig;
    myRig = rig_init(rigNumber);
    if (myRig)
    {
        return my_rig->caps->stop_morse ? true:false;

    }

    return false;

}

bool HamlibRigControl::supportWaitMorse(int rigNumber)
{

    RIG *myRig;
    myRig = rig_init(rigNumber);
    if (myRig)
    {
        return my_rig->caps->wait_morse ? true:false;

    }

    return false;

}

/***************** Memory Channels *****************************/


bool HamlibRigControl::getNumberVoiceCWMemoryChannels(RIG *myRig, int &startNumMem, int &endNumMem, chan_type_t channelType)
{
    if (myRig)
    {

        chan_t* chan_list = myRig->caps->chan_list;
        for (int i = 0; !RIG_IS_CHAN_END(chan_list[i]) && i < HAMLIB_CHANLSTSIZ; i++)
        {
            if (chan_list[i].type == channelType)
            {
                startNumMem = chan_list[i].startc;
                endNumMem = chan_list[i].endc;
                return true;
            }
        }


    }



    return false;


}

/*
int HamlibRigControl::getAllMemoryChannelData(RIG * rig, VFO vfo, channel_t chans[])
{
    int ok = -1;

    ok = rig_get_chan_all(rig, hamlibVfoNames[vfo], chans);
    return ok;

}
*/
/*************** Level Control  ********************************/


setting_t HamlibRigControl::rigHasGetLevel(setting_t level)
{
    return rig_has_get_level (my_rig, level);
}

setting_t HamlibRigControl::rigHasGetLevel(int rigNumber, setting_t level)
{

    RIG *myRig;
    myRig = rig_init(rigNumber);
    if (myRig)
    {
        return rig_has_get_level (myRig, level);
    }
    else
    {
        return 0;
    }

}

setting_t HamlibRigControl::rigHasSetLevel(setting_t level)
{
    return rig_has_set_level (my_rig, level);
}

setting_t HamlibRigControl::rigHasSetLevel(int rigNumber, setting_t level)
{

    RIG *myRig;
    myRig = rig_init(rigNumber);
    if (myRig)
    {
        return rig_has_set_level (myRig, level);
    }
    else
    {
        return 0;
    }
}


int HamlibRigControl::rigSetLevel(vfo_t vfo, setting_t level, value_t val)
{
    return rig_set_level (my_rig, vfo, level, val);
}

int HamlibRigControl::rigGetLevel(vfo_t vfo, setting_t level, value_t *val)
{
    return rig_get_level (my_rig, vfo, level, val);
}

/********************* Get/Set Func **********************************/


setting_t HamlibRigControl::rigHasGetFunc(setting_t func)
{
   return rig_has_get_func(my_rig, func);
}

setting_t HamlibRigControl::rigHasGetFunc(int rigNumber, setting_t func)
{
    RIG *myRig;
    myRig = rig_init(rigNumber);
    if (myRig)
    {
        return rig_has_get_func( myRig, func);
    }
    else
    {
        return 0;
    }
}

setting_t HamlibRigControl::rigHasSetFunc(setting_t func)
{
    return rig_has_set_func (my_rig, func);
}

setting_t HamlibRigControl::rigHasSetFunc(int rigNumber, setting_t func)
{
    RIG *myRig;
    myRig = rig_init(rigNumber);
    if (myRig)
    {
        return rig_has_set_func (myRig, func);
    }
    else
    {
        return 0;
    }
}


/********** set_conf ***********************/

// set configuration parameters..



int HamlibRigControl::setConfigurationParameter(QString cfgparam, QString value)
{


    return  rig_set_conf(my_rig, rig_token_lookup(my_rig, cfgparam.toLatin1().data()), value.toLatin1().data());



}

int HamlibRigControl::getConfigurationParameter(QString cfgparam, QString* value)
{

    char* val = nullptr;
    int retCode =  rig_get_conf(my_rig, rig_token_lookup(my_rig, cfgparam.toLatin1().data()), val);

    value->fromLatin1(val);

    return retCode;

}




/*
void RigControl::getRigList()
{

    if(!riglistLoaded)
    {
        capsList.clear();
        rig_load_all_backends();
        rig_list_foreach(collect,nullptr);
        qSort(capsList.begin(),capsList.end(),model_Sort);
        riglistLoaded=true;
    }
}



bool RigControl::getRigList(QComboBox *cb)
{
    int i;
    rig_port_e portType = RIG_PORT_NONE;

    if(capsList.count()==0) return false;
    QStringList sl;
    // add blank at beginning
    //sl << "";
    for (i = 0;i < capsList.count(); i++)
    {

        QString t;
        t = QString::number(capsList.at(i)->rig_model);
        if (t.trimmed() != "2")      // don't display rigctl model
        {
            t = t.rightJustified(5,' ')+", ";
            t+= capsList.at(i)->mfg_name;
            t+= ", ";
            t+= capsList.at(i)->model_name;
            if (getPortType(capsList.at(i)->rig_model, &portType) != -1)
            {
                if (portType == RIG_PORT_NONE || portType == RIG_PORT_SERIAL  || portType == RIG_PORT_NETWORK || portType == RIG_PORT_UDP_NETWORK)
                {
                    sl << t;        // only add these portTypes
                }
            }

        }

   }
   std::sort(sl.begin(), sl.end());
   cb->addItems(sl);
   return true;
}
*/
/********************** Antenna Switching ---------------------------------*/

/*
int RigControl::getAntSwNum(vfo_t vfo)
{
    int antNum = 0;
    int retCode = 0;
    // api has changed *********************************************************
    //retCode = rig_get_ant(my_rig, vfo, &antNum);
    //if (retCode < 0)
    //{
    //    return retCode;
    //}

    return antNum;

}



int RigControl::setAntSwNum(vfo_t vfo, ant_t antNum)
{
    int retCode = 0;
    // api has changed *********************************************************
    //retCode = rig_set_ant(my_rig, vfo, antNum);
    return retCode;
}


int HamlibRigControl::supportAntSw(int rigNumber, bool *antSwFlag)
{
    int retCode = RIG_OK;
    RIG *myRig;
    myRig = rig_init(rigNumber);
    if (myRig)
    {
        if (myRig->caps->get_ant == nullptr || myRig->caps->set_ant == nullptr)
        {
            *antSwFlag = false;
            return retCode;
        }
        else
        {
            *antSwFlag = true;
            return retCode;
        }
    }

    return retCode = -14;

}
*/

/**************************************** ***********************************************/

/*
int RigControl::getPortType(int rigNumber, rig_port_e *portType)
{

    int retCode = 0;
    RIG *my_rig;
    my_rig = rig_init(rigNumber);
    if (my_rig != nullptr)
    {
        *portType = my_rig->caps->port_type;
        return retCode;
    }

    retCode = -1;
    return retCode;

}

// true set DTR state, false unset DTR state
int RigControl::setDtrState( const bool state)
{
    int retCode = 0;
    retCode = rig_set_conf(my_rig, rig_token_lookup(my_rig, "dtr_state"), state ? "ON" : "OFF");

    return retCode;
}


// true set RTS state, false unset RTS state
int RigControl::setRtsState( const bool state)
{
    int retCode = 0;
    retCode = rig_set_conf(my_rig, rig_token_lookup(my_rig, "rts_state"), state ? "ON" : "OFF");
    return retCode;
}




int RigControl::setRetryNumber(const QString retries)
{
    return rig_set_conf(my_rig, rig_token_lookup(my_rig, "retry") , retries.toLatin1().data());

}

int RigControl::setTimeoutDur(const QString timeoutDur)
{
    return rig_set_conf(my_rig, rig_token_lookup(my_rig, "timeout") , timeoutDur.toLatin1().data());

}





int RigControl::getModelInfo(QString radioModel, int *radioModelNumber, QString *radioMfgName, QString *radioModelName)
{
    bool ok;
    int number;
    QStringList modelInfo = radioModel.remove('\x20').split(',');
    if (modelInfo.length() == 3)
    {
        number = modelInfo[0].toInt(&ok);
        if (!ok)
        {
           return -1;
        }

        *radioModelNumber = number;

        //modelInfo = modelInfo[1].split(',');
        //if (modelInfo.length() > 0)

        *radioMfgName = modelInfo[1].trimmed();
        *radioModelName = modelInfo[2].trimmed();
        return 0;


    }

    return -1;

}

*/

/*

const char * RigControl::getModel_Name(int idx)
{

    if(idx<0) return 0;
    return capsList.at(idx)->model_name;
}


int RigControl::getRigModelIndex()
{
    int i;
    QString t=catParams.radioModel;
    t=t.remove(0,5);
    t=t.simplified();
    QStringList sl=t.split(",");
    if(sl.count()==1) sl.append("");
    for(i=0;i<capsList.count();i++)
    {
        if((capsList.at(i)->mfg_name==sl.at(0)) && (capsList.at(i)->model_name==sl.at(1)))
        {
            return i;
        }
    }
    return -1;
}


*/



 QString HamlibRigControl::getErrorMsgText(int errorCode)
 {

     if (errorCode > static_cast<int>(sizeof(hamlibErrorMsg)/sizeof(const char *)))
     {
         return tr("hamlib Errorcode too large!");
     }
     return tr(hamlibErrorMsg[errorCode]);
 }


QString HamlibRigControl::getLibraryName()
{
    return QString("hamlib");
}

//QStringList RigControl::gethamlibErrorMsg()
//{

//    return serialData::hamlibErrorMsg;
//}

QString HamlibRigControl::getRigLibVersion()
{
    QString ver = hamlib_version;
    return ver;
}


bool model_Sort(const rig_caps *caps1,const rig_caps *caps2)
{
    if(caps1->mfg_name==caps2->mfg_name)
    {
        if (QString::compare(caps1->model_name,caps2->model_name)<0) return true;
        return false;
    }
    if (QString::compare(caps1->mfg_name,caps2->mfg_name)<0) return true;
    return false;
}


// static to init flag
void HamlibRigControl::setTraceCommsFlag(bool value)
{
    hamlibTraceComms = value;
}

void HamlibRigControl::setTraceComms(bool value)
{
    hamlibTraceComms = value;
}

bool HamlibRigControl::getTraceComms()
{
    return hamlibTraceComms;
}

void HamlibRigControl::setRigConnected(bool rigConnected_)
{
    trace(QString("Hamlib: setRigConnected = %1").arg(rigConnected_ ? "true" : "false"));
    rigConnected = rigConnected_;
}

bool HamlibRigControl::getRigConnected()
{
    trace(QString("Hamlib: getRigConnected = %1").arg(rigConnected ? "true" : "false"));
    return rigConnected;
}


