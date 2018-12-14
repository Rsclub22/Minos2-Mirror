#include <QList>
#include <QDebug>
#include <QStringList>
#include "rigcontrol.h"
#include <hamlib/rig.h>
//#include "rotctl_parse.h"


static QList<const rig_caps *> capsList;
static bool riglistLoaded=false;


int collect(const rig_caps *caps,rig_ptr_t)
{
    capsList.append(caps);
    return 1;
}



RigControl::RigControl()
{
    getRigList();
}


RigControl::~RigControl()
{
    rig_close(my_rig); /* close port */
    rig_cleanup(my_rig); /* if you care about memory */
}

int RigControl::init(scatParams currentRadio)
{
    int retcode;
    QString comport = "\\\\.\\";
    comport.append(currentRadio.comport);

    my_rig = rig_init(currentRadio.radioModelNumber);
    if (!my_rig)
    {
        qDebug() << "Error init rig";
    }

    // load cat params

    if(QString(my_rig->caps->mfg_name) == "Icom")
    {
        if(!currentRadio.civAddress.isEmpty())
        {
            rig_set_conf(my_rig, rig_token_lookup(my_rig, "civaddr"),currentRadio.civAddress.toLatin1());
        }
    }
    strncpy(my_rig->state.rigport.pathname, comport.toLatin1().data(), FILPATHLEN);
    my_rig->state.rigport.parm.serial.rate = currentRadio.baudrate;
    my_rig->state.rigport.parm.serial.data_bits = currentRadio.databits;
    my_rig->state.rigport.parm.serial.stop_bits = currentRadio.stopbits;
    my_rig->state.rigport.parm.serial.parity = getSerialParityCode(currentRadio.parity);
    my_rig->state.rigport.parm.serial.handshake = getSerialHandshakeCode(currentRadio.handshake);


    retcode = rig_open(my_rig);
    if (retcode >= 0)
    {
        qDebug() << "rig opened ok";
        set_serialConnected(true);
    }
    else
    {
        qDebug() << "rig open error";
        set_serialConnected(false);
    }

    return retcode;

}


int RigControl::closeRig()
{
    int retcode;
    /*retcode =*/ rig_close(my_rig);

    retcode = rig_cleanup(my_rig);
    set_serialConnected(false);
    return retcode;

}


int RigControl::getFrequency()
{
    int retCode = 0;

    retCode = rig_get_freq(my_rig, RIG_VFO_CURR, &frequency);
    if (retCode == RIG_OK)
    {
        qDebug() << frequency ;
        emit frequency_updated(frequency);
    }
    return retCode;
}




void RigControl::getRigList()
{
    if(!riglistLoaded)
    {
        capsList.clear();
        rig_load_all_backends();
        rig_list_foreach(collect,0);
        qSort(capsList.begin(),capsList.end(),model_Sort);
        riglistLoaded=true;
    }
}

bool RigControl::getRigList(QComboBox *cb)
{
    int i;
    if(capsList.count()==0) return false;
    QStringList sl;
    for (i=0;i<capsList.count();i++)
    {
        QString t;
        t= QString::number(capsList.at(i)->rig_model);
        t=t.rightJustified(5,' ')+" ";
        t+= capsList.at(i)->mfg_name;
        t+=",";
        t+=capsList.at(i)->model_name;
        sl << t;
    }
    cb->addItems(sl);
    return true;
}

int RigControl::getModelNumber(int idx)
{
    if(idx<0) return 0;
    return capsList.at(idx)->rig_model;
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
