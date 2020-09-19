#ifndef FREQUENCY_H
#define FREQUENCY_H

#include <QMetaType>

using  Frequency = quint64;
using FrequencyDelta = qint64;

using ShortFreq = qint32;

//
// Qt type registration
//
void register_frequency_types ();

Q_DECLARE_METATYPE (Frequency);
Q_DECLARE_METATYPE (FrequencyDelta);
Q_DECLARE_METATYPE (ShortFreq);

#endif // FREQUENCY_H
