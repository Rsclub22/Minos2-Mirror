/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                       Bandmap
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2019
//
///
//
//
/////////////////////////////////////////////////////////////////////////////


#ifndef BANDMAPCOMMON_H
#define BANDMAPCOMMON_H

#include <QString>
#include <QMap>
#include <QStringList>
#include "ProfileEnums.h"
#include "BandList.h"



const bool BMP_BOOL_YES = true;
const bool BMP_BOOL_NO = false;

const int SPOTMARKER_XOFFSET = 20;
const int FREQ_SEL_WIDTH = 20;
const int NO_OP_FREQ_WIDTH = 5;

const QString BANDMAP_INI_FILE = "./Configuration/bandmap.ini";

namespace dialData {



const int fMajMrkLength = 20;
const int fMajTextXStart = 5;

const int fMinMrkLength = 10;

const int additionalWidth = 30;  // width in addition to freq text


const int MAXSCALEX = 100;

const int MIN_ZOOM_LEVEL = 0;
const int MAX_ZOOM_LEVEL = 16;
const int START_ZOOM_LEVEL = MAX_ZOOM_LEVEL;


const int DIAL_VERT_OFFSET = 10;      // dial offset to show first text

}

class BandmapZoomLevelIdAndNames
{
public:

    BandmapZoomLevelIdAndNames()
    {
        QVector <QSharedPointer<BandInfo> > bands;
        BandList::getBandList().loadAllBands(bands);

        for(const auto &b: qAsConst(bands))
        {
            QString profile = b->normalisedName();
            profile = "bandmapStartZoomLevel_" + profile;
            startZoomLevelIdByBand.insert(b->uk, profile);
            startZoomLevelDefaultByBand.insert(b->uk, dialData::START_ZOOM_LEVEL);
        }
    }


    QString getStartZoomLevelId(QString band)
    {
        if (startZoomLevelIdByBand.contains(band))
        {
                return startZoomLevelIdByBand.value(band);
        }

        return startZoomLevelIdByBand.value("1.8 MHz");
    }

    int getStartZoomLevelDefault(QString band)
    {
        if (startZoomLevelIdByBand.contains(band))
        {
                return startZoomLevelDefaultByBand.value(band);
        }

        return startZoomLevelDefaultByBand.value("1.8 MHz");
    }


private:

    QMap <QString, QString> startZoomLevelIdByBand;

    QMap <QString, int> startZoomLevelDefaultByBand;

};

#endif // BANDMAPCOMMON_H
