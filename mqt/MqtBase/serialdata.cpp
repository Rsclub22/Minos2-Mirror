#include "serialdata.h"

const serial_parity_e serialData::parityCodes[] = {RIG_PARITY_NONE, RIG_PARITY_ODD, RIG_PARITY_EVEN, RIG_PARITY_MARK, RIG_PARITY_SPACE};
const serial_handshake_e serialData::handshakeCodes[] = { RIG_HANDSHAKE_NONE, RIG_HANDSHAKE_XONXOFF, RIG_HANDSHAKE_HARDWARE };
const serialData::serial_force_Lines_e serialData::forceLinesCodes[] = {serialData::FORCE_LINE_NONE, serialData::FORCE_LINE_OFF, serialData::FORCE_LINE_ON};
const QStringList serialData::parityStr = (QStringList() << tr("None") << tr("Odd") << tr("Even") << tr("Mark") << tr("Space"));
const QStringList serialData::handshakeStr = (QStringList() << tr("None") << tr("XON/XOFF") << tr("CTS/RTS"));
const QStringList serialData::rigctldHandshakeStr = (QStringList() << "None" << "XONXOFF" << "Hardware");   // passed to rigctld
const QStringList serialData::forceLinesStr = QStringList () << tr("None") << tr("High") << tr("Low");
const QStringList serialData::rigctldForceLinesStr = QStringList() << "Unset" << "ON" << "OFF";   // passed to rigctld
const QStringList serialData::baudrateStr = (QStringList() << "" << "1200" << "2400" << "4800" << "9600" << "19200" << "38400" << "57600" << "115200");
const QStringList serialData::databitsStr = (QStringList() << "" << "7" << "8" );
const QStringList serialData::stopbitsStr = (QStringList()<< "" << "1" << "2");
