

#include <QtSerialPort/QSerialPort>
#include <QSerialPortInfo>

#include "rigcommon.h"

rigcommon::rigcommon()
{

}


QString vfoToStr(VFO curVfo)
{
    if (curVfo == VFO::VFOA)
    {
        return "VFOA";
    }
    else if (curVfo == VFO::VFOB)
    {
        return "VFOB";
    }
    else if (curVfo == VFO::CURRENT_VFO)
    {
       return "CURRENT_VFO";
    }

    return "CURRENT_VFO";
}

VFO strToVfo(QString vfo)
{

    if (vfo == "VFOA")
    {
        return VFO::VFOA;
    }
    else if (vfo == "VFOB")
    {
        return VFO::VFOB;
    }
    else if (vfo == "CURRENT_VFO")
    {
        return VFO::CURRENT_VFO;
    }

    return VFO::CURRENT_VFO;

}



void fillPortsInfo(QComboBox* comportSel)
{


    comportSel->clear();

    QString description;
    QString manufacturer;
    QString serialNumber;

    comportSel->addItem("");

    foreach (auto const &info, QSerialPortInfo::availablePorts())
    {
        QStringList list;
        description = info.description();
        manufacturer = info.manufacturer();
#if QT_VERSION > QT_VERSION_CHECK(5, 3, 0)
        serialNumber = info.serialNumber();
#endif
        list << info.portName()
             << (!description.isEmpty() ? description : blankString)
             << (!manufacturer.isEmpty() ? manufacturer : blankString)
             << (!serialNumber.isEmpty() ? serialNumber : blankString)
             << info.systemLocation()
             << (info.vendorIdentifier() ? QString::number(info.vendorIdentifier(), 16) : blankString)
             << (info.productIdentifier() ? QString::number(info.productIdentifier(), 16) : blankString);


        comportSel->addItem(list.first(), list);

    }

}



void getListOfComports(QStringList &listOfAvailComports)
{
    foreach (auto const &info, QSerialPortInfo::availablePorts())
    {

        listOfAvailComports << info.portName();

    }
}

bool isComportAvail(const QString comport)
{
    QStringList availComports;

    foreach (auto const &info, QSerialPortInfo::availablePorts())
    {

        availComports << info.portName();

    }

    if (availComports.contains(comport))
    {
        return true;
    }

    return false;


}




bool scatParams::compareEqual(QSharedPointer <scatParams> radParams)
{
    if (radioName == radParams->radioName &&
        radioNumber == radParams->radioNumber &&
        previousRadioName == radParams->previousRadioName &&
        markForDeletion == radParams->markForDeletion &&
        comport == radParams->comport &&
        rigMfg_Name == radParams->rigMfg_Name &&
        rigModelName == radParams->rigModelName &&
        rigModel == radParams->rigModel &&
        rigModelNumber == radParams->rigModelNumber &&
        pollInterval == radParams->pollInterval &&
        civAddress == radParams->civAddress &&
        baudrate == radParams->baudrate &&
        parity == radParams->parity &&
        stopbits == radParams->stopbits &&
        databits == radParams->databits &&
        handshake == radParams->handshake &&
        forceDtr == radParams->forceDtr &&
        forceRts == radParams->forceRts &&
        portType == radParams->portType &&
        advancedCommsFlag == radParams->advancedCommsFlag &&
        networkAdd == radParams->networkAdd &&
        networkPort == radParams->networkPort &&
        enablePTT == radParams->enablePTT &&
        pttSerialPort == radParams->pttSerialPort &&
        pttType == radParams->pttType &&
        rigCtldEnable == radParams->rigCtldEnable &&
        startMinosRigCtld == radParams->startMinosRigCtld &&
        rigCtldNetworkAdd == radParams->rigCtldNetworkAdd &&
        rigCtldNetworkPort == radParams->rigCtldNetworkPort &&
        mgmMode == radParams->mgmMode &&
        antSwitchAvail == radParams->antSwitchAvail &&
        ritSupported == radParams->ritSupported &&

        transVertEnable == radParams->transVertEnable &&
        supportBands == radParams->supportBands &&     // for non hamlib radios
        compareStringList(transVertNames, radParams->transVertNames) &&
        //numTransverters == radParams->numTransverters &&
        enableTransSwitch == radParams->enableTransSwitch &&
        enableLocTVSwMsg == radParams->enableLocTVSwMsg &&
        locTVSwComport == radParams->locTVSwComport &&
        compareStringList(radioSupBands, radParams->radioSupBands) &&
        compareStringList(radioTransSupBands, radParams->radioTransSupBands) &&
        transVertSettings == radParams->transVertSettings &&
        enableDisableCatFeature == radParams->enableDisableCatFeature)
    {
       return true;
    }

    return false;
}





bool scatParams::compareNotEqual(const QSharedPointer<scatParams> radParams)
{



    if (radioName != radParams->radioName ||
            radioNumber != radParams->radioNumber ||
            previousRadioName != radParams->previousRadioName ||
            markForDeletion != radParams->markForDeletion ||
            comport != radParams->comport ||
            rigMfg_Name != radParams->rigMfg_Name ||
            rigModelName != radParams->rigModelName ||
            rigModel != radParams->rigModel ||
            rigModelNumber != radParams->rigModelNumber ||
            pollInterval != radParams->pollInterval ||
            civAddress != radParams->civAddress ||
            baudrate != radParams->baudrate ||
            parity != radParams->parity ||
            stopbits != radParams->stopbits ||
            databits != radParams->databits ||
            handshake != radParams->handshake ||
            forceDtr != radParams->forceDtr ||
            forceRts != radParams->forceRts ||
            portType != radParams->portType ||
            advancedCommsFlag != radParams->advancedCommsFlag ||
            networkAdd != radParams->networkAdd ||
            networkPort != radParams->networkPort ||
            enablePTT != radParams->enablePTT ||
            pttSerialPort != radParams->pttSerialPort ||
            pttType != radParams->pttType ||
            rigCtldEnable != radParams->rigCtldEnable ||
            startMinosRigCtld != radParams->startMinosRigCtld ||
            rigCtldNetworkAdd != radParams->rigCtldNetworkAdd ||
            rigCtldNetworkPort != radParams->rigCtldNetworkPort ||
            mgmMode != radParams->mgmMode ||
            antSwitchAvail != radParams->antSwitchAvail ||
            ritSupported != radParams->ritSupported ||

            transVertEnable != radParams->transVertEnable ||
            supportBands != radParams->supportBands ||     // for non hamlib radios
            !compareStringList(transVertNames, radParams->transVertNames) ||
            //numTransverters != radParams->numTransverters ||
            enableTransSwitch != radParams->enableTransSwitch ||
            enableLocTVSwMsg != radParams->enableLocTVSwMsg ||
            locTVSwComport != radParams->locTVSwComport ||
            !compareStringList(radioSupBands, radParams->radioSupBands) ||
            !compareStringList(radioTransSupBands, radParams->radioTransSupBands) ||
            transVertSettings != radParams->transVertSettings ||
            enableDisableCatFeature != radParams->enableDisableCatFeature)
    {
       return true;
    }

    return false;

}


bool scatParams::compareStringList(QStringList &sl1, QStringList &sl2)
{
    if (sl1.count() != sl2.count())
    {
        return false;
    }
    else
    {
        for (int i = 0; i <sl1.count(); i++)
        {
            if (sl1[i] != sl2[i])
            {
                return false;
            }
        }
    }
    return true;
}

void scatParams::scatParamsCopy(const QSharedPointer<scatParams> srce)
{
    radioName = srce->radioName;
    radioNumber = srce->radioNumber;
    previousRadioName = srce->previousRadioName;
    markForDeletion = srce->markForDeletion;
    comport = srce->comport;
    rigMfg_Name = srce->rigMfg_Name;
    rigModelName = srce->rigModelName;
    rigModel = srce->rigModel;       // used as key to select radio
    rigModelNumber = srce->rigModelNumber;
    pollInterval = srce->pollInterval;
    civAddress = srce->civAddress;
    baudrate = srce->baudrate;
    parity = srce->parity;
    stopbits = srce->stopbits;
    databits = srce->databits;
    handshake = srce->handshake;
    forceDtr = srce->forceDtr;
    forceRts = srce->forceRts;
    portType = srce->portType;
    advancedCommsFlag = srce->advancedCommsFlag;
    networkAdd = srce->networkAdd;
    networkPort = srce->networkPort;
    enablePTT  = srce->enablePTT;
    pttSerialPort = srce->pttSerialPort;
    pttType = srce->pttType;
    rigCtldEnable = srce->rigCtldEnable;
    startMinosRigCtld = srce->startMinosRigCtld;
    rigCtldNetworkAdd = srce->rigCtldNetworkAdd;
    rigCtldNetworkPort = srce->rigCtldNetworkPort;
    mgmMode = srce->mgmMode;
    antSwitchAvail = srce->antSwitchAvail;
    ritSupported = srce->ritSupported;

    transVertEnable  = srce->transVertEnable;
    supportBands = srce->supportBands;        // for non hamlib radios
    transVertNames = srce->transVertNames;
    //numTransverters = srce->numTransverters;
    enableTransSwitch = srce->enableTransSwitch;
    enableLocTVSwMsg = srce->enableLocTVSwMsg;
    locTVSwComport = srce->locTVSwComport;
    radioSupBands = srce->radioSupBands;  // bands supported by radio
    radioTransSupBands = srce->radioTransSupBands; // band supported by radio and transverters
    transVertSettings = srce->transVertSettings;


    // enable\Disable Cat features
    enableDisableCatFeature = srce->enableDisableCatFeature;


    //return QSharedPointer<scatParams>(this);

}


bool EnableDisableCatFeature::operator ==(const EnableDisableCatFeature &edcf)
{
    if (enableDisplay == edcf.enableDisplay &&
        ritEnable == edcf.ritEnable &&
        sMeterEnable == edcf.sMeterEnable &&
        volumeEnable == edcf.volumeEnable &&
        voiceMemEnable == edcf.voiceMemEnable &&
        cWMemEnable == edcf.cWMemEnable &&
        catEnable == edcf.catEnable)
    {
        return true;
    }

    return false;
}
bool EnableDisableCatFeature::operator !=(const EnableDisableCatFeature &edcf)
{
    if (enableDisplay != edcf.enableDisplay ||
        ritEnable != edcf.ritEnable ||
        sMeterEnable != edcf.sMeterEnable ||
        volumeEnable != edcf.volumeEnable ||
        voiceMemEnable != edcf.voiceMemEnable ||
        cWMemEnable != edcf.cWMemEnable ||
        catEnable != edcf.catEnable)
    {
        return true;
    }

    return false;
}

EnableDisableCatFeature& EnableDisableCatFeature::operator =(const EnableDisableCatFeature &edcf)
{
    enableDisplay = edcf.enableDisplay;
    ritEnable = edcf.ritEnable;
    sMeterEnable = edcf.sMeterEnable;
    volumeEnable = edcf.volumeEnable;
    voiceMemEnable = edcf.voiceMemEnable;
    cWMemEnable = edcf.cWMemEnable;
    catEnable = edcf.catEnable;

    return *this;
}


