#ifndef SERIALDATA_H
#define SERIALDATA_H

#include <QObject>
#include <QStringList>

#include "hamlib/rig.h"
class serialData: public QObject
{
    Q_OBJECT

    // we never instantiate - just a carrier for the translations

    serialData() = delete;
    serialData(const serialData &) = delete;
public:

    enum serial_force_Lines_e {FORCE_LINE_NONE, FORCE_LINE_OFF, FORCE_LINE_ON};

    static const serial_parity_e parityCodes[];
    static const serial_handshake_e handshakeCodes[];
    static const serial_force_Lines_e forceLinesCodes[];
    static QStringList parityStr;
    static QStringList handshakeStr;
    static QStringList rigctldHandshakeStr;
    static QStringList forceLinesStr;
    static QStringList rigctldForceLinesStr;
    static QStringList baudrateStr;
    static QStringList databitsStr;
    static QStringList stopbitsStr;

    static void translateSerialData();
};

#endif // SERIALDATA_H
