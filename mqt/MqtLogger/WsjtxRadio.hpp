#ifndef RADIO_HPP__
#define RADIO_HPP__

#include <QString>

//
// Declarations common to radio software.
//

namespace Radio
{
  //
  // Frequency types
  //
  using Frequency = quint64;
  using FrequencyDelta = qint64;

  QString pretty_frequency_MHz_string (Radio::FrequencyDelta d, QLocale const& locale = QLocale ());

  //
  // Qt type registration
  //
  void register_types ();

}

Q_DECLARE_METATYPE (Radio::Frequency);
Q_DECLARE_METATYPE (Radio::FrequencyDelta);

#endif
