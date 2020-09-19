#ifndef RADIO_HPP__
#define RADIO_HPP__

#include "base_pch.h"

//
// Declarations common to radio software.
//

namespace Radio
{
   QString pretty_frequency_MHz_string (FrequencyDelta d, QLocale const& locale = QLocale ());

}

#endif
