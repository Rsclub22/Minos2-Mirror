#ifndef SERIALDATA_H
#define SERIALDATA_H

#include <QObject>
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
    static const QStringList parityStr;
    static const QStringList handshakeStr;
    static const QStringList rigctldHandshakeStr;
    static const QStringList forceLinesStr;
    static const QStringList rigctldForceLinesStr;
    static const QStringList baudrateStr;
    static const QStringList databitsStr;
    static const QStringList stopbitsStr;
};

#endif // SERIALDATA_H
