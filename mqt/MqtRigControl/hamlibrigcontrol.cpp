/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Hamlib Rig Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2016 - 2020
//
//
//
//
/////////////////////////////////////////////////////////////////////////////



#include "hamlibrigcontrol.h"
#include "minosNetUtils.h"


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


static QList<const rig_caps *> capsList;

int collect(const rig_caps *caps, rig_ptr_t)
{
    capsList.append(caps);
    return 1;
}

bool hamlibTraceComms = false;

int debug_callback (enum rig_debug_level_e level, rig_ptr_t /* arg */, char const * format, va_list ap)
{
  Q_UNUSED(level)
   QString message;

  static char constexpr fmt[] = "Hamlib: ";
  message = message.vsprintf (format, ap).trimmed ();

  if (hamlibTraceComms)
  {
      trace(QString("%1 %2").arg(fmt).arg(message));
  }


  return 0;
}

const int RIGCTLD_MODEL_NUMBER = 2;

extern "C"
{
  //typedef struct rot RIG;
  struct rig_caps;
  //typedef int vfo_t;
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

    capsList.clear();
    rig_load_all_backends();
    rig_list_foreach(collect, nullptr);

    QString key;

    for (int i = 0; i < capsList.count(); i++)
    {
        key = QString("%1 %2").arg(capsList[i]->mfg_name).arg(capsList[i]->model_name);
        auto port_type = RigCapConstants::PortType::none;
        switch(capsList[i]->port_type)
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

        //bool supportGetRit = capsList[i]->get_rit ? true:false;

        //bool supportSetRit = capsList[i]->set_rit ? true:false;

        //bool supportGetRitState = rigHasGetFunc(capsList[i]->rig_model, RIG_FUNC_RIT)  ? true:false;
        //bool supportSetRitState = rigHasSetFunc(capsList[i]->rig_model, RIG_FUNC_RIT)  ? true:false;

        //bool supportSMeter = HamlibRigControl::supportSignalStrength(capsList[i]->rig_model);

        //bool supportGetPtt = capsList[i]->get_ptt ? true:false;

        //bool supportSetPtt = capsList[i]->set_ptt ? true:false;

        //bool supportVolume = false;

        //
       //if (capsList[i]->rig_model != RIG_MODEL_TS590SG || capsList[i]->rig_model != RIG_MODEL_TS590S) // if rig is TS590G ignore volume as it has a bug..
        //{
        //    if ((HamlibRigControl::rigHasGetLevel(capsList[i]->rig_model, RIG_LEVEL_AF) == RIG_LEVEL_AF) && (HamlibRigControl::rigHasSetLevel(capsList[i]->rig_model, RIG_LEVEL_AF) == RIG_LEVEL_AF))
        //    {
        //        supportVolume =  true;
        //    }

        //}

        // support Antenna Switch
        bool supportAntSw = (capsList[i]->get_ant && capsList[i]->set_ant) ? true:false;

        (*rigsList)[key] = RigCapabilities(port_type,
                                           capsList[i]->mfg_name,
                                           capsList[i]->model_name,
                                           key,
                                           capsList[i]->rig_model,
                                           true,                // supports lookup supported bands
                                           true,       // support get rit
                                           true,       // support set rit
                                           true,        // support get rit state
                                           true,        // support set rit state
                                           true,       // support s-meter
                                           true,       // support get Ptt
                                           true,       // support set Ptt
                                           true,       // support volume
                                           true,        // support antenna switch
                                           true,            // support RigCtld
                                           true);    // support poll data
    }




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
        return retcode = -14;
    }

    // load cat params
    if (useRigCtld)
    {
        strncpy(my_rig->state.rigport.pathname, QString(currentRadio.rigCtldNetworkAdd + ":" + currentRadio.rigCtldNetworkPort).toLatin1().data(), FILPATHLEN);
    }
    else
    {

        if (rig_port_e(currentRadio.portType) == RIG_PORT_SERIAL)
        {
            comport.append(currentRadio.comport);
            strncpy(my_rig->state.rigport.pathname, comport.toLatin1().data(), FILPATHLEN);
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
            strncpy(my_rig->state.rigport.pathname, QString(netAdd + ":" + currentRadio.networkPort).toLatin1().data(), FILPATHLEN);
        }
        else if (rig_port_e(currentRadio.portType) == RIG_PORT_NONE)
        {
            strncpy(my_rig->state.rigport.pathname, QString("").toLatin1().data(), FILPATHLEN);
        }


    }



    if(QString(my_rig->caps->mfg_name) == "Icom")
    {
        if(!currentRadio.civAddress.isEmpty())
        {
            retcode = rig_set_conf(my_rig, rig_token_lookup(my_rig, "civaddr"),currentRadio.civAddress.toLatin1());
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
        return retcode = -14;
    }

    retcode = rig_close(my_rig);

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
        frequency = static_cast<Frequency>(f);
    }

    return retCode;
}


int HamlibRigControl::setFrequency(Frequency frequency, VFO vfo)
{
    return (rig_set_freq(my_rig, hamlibVfoNames[vfo], static_cast<freq_t>(frequency)));
}

/*

bool RigControl::checkFreqValid(freq_t freq, rmode_t mode)
{

    const freq_range_t* freq_range = rig_get_range(my_rig->caps->tx_range_list1, freq, mode);
    return (freq_range != nullptr)? true:false;

}
*/

/* ---------------------- Freq Range ---------------------------------*/





bool HamlibRigControl::checkFreqRange(int rigNumber, Frequency freq)
{

    RIG *myRig = rig_init(rigNumber);
    if (myRig)
    {
        rmode_t mode = convertQStrRmode_t("USB");

        const freq_range_t* freq_range = nullptr;
        if (myRig->caps->rig_model == RIG_MODEL_IC9700)
        {
             freq_range = rig_get_range(myRig->caps->tx_range_list2, freq, mode);
        }
        else
        {
            freq_range = rig_get_range(myRig->caps->tx_range_list1, freq, mode);
        }

        return (freq_range != nullptr)? true:false;
    }


    return false;



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
    return rig_set_mode(my_rig, hamlibVfoNames[vfo], mapMode(mode), rwidth);
}




// Hamlib conversion



rmode_t HamlibRigControl::convertQStrRmode_t(QString mode)
{
    return rig_parse_mode(mode.toLatin1());
}

/*

// rigControl conversion

int RigControl::rigConvertQStrMode(QString mode)
{
    for (int i = 0; i < hamlibData::supModeList.count(); i++)
    {
        if (mode == hamlibData::supModeList[i])
        {
            return i;
        }
    }
    return -1; //not found
}

*/


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
    if (mode == "AM") return RIG_MODE_AM;
    if (mode == "CW") return RIG_MODE_CW;
    if (mode == "CW_R") return RIG_MODE_CWR;
    if (mode == "USB") return RIG_MODE_USB;
    if (mode == "LSB") return RIG_MODE_LSB;
    if (mode == "FSK") return RIG_MODE_RTTY;
    if (mode == "FSK_R") return RIG_MODE_RTTYR;
    if (mode == "DIG_L") return RIG_MODE_PKTLSB;
    if (mode == "DIG_U") return RIG_MODE_PKTUSB;
    if (mode == "FM") return RIG_MODE_FM;
    if (mode == "DIG_FM") return RIG_MODE_PKTFM;
    else return RIG_MODE_USB;


}


/* ---------------------- VFO ------------------------------------ */
// Note not all radios support reading the VFO

/*
int RigControl::getVfo(vfo_t *vfo)
{

    return rig_get_vfo(my_rig, vfo);
}



int RigControl::setVfo(vfo_t vfo)
{
    return rig_set_vfo(my_rig, vfo);
}




QString RigControl::convertVfoQStr(vfo_t vfo)
{
    return QString::fromLatin1(rig_strvfo(vfo));
}
*/

/*************** RIT ********************************/





int HamlibRigControl::getRit(VFO vfo, ShortFreq &ritfreq)
{
    shortfreq_t freq;
    int retCode = rig_get_rit(my_rig, hamlibVfoNames[vfo], &freq);
    if (retCode == RIG_OK)
    {
        ritfreq = static_cast<ShortFreq>(freq);
    }

    return retCode;
}

int HamlibRigControl::setRit(VFO vfo, ShortFreq ritfreq)
{
    return rig_set_rit(my_rig, hamlibVfoNames[vfo], static_cast<shortfreq_t>(ritfreq));
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

/*************** PTT Control  ********************************/


/*
int  RigControl::getPttStatus(vfo_t vfo, ptt_t *pttStatus)
{
    return rig_get_ptt	(my_rig, vfo, pttStatus);
}


int RigControl::setPtt(vfo_t vfo, ptt_t ptt)
{
    return rig_set_ptt(my_rig, vfo, ptt);
}


*/


/*************** Passband ********************************/



/*

pbwidth_t RigControl::passbandNarrow(rmode_t mode)
{
    return rig_passband_narrow(my_rig, mode);
}

pbwidth_t RigControl::passbandNormal(rmode_t mode)
{
    return rig_passband_normal(my_rig, mode);
}

pbwidth_t RigControl::passbandWide(rmode_t mode)
{
    return rig_passband_wide(my_rig, mode);
}


void RigControl::buildPassBandTable(QString mgmMode)
{

    passBandWidth[0][0] = passbandNarrow(convertQStrMode("CW"));
    passBandWidth[0][1] = passbandNormal(convertQStrMode("CW"));
    passBandWidth[0][2] = passbandWide(convertQStrMode("CW"));

    passBandWidth[1][0] = passbandNarrow(convertQStrMode("USB"));
    passBandWidth[1][1] = passbandNormal(convertQStrMode("USB"));
    passBandWidth[1][2] = passbandWide(convertQStrMode("USB"));

    passBandWidth[2][0] = passbandNarrow(convertQStrMode("FM"));
    passBandWidth[2][1] = passbandNormal(convertQStrMode("FM"));
    passBandWidth[2][2] = passbandWide(convertQStrMode("FM"));

    passBandWidth[3][0] = passbandNarrow(convertQStrMode(mgmMode));
    passBandWidth[3][1] = passbandNormal(convertQStrMode(mgmMode));
    passBandWidth[3][2] = passbandWide(convertQStrMode(mgmMode));


}


pbwidth_t RigControl::lookUpPassBand(QString mode, int modeState)
{
    int m = -1;

    for (int i=0; i < hamlibData::supModeList.count(); i++)
    {
        if (mode == hamlibData::supModeList[i])
        {
            m = i;
        }
    }
    if (m < 0)
    {
        return 0; //error
    }
    else
    {

        return passBandWidth[m][modeState];
    }
}


void RigControl::setPassBand(QString mode, int modeState)
{
    int imode = rigConvertQStrMode(mode);
    if (imode == -1)
    {
        return;
    }
    else
    {
        pbwidth = passBandWidth[imode][modeState];
    }

}

pbwidth_t RigControl::getPassBand()
{
    return pbwidth;
}
*/

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
        return rig_has_get_level (myRig, func);
    }
    else
    {
        return 0;
    }
}

setting_t HamlibRigControl::rigHasSetFunc(setting_t func)
{
    return rig_has_set_level (my_rig, func);
}

setting_t HamlibRigControl::rigHasSetFunc(int rigNumber, setting_t func)
{
    RIG *myRig;
    myRig = rig_init(rigNumber);
    if (myRig)
    {
        return rig_has_set_level (myRig, func);
    }
    else
    {
        return 0;
    }
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
    trace(QString("Omnirig: setRigConnected = %1").arg(rigConnected_ ? "true" : "false"));
    rigConnected = rigConnected_;
}

bool HamlibRigControl::getRigConnected()
{
    trace(QString("Omnirig: getRigConnected = %1").arg(rigConnected ? "true" : "false"));
    return rigConnected;
}


