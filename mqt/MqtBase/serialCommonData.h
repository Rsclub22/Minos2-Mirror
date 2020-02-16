#ifndef SERIALCOMMONDATA_H
#define SERIALCOMMONDATA_H


namespace serialCommonData
{
    enum parityCodes  {PARITY_NONE, PARITY_ODD, PARITY_EVEN, PARITY_MARK, PARITY_SPACE};
    const parityCodes parityCodesList[] = {PARITY_NONE, PARITY_ODD, PARITY_EVEN, PARITY_MARK, PARITY_SPACE};
    const QStringList parityStr = (QStringList() << "None" << "Odd" << "Even" << "Mark" << "Space");

    enum handshakeCodes {HANDSHAKE_NONE, HANDSHAKE_XONXOFF, HANDSHAKE_HARDWARE };
    const handshakeCodes handshakeCodesList[] = {HANDSHAKE_NONE, HANDSHAKE_XONXOFF, HANDSHAKE_HARDWARE };
    const QStringList handshakeStr = (QStringList() << "None" << "XON/XOFF" << "CTS/RTS");

    enum forceLinesCodes {FORCE_LINE_NONE, FORCE_LINE_OFF, FORCE_LINE_ON};
    const forceLinesCodes forceLinesCodesList[] = {FORCE_LINE_NONE, FORCE_LINE_OFF, FORCE_LINE_ON};
    const QStringList forceLinesStr = QStringList() << "" << "High" << "Low";

    const QStringList baudrateStr = (QStringList() << "" << "1200" << "2400" << "4800" << "9600" << "19200" << "38400" << "57600" << "115200");
    const QStringList databitsStr = (QStringList() << "" << "7" << "8" );
    const QStringList stopbitsStr = (QStringList()<< "" << "1" << "2");
    const QStringList pollTimeStr  = (QStringList() << "0.5" << "1" << "2" << "3");

}


#endif // SERIALCOMMONDATA_H
