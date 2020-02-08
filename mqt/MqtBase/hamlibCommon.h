#ifndef HAMLIBCOMMON_H
#define HAMLIBCOMMON_H

#include <hamlib/rotator.h>


namespace hamlibSerialData
{



    const serial_parity_e parityCodes[] = {RIG_PARITY_NONE, RIG_PARITY_ODD, RIG_PARITY_EVEN, RIG_PARITY_MARK, RIG_PARITY_SPACE};
    const serial_handshake_e handshakeCodes[] = { RIG_HANDSHAKE_NONE, RIG_HANDSHAKE_XONXOFF, RIG_HANDSHAKE_HARDWARE };

    enum serial_parity_e getSerialParityCode(int index){return parityCodes[index];}
    enum serial_handshake_e getSerialHandshakeCode(int index){return handshakeCodes[index];}


}

namespace hamlibText
{

const char * hamlibErrorMsg[] = {QT_TR_NOOP("No Error, operation completed sucessfully"),
                                    QT_TR_NOOP("Invalid parameter"),
                                    QT_TR_NOOP("Invalid configuration"),
                                    QT_TR_NOOP("Memory shortage"),
                                    QT_TR_NOOP("Function not implemented"),
                                    QT_TR_NOOP("Communication timed out"),
                                    QT_TR_NOOP("IO error, including open failed"),
                                    QT_TR_NOOP("Internal Hamlib error"),
                                    QT_TR_NOOP("Protocol error"),
                                    QT_TR_NOOP("Command rejected by the rig"),
                                    QT_TR_NOOP("Command performed, but arg truncated"),
                                    QT_TR_NOOP("Function not available"),
                                    QT_TR_NOOP("VFO not targetable"),
                                    QT_TR_NOOP("Error talking on the bus"),
                                    QT_TR_NOOP("Collision on the bus"),
                                    QT_TR_NOOP("NULL RIG handle or any invalid pointer parameter in get arg"),
                                    QT_TR_NOOP("Invalid VFO"),
                                    QT_TR_NOOP("RIG_EDOM")};

}


namespace hamlibErrorTxt
{

const char * hamlibErrorMsg[] = {QT_TR_NOOP("No Error, operation completed sucessfully"),
                                    QT_TR_NOOP("Invalid parameter"),
                                    QT_TR_NOOP("Invalid configuration"),
                                    QT_TR_NOOP("Memory shortage"),
                                    QT_TR_NOOP("Function not implemented"),
                                    QT_TR_NOOP("Communication timed out"),
                                    QT_TR_NOOP("IO error, including open failed"),
                                    QT_TR_NOOP("Internal Hamlib error"),
                                    QT_TR_NOOP("Protocol error"),
                                    QT_TR_NOOP("Command rejected by the rig"),
                                    QT_TR_NOOP("Command performed, but arg truncated"),
                                    QT_TR_NOOP("Function not available"),
                                    QT_TR_NOOP("VFO not targetable"),
                                    QT_TR_NOOP("Error talking on the bus"),
                                    QT_TR_NOOP("Collision on the bus"),
                                    QT_TR_NOOP("NULL RIG handle or any invalid pointer parameter in get arg"),
                                    QT_TR_NOOP("Invalid VFO"),
                                    QT_TR_NOOP("RIG_EDOM")};

}


#endif // HAMLIBCOMMON_H
