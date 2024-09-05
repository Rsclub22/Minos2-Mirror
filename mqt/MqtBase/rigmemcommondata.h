/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2017
//
// Interprocess Control Logic
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2017
//
//
//
/////////////////////////////////////////////////////////////////////////////



#ifndef RIGMEMCOMMONDATA_H
#define RIGMEMCOMMONDATA_H

#include <QColor>
#include "rigcontrolcommonconstants.h"

namespace maskData
{

inline const QStringList freqMask = { "9.999",
                         "99.999",
                         "999.999",
                         "9.999.999",
                         "99.999.999",
                         "999.999.999",
                         "9.999.999.999",
                         "99.999.999.999",
                         "999.999.999.999"
                             };
}

namespace memDefData
{

inline const QString DEFAULT_CALLSIGN = "";
//const QString DEFAULT_FREQ = "0000000000";
inline const QString DEFAULT_FREQ = "";
inline const QString DEFAULT_MODE = hamlibData::USB;
inline const QString DEFAULT_LOCATOR = "";
inline const int DEFAULT_BEARING = 0;
inline const QString DEFAULT_TIME = "00:00";

}

namespace memoryData
{

class  memData
{
public:
    memData():
        freq(memDefData::DEFAULT_FREQ)
      , mode(memDefData::DEFAULT_MODE)
      , bearing(memDefData::DEFAULT_BEARING)
    {
    }

    int memno = 0;

    QString callsign;
    Frequency freq;
    QString band;
    QString mode;
    QString locator;
    QString exchange;
    int bearing = 0;
    QString time = "00:00";
    bool worked = false;
    bool fromBandmapOrMemory = false;
    bool dxLocFromNode = false;

    QColor headerColor = Qt::black;

    bool operator==( const memData& rhs ) const
    {
        if (callsign.compare(rhs.callsign, Qt::CaseInsensitive) == 0
                && freq == rhs.freq
                && band == rhs.band
                && mode.compare(rhs.mode, Qt::CaseInsensitive) == 0
                && locator.compare(rhs.locator, Qt::CaseInsensitive) == 0
                && exchange.compare(rhs.exchange, Qt::CaseInsensitive) == 0
                && worked == rhs.worked
                && fromBandmapOrMemory == rhs.fromBandmapOrMemory
                && time.compare(rhs.time, Qt::CaseInsensitive) == 0
                && bearing == rhs.bearing
                && dxLocFromNode == rhs.dxLocFromNode
                )

        {
            return true;
        }
        return false;
    }
    bool operator!=( const memData& rhs ) const
    {
        return !(*this == rhs);
    }
};


}


namespace runButData
{

const int NUM_RUNBUTTONS = 2;

inline const QStringList runButTitle = {
    "Run 1 ([) ",
    "Run 2 (]) "
};
}
#endif // RIGMEMCOMMONDATA_H
