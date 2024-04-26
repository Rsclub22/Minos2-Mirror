#ifndef SERIALCOMMONDATA_H
#define SERIALCOMMONDATA_H

#include <QStringList>

namespace serialCommonData
{
    enum class serialParityCodes  {MINOS_PARITY_NONE, MINOS_PARITY_ODD, MINOS_PARITY_EVEN, MINOS_PARITY_MARK, MINOS_PARITY_SPACE};  // added MINOS_ to prevent compile errors - clash with PARITY_NONE defined in windows.h
    const serialParityCodes parityCodesList[] = {serialParityCodes::MINOS_PARITY_NONE, serialParityCodes::MINOS_PARITY_ODD, serialParityCodes::MINOS_PARITY_EVEN, serialParityCodes::MINOS_PARITY_MARK, serialParityCodes::MINOS_PARITY_SPACE};
    const QStringList parityStr = QStringList() << "None" << "Odd" << "Even" << "Mark" << "Space";

    enum  class s_handshakeCodes {HANDSHAKE_NONE, HANDSHAKE_XONXOFF, HANDSHAKE_HARDWARE };
    const s_handshakeCodes handshakeCodesList[] = {s_handshakeCodes::HANDSHAKE_NONE, s_handshakeCodes::HANDSHAKE_XONXOFF, s_handshakeCodes::HANDSHAKE_HARDWARE };
    const QStringList handshakeStr = QStringList() << "None" << "XON/XOFF" << "CTS/RTS";

    enum  class s_forceLinesCodes {FORCE_LINE_NONE, FORCE_LINE_OFF, FORCE_LINE_ON};
    const s_forceLinesCodes forceLinesCodesList[] = {s_forceLinesCodes::FORCE_LINE_NONE, s_forceLinesCodes::FORCE_LINE_OFF, s_forceLinesCodes::FORCE_LINE_ON};
    const QStringList forceLinesStr = QStringList() << "None" << "High" << "Low";
    const QStringList forceLinstStr_Trans = QStringList() << "None" << "High" << "Low";

    const QStringList baudrateStr = QStringList() << "" << "1200" << "2400" << "4800" << "9600" << "19200" << "38400" << "57600" << "115200";
    const QStringList databitsStr = QStringList() << "" << "7" << "8" ;
    const QStringList stopbitsStr = QStringList()<< "" << "1" << "2";
    const QStringList pollTimeStr  = QStringList() << "0.5" << "1" << "2" << "3";

    enum  class MINOS_PTT_TYPES {PTT_TYPE_NONE, PTT_TYPE_VOX, PTT_TYPE_CAT, PTT_TYPE_DTR, PTT_TYPE_RTS};
    const MINOS_PTT_TYPES pttTypeCodesList[] = { MINOS_PTT_TYPES::PTT_TYPE_NONE, MINOS_PTT_TYPES::PTT_TYPE_VOX, MINOS_PTT_TYPES::PTT_TYPE_CAT, MINOS_PTT_TYPES::PTT_TYPE_DTR, MINOS_PTT_TYPES::PTT_TYPE_RTS};
    const QStringList pttTypeStr = QStringList() << "NONE" << "VOX" << "CAT" << "DTR" << "RTS" ;
}


#endif // SERIALCOMMONDATA_H
