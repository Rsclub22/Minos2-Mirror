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

#include "rigcontrolcommonconstants.h"

namespace maskData {

const QStringList freqMask = { "9.999",
                         "99.999",
                         "999.999",
                         "9.999.999",
                         "99.999.999",
                         "999.999.999",
                         "9.999.999.999",
                         "99.999.999.999"};
enum freqMask_lookup {
                  KHZ = 0,              /* 9 Khz   */
                  KHZ99,        		/* 99 khz  */
                  KHZ999,        		/* 999 khz */
                  MHZ,                  /* 9 Mhz   */
                  MHZ99,        		/* 99 Mhz  */
                  MHZ999,       		/* 999 Mhz */
                  GHZ,                  /* Ghz     */
                  GHZ99                 /* Ghz 99  */
};


}

namespace bandSelData {

const QStringList bandNames = { "",
                                "10m",
                                "6m",
                                "4m",
                                "2m",
                                "70cm",
                                "23cm",
                                "GHz"};

const QStringList bandFreq = { "",
                               "28170000",
                               "50170000",
                               "70210000",
                               "144290000",
                               "432170000",
                               "1296210000",
                               "10000000000"};

const QStringList freqDialZero =  { "",
                                    "00.000.000",
                                    "00.000.000",
                                    "00.000.000",
                                    "000.000.000",
                                    "000.000.000",
                                    "0.000.000.000",
                                    "00.000.000.000"};



/*

const QStringList bandFreq = { "",
                               "28.170.000",
                               "50.170.000",
                               "70.210.000",
                               "144.290.000",
                               "432.170.000",
                               "1.296.210.000",
                               "10.000.000.000"};

const QStringList freqDialZero =  { "",
                                    "00.000.000",
                                    "00.000.000",
                                    "00.000.000",
                                    "000.000.000",
                                    "000.000.000",
                                    "0.000.000.000",
                                    "00.000.000.000"};
*/

const maskData::freqMask_lookup bandMaskIdx[] = { maskData::MHZ99,      /* Not used */
                                                    maskData::MHZ99,    /* 10m  */
                                                    maskData::MHZ99,    /* 6m  */
                                                    maskData::MHZ99,    /* 4m  */
                                                    maskData::MHZ999,   /* 2m  */
                                                    maskData::MHZ999,   /* 70cm  */
                                                    maskData::GHZ,      /* 23cm  */
                                                    maskData::GHZ99};   /* GHz  */


}


namespace memDefData {

const QString DEFAULT_CALLSIGN = "";
const QString DEFAULT_FREQ = "00000000000";
const QString DEFAULT_MODE = "USB";
const QString DEFAULT_LOCATOR = "";
const int DEFAULT_BEARING = 0;
const QString DEFAULT_TIME = "00:00";

}

namespace memoryData {

class  memData
{
public:
    memData():bearing(memDefData::DEFAULT_BEARING),
        freq(memDefData::DEFAULT_FREQ), mode(memDefData::DEFAULT_MODE)
    {
    }

    QString callsign;
    QString freq;
    QString mode;
    QString locator;
    int bearing = 0;
    QString time = "00:00";

    bool operator==( const memData& rhs ) const
    {
        if (callsign.compare(rhs.callsign, Qt::CaseInsensitive) == 0
                && freq.compare(rhs.freq, Qt::CaseInsensitive) == 0
                && mode.compare(rhs.mode, Qt::CaseInsensitive) == 0
                && locator.compare(rhs.locator, Qt::CaseInsensitive) == 0)

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


namespace runButData {

const int NUM_RUNBUTTONS = 2;

const QStringList runButTitle = {
    "Run 1 ([) ",
    "Run 2 (]) "

};

}
#endif // RIGMEMCOMMONDATA_H
