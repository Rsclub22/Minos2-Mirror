/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rig Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2017
//
//
// Hamlib Library
//
/////////////////////////////////////////////////////////////////////////////



#include <QList>
#include <QDebug>
#include <QStringList>
#include "rigcontrol.h"
#include <hamlib/rig.h>





static QList<const rig_caps *> capsList;
static bool riglistLoaded=false;





int collect(const rig_caps *caps,rig_ptr_t)
{
    capsList.append(caps);
    return 1;
}



RigControl::RigControl(QObject *parent) : QObject(parent)
{


    // set callback for debug messages
    // NB callback is the C function, not the class method.
    // user_data is used to point to our class.

    rig_set_debug_callback (::rig_message_cb, static_cast<rig_ptr_t>(this));

}



RigControl::~RigControl()
{
    rig_close(my_rig); /* close port */
    rig_cleanup(my_rig); /* if you care about memory */
}

int RigControl::init(scatParams &currentRadio, bool useRigCtld)
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
        if (currentRadio.rigCtldNetworkAdd.isEmpty())
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
        my_rig = rig_init(currentRadio.radioModelNumber);
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
        }
        else if (rig_port_e(currentRadio.portType) == RIG_PORT_NETWORK || rig_port_e(currentRadio.portType) == RIG_PORT_UDP_NETWORK)
        {
            strncpy(my_rig->state.rigport.pathname, QString(currentRadio.networkAdd + ":" + currentRadio.networkPort).toLatin1().data(), FILPATHLEN);
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
        set_serialConnected(true);
    }
    else
    {
        set_serialConnected(false);
    }

    return retcode;

}


int RigControl::closeRig()
{
    int retcode;
    if (!my_rig)
    {
        return retcode = -14;
    }

    retcode = rig_close(my_rig);

    retcode = rig_cleanup(my_rig);
    set_serialConnected(false);
    return retcode;

}

/* ---------------------- Freq ------------------------------------ */


int RigControl::getFrequency(vfo_t vfo, freq_t *frequency)
{
    return rig_get_freq(my_rig, vfo, frequency);
}


int RigControl::setFrequency(freq_t frequency, vfo_t vfo)
{
    return (rig_set_freq(my_rig, vfo, frequency));
}



bool RigControl::checkFreqValid(freq_t freq, rmode_t mode)
{

    const freq_range_t* freq_range = rig_get_range(my_rig->caps->tx_range_list1, freq, mode);
    return (freq_range != nullptr)? true:false;

}


/* ---------------------- Freq Range ---------------------------------*/



bool RigControl::chkFreqRange(RIG *my_rig, freq_t freq, QString modeStr)
{
    rmode_t mode = convertQStrMode(modeStr);
    const freq_range_t* freq_range = rig_get_range(my_rig->caps->rx_range_list1, freq, mode);
    return (freq_range != nullptr)? true:false;

}

/* ---------------------- Mode ------------------------------------ */

int RigControl::getMode(vfo_t vfo, rmode_t *mode, pbwidth_t *width)
{
    return rig_get_mode(my_rig, vfo, mode, width);
}

int RigControl::setMode(vfo_t vfo, rmode_t mode, pbwidth_t passBandwidth)
{
    return (rig_set_mode(my_rig, vfo, mode, passBandwidth));
}

// Hamlib conversion
QString RigControl::convertModeQstr(rmode_t mode)
{
    return QString::fromLatin1(rig_strrmode(mode));
}

rmode_t RigControl::convertQStrMode(QString mode)
{
    return rig_parse_mode(mode.toLatin1());

}

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

/* ---------------------- VFO ------------------------------------ */
// Note not all radios support reading the VFO

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


/*************** RIT ********************************/





int RigControl::getRit(vfo_t vfo, shortfreq_t *ritfreq)
{
    return rig_get_rit(my_rig, vfo, ritfreq);
}

int RigControl::setRit(vfo_t vfo, shortfreq_t ritfreq)
{
    return rig_set_rit(my_rig, vfo, ritfreq);
}

bool RigControl::supportGetRit(int rigNumber)
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


bool RigControl::supportSetRit(int rigNumber)
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



bool RigControl::supportRitOnOff(int rigNumber)
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

int RigControl::toggleRitState(vfo_t vfo, bool state)
{
    return rig_set_func(my_rig, vfo, RIG_FUNC_RIT, state);
}

int RigControl::getRitState(vfo_t vfo, bool* state)
{
    int status = 0;
    int retCode = RIG_OK;
    retCode = rig_get_func(my_rig, vfo, RIG_FUNC_RIT, &status);
    *state = status ? true : false;
    return retCode;
}


bool RigControl::supportGetRitState(int rigNumber)
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



int  RigControl::getPttStatus(vfo_t vfo, ptt_t *pttStatus)
{
    return rig_get_ptt	(my_rig, vfo, pttStatus);
}


int RigControl::setPtt(vfo_t vfo, ptt_t ptt)
{
    return rig_set_ptt(my_rig, vfo, ptt);
}





/*************** Passband ********************************/





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


/*************** Volume Level Control  ********************************/

bool RigControl::supportVolControl(int rigNumber)
{
    if ((rigHasGetLevel(rigNumber, RIG_LEVEL_AF) == RIG_LEVEL_AF) && (rigHasSetLevel(rigNumber, RIG_LEVEL_AF) == RIG_LEVEL_AF))
    {
        return true;
    }
    else
    {

        return false;
    }
}

int RigControl::setVolume(vfo_t vfo, float val)
{
    value_t value;
    value.f = val;
    return rigSetLevel(vfo, RIG_LEVEL_AF, value);
}

int RigControl::getVolume(vfo_t vfo, value_t *val)
{
    return rigGetLevel(vfo, RIG_LEVEL_AF, val);
}


/*************** Signal Strength Level Control  ********************************/

bool RigControl::supportSignalStrength(int modelNumber)
{

    return rigHasGetLevel(modelNumber, RIG_LEVEL_STRENGTH);
}



int RigControl::getSignalStrength(vfo_t vfo, value_t *val)
{
    return rigGetLevel(vfo, RIG_LEVEL_STRENGTH, val);
}



/*************** Level Control  ********************************/

setting_t RigControl::rigHasGetLevel(setting_t level)
{
    return rig_has_get_level (my_rig, level);
}

setting_t RigControl::rigHasGetLevel(int rigNumber, setting_t level)
{

    RIG *myRig;
    myRig = rig_init(rigNumber);
    if (myRig)
    {
        return rig_has_get_level (my_rig, level);
    }
    else
    {
        return 0;
    }

}

setting_t RigControl::rigHasSetLevel(setting_t level)
{
    return rig_has_set_level (my_rig, level);
}

setting_t RigControl::rigHasSetLevel(int rigNumber, setting_t level)
{

    RIG *myRig;
    myRig = rig_init(rigNumber);
    if (myRig)
    {
        return rig_has_set_level (my_rig, level);
    }
    else
    {
        return 0;
    }
}


int RigControl::rigSetLevel(vfo_t vfo, setting_t level, value_t val)
{
    return rig_set_level (my_rig, vfo, level, val);
}

int RigControl::rigGetLevel(vfo_t vfo, setting_t level, value_t *val)
{
    return rig_get_level (my_rig, vfo, level, val);
}



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

/********************** Antenna Switching ---------------------------------*/


int RigControl::getAntSwNum(vfo_t vfo)
{
    int antNum = 0;
    int retCode = 0;

    retCode = rig_get_ant(my_rig, vfo, &antNum);
    if (retCode < 0)
    {
        return retCode;
    }

    return antNum;

}



int RigControl::setAntSwNum(vfo_t vfo, ant_t antNum)
{
    int retCode = 0;

    retCode = rig_set_ant(my_rig, vfo, antNum);
    return retCode;
}


int RigControl::supportAntSw(int rigNumber, bool *antSwFlag)
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


/**************************************** ***********************************************/


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


/*
int RigControl::getModelNumber(int idx)
{
    if(idx<0) return 0;
    int num = capsList.at(idx)->rig_model;
    return num;
}


const char * RigControl::getMfg_Name(int idx)
{

    if(idx<0) return 0;
    return capsList.at(idx)->mfg_name;
}




const char * RigControl::getModel_Name(int idx)
{

    if(idx<0) return 0;
    return capsList.at(idx)->model_name;
}
*/





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


void RigControl::set_serialConnected(bool connectFlag)
{
    serialConnected = connectFlag;
}




bool RigControl::get_serialConnected()
{
    return serialConnected;
}



 enum serial_parity_e RigControl::getSerialParityCode(int index)
 {

     return serialData::parityCodes[index];

 }

 enum serial_handshake_e RigControl::getSerialHandshakeCode(int index)
 {

     return serialData::handshakeCodes[index];
 }

 QStringList RigControl::getParityCodeNames()
 {
    return serialData::parityStr;
 }

 QStringList RigControl::getHandShakeNames()
 {
     return serialData::handshakeStr;
 }

 QStringList RigControl::getBaudRateNames()
 {


     return serialData::baudrateStr;
 }

 QStringList RigControl::getDataBitsNames()
 {
     return serialData::databitsStr;
 }

 QStringList RigControl::getStopBitsNames()
 {
     return serialData::stopbitsStr;
 }

QString RigControl::gethamlibErrorMsg(int errorCode)
{
    if (errorCode > serialData::hamlibErrorMsg.count())
    {
        return "hamlib Errorcode too large!";
    }
    return serialData::hamlibErrorMsg[errorCode];
}


QStringList RigControl::gethamlibErrorMsg()
{

    return serialData::hamlibErrorMsg;
}

QString RigControl::gethamlibVersion()
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


void RigControl::enableTraceComms(bool state)
{
    traceComms = state;
}

bool RigControl::getTraceState()
{
    return traceComms;
}

// which passes the call to this method
int RigControl::rig_message_cb(enum rig_debug_level_e /*debug_level*/, const char *fmt, va_list ap)
{
    char buf[1024];
//    rig_debug_level_e dbl = debug_level;

    vsprintf (buf, fmt, ap);
    QString s = QString::fromLatin1(buf);
    if (traceComms)
    {
        emit debug_protocol(s);
    }


    return RIG_OK;
}


int rig_message_cb(enum rig_debug_level_e debug_level, rig_ptr_t user_data, const char *fmt, va_list ap)
{
    RigControl *rt = static_cast<RigControl *>(user_data);
    return rt->rig_message_cb(debug_level, fmt, ap);

}
