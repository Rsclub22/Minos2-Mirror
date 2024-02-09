#include "bandmapcommon.h"
#include "AppStartup.h"
QString BAND_LIST_INI()
{
    return getDirectoryLocation(dlConfiguration) + "/bandList.ini";
}
const char * BAND_LIST_SECT_FREQ_LOW = "BandLimitsLow";
const char * BAND_LIST_SECT_FREQ_HIGH = "BandLimitsHigh";
