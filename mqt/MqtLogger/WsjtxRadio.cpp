#include "WsjtxRadio.hpp"

namespace Radio
{
    static double constexpr MHz_factor {1.e6};
    static int constexpr frequency_precsion {6};
    QString pretty_frequency_MHz_string (Radio::FrequencyDelta d, QLocale const& locale)
    {
      auto d_string = locale.toString (d / MHz_factor, 'f', frequency_precsion);
      return d_string.insert (d_string.size () - 3, QChar::Nbsp);
    }
}
