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



const int khzStep[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 25, 50, 50, 50};
const int khzPixelStep[] = {200, 150, 110, 85, 65, 50, 35, 25, 20, 15, 11, 8, 6, 4, 3, 2, 1};
//const int khzPixelStep[] = {225, 196, 169, 144, 121, 100, 81, 64, 49, 36, 25, 16, 9, 4, 1, 1, 1};
const int hzPixelStep[] = {5, 6, 9, 12, 15, 20, 28, 40, 50, 66, 90, 125, 166, 250, 333, 500, 1000};
const int minorMarker[] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 2, 2, 3, 0, 0, 0};
const int endScrollMult[] = {1, 1, 1, 1, 1, 1, 1, 1, 5, 5, 5, 5, 5, 5, 5, 5, 5};

const int fMajMrkXStart = 50;
const int fMajMrkXEnd = 70;
const int fMajMrkLength = 20;
const int fMajTextXStart = 5;

const int fMinMrkXStart = 60;
const int fMinMrkXEnd = 70;
const int fMinMrkLength = 10;

const int additionalWidth = 30;  // width in addition to freq text


const int MAXSCALEY = 675;
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

        for (int i = 0; i < bands.count() && i < startZoomLevelId.count(); i++)
        {

            startZoomLevelIdByBand.insert(bands[i].data()->uk, startZoomLevelId[i]);
        }
    }


    LOGGERPROFILE getStartZoomLevelId(QString band)
    {
        if (startZoomLevelIdByBand.contains(band))
        {
                return startZoomLevelIdByBand.value(band);
        }

        return elpBandmapStartZoomLevel_1_8MHz;
    }

    QString getStartZoomLevelName(QString band)
    {
        QString name = "bandmapStartZoomLevel_" + band.remove('\x20').replace('.', '_');
        return name;
    }



private:

    QMap <QString, LOGGERPROFILE> startZoomLevelIdByBand;



    const QList<LOGGERPROFILE> startZoomLevelId  {
                                            elpBandmapStartZoomLevel_1_8MHz,
                                            elpBandmapStartZoomLevel_3_5MHz,
                                            elpBandmapStartZoomLevel_7MHz,
                                            elpBandmapStartZoomLevel_14MHz,
                                            elpBandmapStartZoomLevel_21MHz,
                                            elpBandmapStartZoomLevel_28MHz,
                                            elpBandmapStartZoomLevel_50MHz,
                                            elpBandmapStartZoomLevel_70MHz,
                                            elpBandmapStartZoomLevel_144MHz,
                                            elpBandmapStartZoomLevel_432MHz,
                                            elpBandmapStartZoomLevel_1296MHz,
                                            elpBandmapStartZoomLevel_2300MHz,
                                            elpBandmapStartZoomLevel_3_4GHz,
                                            elpBandmapStartZoomLevel_5_6GHz,
                                            elpBandmapStartZoomLevel_10GHz};




};

#endif // BANDMAPCOMMON_H
