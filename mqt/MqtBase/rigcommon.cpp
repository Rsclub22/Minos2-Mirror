

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

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
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
