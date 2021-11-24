

#include <QtSerialPort/QSerialPort>
#include <QSerialPortInfo>

#include "rigcommon.h"

rigcommon::rigcommon()
{

}
MODE rigcommon::convertQStringToMode(QString modeStr)
{
    if (modeStr == hamlibData::AM) return AM;
    if (modeStr == hamlibData::CW) return CW;
    if (modeStr == hamlibData::CWR) return CW_R;
    if (modeStr == hamlibData::USB) return USB;
    if (modeStr == hamlibData::LSB) return LSB;
    if (modeStr == hamlibData::RTTY) return FSK;
    if (modeStr == hamlibData::RTTYR) return FSK_R;
    if (modeStr == hamlibData::PKTLSB) return DIG_L;
    if (modeStr == hamlibData::PKTUSB) return DIG_U;
    if (modeStr == hamlibData::FM) return FM;
    if (modeStr == hamlibData::PKTFM) return DIG_FM;

    return USB; // default


}
QString rigcommon::convertModeToQString(MODE mode)
{
    switch (mode)
    {
    case AM: return hamlibData::AM;
    case CW: return hamlibData::CW;
    case CW_R: return hamlibData::CWR;
    case USB: return hamlibData::USB;
    case LSB: return hamlibData::LSB;
    case FSK: return hamlibData::RTTY;
    case FSK_R: return hamlibData::RTTYR;
    case DIG_L: return hamlibData::PKTLSB;
    case DIG_U: return hamlibData::PKTUSB;
    case FM: return hamlibData::FM;
    case DIG_FM: return hamlibData::PKTFM;
    default: break;
    }
    return hamlibData::USB;

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

    QList<QSerialPortInfo> availablePorts = QSerialPortInfo::availablePorts();
    for (auto const &info: qAsConst(availablePorts))
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
    QList<QSerialPortInfo> availablePorts = QSerialPortInfo::availablePorts();
    for (auto const &info: qAsConst(availablePorts))
    {

        listOfAvailComports << info.portName();

    }
}

bool isComportAvail(const QString comport)
{
    QStringList availComports;

    QList<QSerialPortInfo> availablePorts = QSerialPortInfo::availablePorts();
    for (auto const &info: qAsConst(availablePorts))
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
        //compareStringList(transVertNames, radParams->transVertNames) &&
        //numTransverters == radParams->numTransverters &&
        enableTransSwitch == radParams->enableTransSwitch &&
        enableLocTVSwMsg == radParams->enableLocTVSwMsg &&
        locTVSwComport == radParams->locTVSwComport &&
        compareStringList(radioSupBands, radParams->radioSupBands) &&
        compareStringList(radioTransSupBands, radParams->radioTransSupBands) &&
        //transVertSettings == radParams->transVertSettings &&
        enableDisableCatFeature == radParams->enableDisableCatFeature)
    {
       return true;
    }

    return false;
}





bool scatParams::compareNotEqual(const QSharedPointer<scatParams> radParams)
{
/*
    radioName != radParams->radioName ? qDebug() << "radioName = true" : qDebug() << "radioName = false";
    radioNumber != radParams->radioNumber ? qDebug() << "radioNumber = true" : qDebug() << "radioNumber = false";
    previousRadioName != radParams->previousRadioName ? qDebug() << "previousRadioName = true" : qDebug() << "previousRadioName = false";
    markForDeletion != radParams->markForDeletion ? qDebug() << "markForDeletion = true" : qDebug() << "markForDeletion = false";
                comport != radParams->comport ? qDebug() << "comport = true" : qDebug() << "comport = false";
                rigMfg_Name != radParams->rigMfg_Name ? qDebug() << "rigMfg_Name = true" : qDebug() << "rigMfg_Name = false";
                rigModelName != radParams->rigModelName ? qDebug() << "rigModelName = true" : qDebug() << "rigModelName = false";
                rigModel != radParams->rigModel ? qDebug() << "rigModel = true" : qDebug() << "rigModel = false";
                rigModelNumber != radParams->rigModelNumber ? qDebug() << "rigModelNumber = true" : qDebug() << "rigModelNumber = false";
                pollInterval != radParams->pollInterval ? qDebug() << "pollInterval = true" : qDebug() << "pollInterval = false";
                civAddress != radParams->civAddress ? qDebug() << "civAddress = true" : qDebug() << "civAddress = false";
                baudrate != radParams->baudrate ? qDebug() << "baudrate = true" : qDebug() << "baudrate = false";
                parity != radParams->parity ? qDebug() << "parity = true" : qDebug() << "parity = false";
                stopbits != radParams->stopbits ? qDebug() << "stopbits = true" : qDebug() << "stopbits = false";
                databits != radParams->databits ? qDebug() << "databits = true" : qDebug() << "databits = false";
                handshake != radParams->handshake ? qDebug() << "handshake = true" : qDebug() << "handshake = false";
                forceDtr != radParams->forceDtr ? qDebug() << "forceDtr = true" : qDebug() << "forceDtr = false";
                forceRts != radParams->forceRts ? qDebug() << "forceRts = true" : qDebug() << "forceRts = false";
                portType != radParams->portType ? qDebug() << "portType = true" : qDebug() << "portType = false";
                advancedCommsFlag != radParams->advancedCommsFlag ? qDebug() << "advancedCommsFlag = true" : qDebug() << "advancedCommsFlag = false";
                networkAdd != radParams->networkAdd ? qDebug() << "networkAdd = true" : qDebug() << "networkAdd = false";
                networkPort != radParams->networkPort ? qDebug() << "networkPort = true" : qDebug() << "networkPort = false";
                enablePTT != radParams->enablePTT ? qDebug() << "enablePTT = true" : qDebug() << "enablePTT = false";
                pttSerialPort != radParams->pttSerialPort ? qDebug() << "pttSerialPort = true" : qDebug() << "pttSerialPort = false";
                pttType != radParams->pttType ? qDebug() << "pttType = true" : qDebug() << "pttType = false";
                rigCtldEnable != radParams->rigCtldEnable ? qDebug() << "rigCtldEnable = true" : qDebug() << "rigCtldEnable = false";
                startMinosRigCtld != radParams->startMinosRigCtld ? qDebug() << "startMinosRigCtld = true" : qDebug() << "startMinosRigCtld = false";
                rigCtldNetworkAdd != radParams->rigCtldNetworkAdd ? qDebug() << "rigCtldNetworkAdd = true" : qDebug() << "rigCtldNetworkAdd = false";
                rigCtldNetworkPort != radParams->rigCtldNetworkPort ? qDebug() << "rigCtldNetworkPort = true" : qDebug() << "rigCtldNetworkPort = false";
                mgmMode != radParams->mgmMode ? qDebug() << "mgmMode = true" : qDebug() << "mgmMode = false";
                antSwitchAvail != radParams->antSwitchAvail ? qDebug() << "antSwitchAvail = true" : qDebug() << "antSwitchAvail = false";
                ritSupported != radParams->ritSupported ? qDebug() << "ritSupported = true" : qDebug() << "ritSupported = false";

                transVertEnable != radParams->transVertEnable ? qDebug() << "transVertEnable = true" : qDebug() << "transVertEnable = false";
                supportBands != radParams->supportBands ? qDebug() << "supportBands = true" : qDebug() << "supportBands = false";
                //!compareStringList(transVertNames, radParams->transVertNames) ? qDebug() << "transVertNames = true" : qDebug() << "transVertNames = false";
                //numTransverters != radParams->numTransverters ? qDebug() << "radioName = true" : qDebug() << "radioName = false";
                enableTransSwitch != radParams->enableTransSwitch ? qDebug() << "enableTransSwitch = true" : qDebug() << "enableTransSwitch = false";
                enableLocTVSwMsg != radParams->enableLocTVSwMsg ? qDebug() << "enableLocTVSwMsg = true" : qDebug() << "enableLocTVSwMsg = false";
                locTVSwComport != radParams->locTVSwComport ? qDebug() << "locTVSwComport = true" : qDebug() << "locTVSwComport = false";
                !compareStringList(radioSupBands, radParams->radioSupBands) ? qDebug() << "radioSupBands = true" : qDebug() << "radioSupBands = false";
                !compareStringList(radioTransSupBands, radParams->radioTransSupBands) ? qDebug() << "radioTransSupBands = true" : qDebug() << "radioTransSupBands = false";
                //transVertSettings != radParams->transVertSettings ? qDebug() << "transVertSettings = true" : qDebug() << "transVertSettings = false";
                enableDisableCatFeature != radParams->enableDisableCatFeature ? qDebug() << "enableDisableCatFeature = true" : qDebug() << "enableDisableCatFeature = false";

*/
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
            //!compareStringList(transVertNames, radParams->transVertNames) ||
            //numTransverters != radParams->numTransverters ||
            enableTransSwitch != radParams->enableTransSwitch ||
            enableLocTVSwMsg != radParams->enableLocTVSwMsg ||
            locTVSwComport != radParams->locTVSwComport ||
            !compareStringList(radioSupBands, radParams->radioSupBands) ||
            !compareStringList(radioTransSupBands, radParams->radioTransSupBands) ||
            enableDisableCatFeature != radParams->enableDisableCatFeature)
    {
       return true;
    }

    return false;

}

bool scatParams::transVertSettingsNotEqual(const QMap<QString, QSharedPointer<TransVertParams> > tvs2)
{
    QStringList tvs1List = transVertSettings.keys();
    QStringList tvs2List = tvs2.keys();
    if (!compareStringList(tvs1List, tvs2List))
    {
        return true;
    }

    for (const auto &tv: qAsConst(tvs1List))
    {
        if(transVertSettings.value(tv)->notEqual(tvs2.value(tv)))
        {
            return true;
        }
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
    //transVertNames = srce->transVertNames;
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


