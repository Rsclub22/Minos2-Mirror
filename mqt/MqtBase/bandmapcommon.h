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

// Cluster Data and View Columns



//const int BMP_TIME_COL_NUM = 0;
//const int BMP_FREQ_COL_NUM = 1;
//const int BMP_DXSPOT_CALL_COL_NUM = 2;
//const int BMP_DXSPOT_CALL_WORKED_COL_NUM = 3;
//const int BMP_DXLOC_COL_NUM = 4;
//const int BMP_DXDIST_COL_NUM = 5;
//const int BMP_DXBRG_COL_NUM = 6;
//const int BMP_DXLOC_WORKED_COL_NUM = 7;
//const int BMP_SPOT_CALL_COL_NUM = 8;
//const int BMP_SPOTLOC_COL_NUM = 9;
//const int BMP_COMMENT_COL_NUM = 10;
//const int BMP_DXBANDMASK_COL_NUM = 11;
//const int BMP_MODEMASK_COL_NUM = 12;
//const int BMP_DXSPOT_TO_MEMORY_FLAG_COL_NUM = 13;
//const int BMP_RXTIME_COL_NUM = 14;

//const int BMP_TIME_COL_WIDTH = 40;
//const int BMP_FREQ_COL_WIDTH = 60;
//const int BMP_DXSPOT_CALL_COL_WIDTH = 60;
//const int BMP_DXSPOT_CALL_WKD_COL_WIDTH = 30;
//const int BMP_DXLOC_COL_WIDTH = 50;
//const int BMP_DXDIST_COL_WIDTH = 40;
//const int BMP_DXBRG_COL_WIDTH = 30;
//const int BMP_DXLOC_WKD_COL_WIDTH = 30;
//const int BMP_SPOT_CALL_COL_WIDTH = 60;
//const int BMP_SPOTLOC_COL_WIDTH = 50;
//const int BMP_COMMENT_COL_WIDTH = 170;

const bool BMP_BOOL_YES = true;
const bool BMP_BOOL_NO = false;

const int SPOTMARKER_XOFFSET = 20;
const int FREQ_SEL_WIDTH = 20;
const int NO_OP_FREQ_WIDTH = 5;

const QString BANDMAP_INI_FILE = "./Configuration/bandmap.ini";

namespace dialData {



const int khzStep[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 25, 50, 50, 50};
const int khzPixelStep[] = {200, 150, 110, 85, 65, 50, 35, 25, 20, 15, 11, 8, 6, 4, 3, 2, 1};
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
    LOGGERPROFILE getStartZoomLevelId(int i){return startZoomLevelId[i];}
    QString getStartZoomLevelName(int i){return startZoomLevelName[i];}
    int getStartZoomLevelNameCount(){return startZoomLevelName.count();}


private:

    const LOGGERPROFILE startZoomLevelId [9] {elpBandmapStartZoomLevel_50MHz,
                                            elpBandmapStartZoomLevel_70MHz,
                                            elpBandmapStartZoomLevel_144MHz,
                                            elpBandmapStartZoomLevel_432MHz,
                                            elpBandmapStartZoomLevel_1296MHz,
                                            elpBandmapStartZoomLevel_2300MHz,
                                            elpBandmapStartZoomLevel_3_4GHz,
                                            elpBandmapStartZoomLevel_5_6GHz,
                                            elpBandmapStartZoomLevel_10GHz};

    const QStringList startZoomLevelName  {"bandmapStartZoomLevel_50MHz",
                                "bandmapStartZoomLevel_70MHz",
                                "bandmapStartZoomLevel_144MHz",
                                "bandmapStartZoomLevel_432MHz",
                                "bandmapStartZoomLevel_1296MHz",
                                "bandmapStartZoomLevel_2300MHz",
                                "bandmapStartZoomLevel_3_4GHz",
                                "bandmapStartZoomLevel_5_6GHz",
                                "bandmapStartZoomLevel_10GHz"};


};

#endif // BANDMAPCOMMON_H
