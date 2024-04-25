#ifndef SERIALCOMMONDATA_H
#define SERIALCOMMONDATA_H

#include <QStringList>

namespace serialCommonData
{
    enum class serialParityCodes  {PARITY_NONE, PARITY_ODD, PARITY_EVEN, PARITY_MARK, PARITY_SPACE};
    const serialParityCodes parityCodesList[] = {serialParityCodes::PARITY_NONE, serialParityCodes::PARITY_ODD,
                                                 serialParityCodes::PARITY_EVEN, serialParityCodes::PARITY_MARK, serialParityCodes::PARITY_SPACE};
    const QStringList parityStr = QStringList() << "None" << "Odd" << "Even" << "Mark" << "Space";

    enum class s_handshakeCodes {HANDSHAKE_NONE, HANDSHAKE_XONXOFF, HANDSHAKE_HARDWARE };
    const s_handshakeCodes handshakeCodesList[] = {s_handshakeCodes::HANDSHAKE_NONE, s_handshakeCodes::HANDSHAKE_XONXOFF, s_handshakeCodes::HANDSHAKE_HARDWARE };
    const QStringList handshakeStr = QStringList() << "None" << "XON/XOFF" << "CTS/RTS";

    enum class s_forceLinesCodes {FORCE_LINE_NONE, FORCE_LINE_OFF, FORCE_LINE_ON};
    const s_forceLinesCodes forceLinesCodesList[] = {s_forceLinesCodes::FORCE_LINE_NONE, s_forceLinesCodes::FORCE_LINE_OFF, s_forceLinesCodes::FORCE_LINE_ON};
    const QStringList forceLinesStr = QStringList() << "None" << "High" << "Low";
    const QStringList forceLinstStr_Trans = QStringList() << "None" << "High" << "Low";

    const QStringList baudrateStr = QStringList() << "" << "1200" << "2400" << "4800" << "9600" << "19200" << "38400" << "57600" << "115200";
    const QStringList databitsStr = QStringList() << "" << "7" << "8" ;
    const QStringList stopbitsStr = QStringList()<< "" << "1" << "2";
    const QStringList pollTimeStr  = QStringList() << "0.5" << "1" << "2" << "3";

    enum class PTTMethodCodes {PTT_METHOD_VOX, PTT_METHOD_CAT, PTT_METHOD_DTR, PTT_METHOD_RTS, PTT_METHOD_NONE };
    const PTTMethodCodes pttMethodCodesList[] = {PTTMethodCodes::PTT_METHOD_VOX, PTTMethodCodes::PTT_METHOD_CAT, PTTMethodCodes::PTT_METHOD_DTR, PTTMethodCodes::PTT_METHOD_RTS, PTTMethodCodes::PTT_METHOD_NONE };
    const QStringList pttMethodStr = QStringList()  << "NONE" << "VOX" << "CAT" << "DTR" << "RTS" << "NONE";
}


#endif // SERIALCOMMONDATA_H
