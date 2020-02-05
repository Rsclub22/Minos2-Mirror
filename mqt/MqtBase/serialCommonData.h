#ifndef SERIALCOMMONDATA_H
#define SERIALCOMMONDATA_H


namespace serialCommonData
{
    const QStringList parityStr = (QStringList() << "None" << "Odd" << "Even" << "Mark" << "Space");
    const QStringList handshakeStr = (QStringList() << "None" << "XON/XOFF" << "CTS/RTS");
    const QStringList forceLinesStr = QStringList() << "" << "High" << "Low";
    const QStringList baudrateStr = (QStringList() << "" << "1200" << "2400" << "4800" << "9600" << "19200" << "38400" << "57600" << "115200");
    const QStringList databitsStr = (QStringList() << "" << "7" << "8" );
    const QStringList stopbitsStr = (QStringList()<< "" << "1" << "2");
    const QStringList pollTimeStr  = (QStringList() << "0.5" << "1" << "2" << "3");

}


#endif // SERIALCOMMONDATA_H
