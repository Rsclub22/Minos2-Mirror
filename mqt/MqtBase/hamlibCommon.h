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


#endif // HAMLIBCOMMON_H
