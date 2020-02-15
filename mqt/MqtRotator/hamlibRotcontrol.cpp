/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2016 - 2020
//
//
// Hamlib Library
//
/////////////////////////////////////////////////////////////////////////////

#include <QList>
#include <QDebug>
#include <QStringList>
#include <QThread>
#include "hamlibRotcontrol.h"
#include "rotatorfactory.h"
#include "rotcapabilities.h"
#include "hamlibCommon.h"
#include "minosNetUtils.h"
#include "MTrace.h"




static QList<const rot_caps *> capsList;

int collect(const rot_caps *caps, rig_ptr_t)
{
    capsList.append(caps);
    return 1;
}

bool traceComms = false;

int debug_callback (enum rig_debug_level_e level, rig_ptr_t /* arg */, char const * format, va_list ap)
{
  Q_UNUSED(level)
   QString message;

  static char constexpr fmt[] = "Hamlib: ";
  message = message.vsprintf (format, ap).trimmed ();

  if (traceComms)
  {
      trace(QString("%1 %2").arg(fmt).arg(message));
  }


  return 0;
}

extern "C"
{
  //typedef struct rot RIG;
  struct rot_caps;
  //typedef int vfo_t;
}

HamlibRotControl::HamlibRotControl(QObject *parent) : RotatorBase(parent)
{

//   getRotatorList();
   rot_azimuth = 0.0;


   // set callback for debug messages
   // NB callback is the C function, not the class method.
   // user_data is used to point to our class.

   //rig_set_debug_callback (::rig_message_cb, static_cast<rig_ptr_t>(this));

}




HamlibRotControl::~HamlibRotControl()
{
    //rot_close(my_rot); /* close port */
    //rot_cleanup(my_rot); /* if you care about memory */
}


/*

int HamlibRotControl::getSupportCwCcwCmd(int rotNumber, bool *flag)
{
    int retCode = 0;
    ROT *my_rot;
    my_rot = rot_init(rotNumber);
    if (my_rot != nullptr)
    {
            if (my_rot->caps->move == nullptr)
            {
                *flag = false;
            }
            else
            {
                *flag = true;
            }

            return retCode;
    }

    retCode = -1;
    return retCode;
}

int HamlibRotControl::getMaxMinRotation(int rotNumber, int *maxRot, int *minRot)
{
    int retCode = 0;
    ROT *my_rot;
    my_rot = rot_init(rotNumber);
    if (my_rot != nullptr)
    {
        *maxRot = int(my_rot->caps->max_az);
        *minRot = int(my_rot->caps->min_az);
    }
    else
    {
        *maxRot = COMPASS_MAX360;
        *minRot = COMPASS_MIN0;
        retCode = -1;
    }


    return retCode;
}

*/
int HamlibRotControl::rotInit(srotParams &selectedAntenna)
{
    int retcode;

#if defined Q_OS_WIN32
    QString comport = "\\\\.\\";
#elif defined Q_OS_LINUX
    QString comport = "/dev/";
#elif defined Q_OS_MAC 
    QString comport = "/dev/";
    
#endif

    comport.append(selectedAntenna.comport);

    setRotConnected(false);

    my_rot = rot_init(selectedAntenna.rotatorModelNumber);
    if (!my_rot)
    {
        return retcode = -14;
    }



    // load rotator params to open
    if ((selectedAntenna.portType) == RotCapConstants::PortType::serial)
    {
        strncpy(my_rot->state.rotport.pathname, comport.toLatin1().data(), comport.length());
        my_rot->state.rotport.parm.serial.rate = selectedAntenna.baudrate;
        my_rot->state.rotport.parm.serial.data_bits = selectedAntenna.databits;
        my_rot->state.rotport.parm.serial.stop_bits = selectedAntenna.stopbits;
        my_rot->state.rotport.parm.serial.parity = hamlibSerialData::getSerialParityCode(selectedAntenna.parity);
        my_rot->state.rotport.parm.serial.handshake = hamlibSerialData::getSerialHandshakeCode(selectedAntenna.handshake);
        //if (my_rot->state.rotport.parm.serial.handshake == RIG_HANDSHAKE_NONE)
       // {
       //     my_rot->state.rotport.parm.serial.dtr_state = RIG_SIGNAL_ON;
       //     my_rot->state.rotport.parm.serial.rts_state = RIG_SIGNAL_ON;
       // }
       // else
       // {
       //     my_rot->state.rotport.parm.serial.dtr_state = RIG_SIGNAL_UNSET;
       //     my_rot->state.rotport.parm.serial.rts_state = RIG_SIGNAL_UNSET;
       // }

    }
    else if (selectedAntenna.portType == RotCapConstants::PortType::network )
    {
        QString netAdd;
        if (selectedAntenna.networkAdd.isEmpty() || isHostLocal(selectedAntenna.networkAdd))
        {
            netAdd = RIGCTLD_LOCAL_HOST_ADDRESS;
        }
        else
        {
            netAdd = selectedAntenna.networkAdd;
        }
        strncpy(my_rot->state.rotport.pathname, QString(netAdd + ":" + selectedAntenna.networkPort).toLatin1().data(), FILPATHLEN);
    }
    else if (selectedAntenna.portType == RotCapConstants::PortType::none)
    {
        strncpy(my_rot->state.rotport.pathname, QString("").toLatin1().data(), FILPATHLEN);
    }


    retcode = rot_open(my_rot);
    if (retcode >= 0)
    {

        setRotConnected(true);


    }
    else
    {

        setRotConnected(false);
    }

    return retcode;

}


int HamlibRotControl::closeRotator()
{

    if (my_rot->caps->rot_model == ROT_MODEL_SPID_ROT1PROG || my_rot->caps->rot_model == ROT_MODEL_SPID_ROT2PROG)
    {
        // make sure we stop for a moment to let any last poll finish sending
        QThread::msleep(1000);
    }
    int retcode;
    retcode = rot_close (my_rot);
    retcode = rot_cleanup (my_rot);
    setRotConnected(false);
    return retcode;

}


void HamlibRotControl::register_rotators(RotatorFactory::Rotators *rotatorsList)
{


    //rig_set_debug_callback (::rig_message_cb, static_cast<rig_ptr_t>(this));
    //rig_set_debug_callback (::rig_message_cb, nullptr);


    rig_set_debug_callback (debug_callback, nullptr);

    capsList.clear();
    rot_load_all_backends();
    rot_list_foreach(collect, nullptr);

    for (int i = 0; i < capsList.count(); i++)
    {
        //QString t = QString::number(capsList[i]->rot_model);
        QString key = QString("%1 %2").arg(capsList[i]->mfg_name).arg(capsList[i]->model_name);
        auto port_type = RotCapConstants::PortType::none;
        switch(capsList[i]->port_type)
        {
            case RIG_PORT_SERIAL:
                port_type = RotCapConstants::PortType::serial;
            break;

            case RIG_PORT_NETWORK:
                port_type = RotCapConstants::PortType::network;
            break;

            case RIG_PORT_USB:
                port_type = RotCapConstants::PortType::usb;
            break;
            default:
            {}
        }



        (*rotatorsList)[key] = RotCapabilities(capsList[i]->rot_model, port_type,
                                                               capsList[i]->mfg_name, capsList[i]->model_name,
                                                               capsList[i]->move != nullptr ? true : false,
                                                               capsList[i]->min_az, capsList[i]->max_az,
                                                               RotCapContstants::PollData::pollDataOn,
                                                               RotCapContstants::RotatorDisplay::displayFull);

    }




}

// static to init flag
void HamlibRotControl::setTraceCommsFlag(bool value)
{
    traceComms = value;
}

void HamlibRotControl::setTraceComms(bool value)
{
    traceComms = value;
}

bool HamlibRotControl::getTraceComms()
{
    return traceComms;
}



void HamlibRotControl::getRotatorList()
{


        capsList.clear();
        rot_load_all_backends();
        rot_list_foreach(collect, nullptr);
        qSort(capsList.begin(),capsList.end(),model_Sort);

}



bool HamlibRotControl::getRotatorList(QComboBox *cb)
{
    int i;
    rig_port_e portType = RIG_PORT_NONE;

    if(capsList.count()==0) return false;
    QStringList sl;
    // add blank at beginning
    //sl << "";
    for (i=0;i<capsList.count();i++)
    {

        QString t;
        t= QString::number(capsList.at(i)->rot_model);
        t=t.rightJustified(5,' ')+", ";
        t+= capsList.at(i)->mfg_name;
        t+=", ";
        t+=capsList.at(i)->model_name;
        if (getPortType(capsList.at(i)->rot_model, &portType) != -1)
        {
            //qDebug() << capsList.at(i)->rot_model << capsList.at(i)->model_name << portType;
            if (portType == RIG_PORT_SERIAL || portType == RIG_PORT_NETWORK || portType == RIG_PORT_UDP_NETWORK || portType == RIG_PORT_NONE)
            {
                sl << t;
            }
        }
    }
    std::sort(sl.begin(), sl.end());
    cb->addItems(sl);
    return true;
}


int HamlibRotControl::getPortType(int rotNumber, rig_port_e *portType)
{

    int retCode = 0;
    ROT *my_rot;
    my_rot = rot_init(rotNumber);
    if (my_rot != nullptr)
    {
        *portType = my_rot->caps->port_type;
        return retCode;
    }

    retCode = -1;
    return retCode;

}


int HamlibRotControl::getModelInfo(QString rotModel, int *rotModelNumber, QString *rotMfgName, QString *rotModelName)
{
    bool ok;
    int number;
    QStringList modelInfo = rotModel.remove('\x20').split(',');
    if (modelInfo.length() == 3)
    {
        number = modelInfo[0].toInt(&ok);
        if (!ok)
        {
           return -1;
        }

        *rotModelNumber = number;
        *rotMfgName = modelInfo[1].trimmed();
        *rotModelName = modelInfo[2].trimmed();
        return 0;


    }

    return -1;

}



// stop azimuth rotation


int HamlibRotControl::stop_rotation()
{
    int retCode = RIG_OK;
    retCode = rot_stop(my_rot);


    return retCode;
}

// request current bearing from controller



int HamlibRotControl::request_bearing()
{
    int retCode = RIG_OK;

    retCode = rot_get_position (my_rot, &rot_azimuth, &rot_elevation);

    if (retCode == RIG_OK)
    {
        emit bearing_updated(static_cast<int>(rot_azimuth));
    }

    return retCode;

}


//int HamlibRotControl::getRotatorAzimuth()
//{
//    return rot_azimuth;
//}



int HamlibRotControl::rotateClockwise(const int speed)
{

    int retCode = RIG_OK;
    retCode = rot_move(my_rot, ROT_MOVE_RIGHT , speed);
    if (my_rot->caps->rot_model == ROT_MODEL_SPID_ROT1PROG || my_rot->caps->rot_model == ROT_MODEL_SPID_ROT2PROG)
    {
        QThread::msleep(1000);
    }

    return retCode;
}

int HamlibRotControl::rotateCClockwise(const int speed)
{
    int retCode = RIG_OK;
    retCode = rot_move(my_rot, ROT_MOVE_LEFT , speed);
    if (my_rot->caps->rot_model == ROT_MODEL_SPID_ROT1PROG || my_rot->caps->rot_model == ROT_MODEL_SPID_ROT2PROG)
    {
        QThread::msleep(1000);
    }

    return retCode;
}


int HamlibRotControl::rotate_to_bearing(const int bearing)
{
    int retCode = RIG_OK;
    float rotbearing = bearing;

    retCode = rot_set_position(my_rot, rotbearing, 0.0);
    if (my_rot->caps->rot_model == ROT_MODEL_SPID_ROT1PROG || my_rot->caps->rot_model == ROT_MODEL_SPID_ROT2PROG)
    {
        QThread::msleep(1000);
    }

    return retCode;

}



void HamlibRotControl::set_rotatorSpeed(int speed)
{
    rot_speed = speed;
}

int HamlibRotControl::get_rotatorSpeed()
{
    return rot_speed;
}
/*

void HamlibRotControl::set_serialConnected(bool connectFlag)
{
    serialConnected = connectFlag;
}




bool HamlibRotControl::get_serialConnected()
{
    return serialConnected;
}

*/





QString HamlibRotControl::getErrorMsgText(int errorCode)
{

    if (errorCode > static_cast<int>(sizeof(hamlibText::hamlibErrorMsg)/sizeof(const char *)))
    {
        return tr("hamlib Errorcode too large!");
    }
    return tr(hamlibText::hamlibErrorMsg[errorCode]);
}






QString HamlibRotControl::getRotLibVersion()
{
    QString ver = hamlib_version;
    return ver;
}






// which passes the call to this method
//int HamlibRotControl::rig_message_cb(enum rig_debug_level_e /*debug_level*/, const char *fmt, va_list ap)
//{
//    char buf[1024];
//    rig_debug_level_e dbl = debug_level;

//    vsprintf (buf, fmt, ap);
//    QString s = QString::fromLatin1(buf);
//    if (traceComms)
//    {
//        emit debug_protocol(s);
//    }


//    return RIG_OK;
//}



//int rig_message_cb(enum rig_debug_level_e debug_level, rig_ptr_t user_data, const char *fmt, va_list ap)
//{
//    HamlibRotControl *rt = static_cast<HamlibRotControl *>(user_data);
//    return rt->rig_message_cb(debug_level, fmt, ap);

//}


bool model_Sort(const rot_caps *caps1,const rot_caps *caps2)
{
    if(caps1->mfg_name==caps2->mfg_name)
    {
        if (QString::compare(caps1->model_name,caps2->model_name)<0) return true;
        return false;
    }
    if (QString::compare(caps1->mfg_name,caps2->mfg_name)<0) return true;
    return false;
}


