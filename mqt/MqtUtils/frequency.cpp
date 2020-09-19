#include "mqtUtils_pch.h"

void register_frequency_types ()
{
  qRegisterMetaType<Frequency> ("Frequency");
  qRegisterMetaType<FrequencyDelta> ("FrequencyDelta");
  qRegisterMetaType<ShortFreq> ("ShortFreq");
}
