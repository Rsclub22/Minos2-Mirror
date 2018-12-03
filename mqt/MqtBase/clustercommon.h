#ifndef CLUSTERCOMMON_H
#define CLUSTERCOMMON_H

#include "base_pch.h"
#include <QList>
#include <QColor>

// Files
const QString CLUSTER_PATH = "./Configuration/Cluster/";
const QString CLUSTER_SITES = "ClusterSites.ini";
const QString CLUSTER_COMMANDS = "ClusterCommands.ini";
const QString CLUSTER_START_FILE = "cluster_start.txt";
const QString CLUSTER_LOCATORLIST_DIR = "locatorFilterLists/";
const QString CLUSTER_CALLSIGNLIST_DIR = "callsignFilterLists/";



const int NUMBANDS = 9;
const int VHFBANDSTART = 0;
const int VHFBANDEND = 4;
const int MWBANDSTART = 4;
const int MWBANDEND = 9;

enum allBandOffsets {_50M, _70M, _144M, _432M, _1296M, _2300M, _3_4G, _5_6G, _10G};


enum allModeOffsets {CWMODE, PHONEMODE, RTTYMODE, PSKMODE, MGMMODE};
const int NUM_MODES = 5;

// OffSet to items in spot message - Note! add 1 for raw message as that has "DXSPOT" as header
const int DXCALL = 0;
const int DXLOCATOR = 1;
const int DXFREQ = 2;
const int DXBANDSTR = 3;
const int DXBANDMASK = 4;
const int DXMODESTR = 5;
const int DXMODEMASK = 6;
const int SPOTCALL = 7;
const int SPOTLOCATOR = 8;
const int SPOTTIME = 9;
const int SPOTCOMMENT = 10;
const int TTLVALUE = 11;


// Cluster Data and View Columns



const int TIME_COL_NUM = 0;
const int FREQ_COL_NUM = 1;
const int DXSPOT_CALL_COL_NUM = 2;
const int DXSPOT_CALL_WORKED_COL_NUM = 3;
const int DXLOC_COL_NUM = 4;
const int DXDIST_COL_NUM = 5;
const int DXBRG_COL_NUM = 6;
const int DXLOC_WORKED_COL_NUM = 7;
const int SPOT_CALL_COL_NUM = 8;
const int SPOTLOC_COL_NUM = 9;
const int COMMENT_COL_NUM = 10;
const int DXBANDMASK_COL_NUM = 11;
const int MODEMASK_COL_NUM = 12;
const int DXSPOT_TO_MEMORY_FLAG_COL_NUM = 13;

const int TIME_COL_WIDTH = 40;
const int FREQ_COL_WIDTH = 60;
const int DXSPOT_CALL_COL_WIDTH = 60;
const int DXSPOT_CALL_WKD_COL_WIDTH = 30;
const int DXLOC_COL_WIDTH = 50;
const int DXDIST_COL_WIDTH = 40;
const int DXBRG_COL_WIDTH = 30;
const int DXLOC_WKD_COL_WIDTH = 30;
const int SPOT_CALL_COL_WIDTH = 60;
const int SPOTLOC_COL_WIDTH = 50;
const int COMMENT_COL_WIDTH = 170;

const bool BOOL_YES = true;
const bool BOOL_NO = false;

const QChar SPOT_DATA_SEPERATOR = ':';
// Only locators in Europe
const QRegExp FULL_LOC_EXP = QRegExp("[I,i,J,j,K,k,L,l][P,p,O,o,N,n,M,m,L,l]\\d\\d[A-Za-z][A-Za-z]");
const QRegExp PART_LOC_EXP = QRegExp("[I,i,J,j,K,k,L,l][P,p,O,o,N,n,M,m,L,l]\\d\\d\\W");
const QRegExp SEARCH_LOC_EXP = QRegExp("[I,i,J,j,K,k,L,l][P,p,O,o,N,n,M,m,L,l]\\d\\d");

const int MIN_TTL = 10;
const int MAX_TTL = 180;

// cluster tab text colours
const QColor CLUSTER_TAB_SELECT_COLOR = Qt::red;
const QColor CLUSTER_TAB_NOT_SELECT_COLOR = Qt::black;

// Message headers
const QString DXSPOT = "DXSPOT:";
const QString TIMETOLIVE = "TIMETOLIVE:";

// Filter entry delimiter
const QChar FILTER_DELIMITER = ',';

const int PURGE_TIME = 1000 * 60 * 1;   // mins

//const QStringList locatorSeperators = {"<", "tr", "-"};

// Filter Change
const int FREQFILTERUP = 1 << 0;
const int CALLSIGNUP = 1 << 1;
const int LOCATORUP = 1 << 2;

// Colour of text for worked callsign and locators in cluster views
const QColor CALLSIGN_WORKED_COLOUR = Qt::red;
const QColor LOCATOR_WORKED_COLOUR = Qt::red;
const QColor SPOT_TO_MEMORY = Qt::blue;
const QColor NO_SPOT_TO_MEMORY = Qt::black;


// Callsign Edit Shortcuts
const QString ADD_CALLSIGN_KEY = "Ctrl-a";
const QString EDIT_CALLSIGN_KEY = "Ctrl-e";
const QString DEL_CALLSIGN_KEY = "Ctrl-d";

const QChar CLUSTER_START_COMMENT_DELIMTER = '#';


class ClusterClientFilterSettings
{

public:
    ClusterClientFilterSettings() :
        instanceNum(-1),
        bandFilter50Mhz(false),
        bandFilter70Mhz(false),
        bandFilter144Mhz(false),
        bandFilter432Mhz(false),
        bandFilter1296Mhz(false),
        bandFilter2300Mhz(false),
        bandFilter3_4Ghz(false),
        bandFilter5_6Ghz(false),
        bandFilter10Ghz(false),
        modeFilterCW(false),
        modeFilterPHONEMODE(false),
        modeFilterRTTYMODE(false),
        modeFilterPSKMODE(false),
        modeFilterMGMMODE(false)
    {


    }


    // note the list of callsign and locator filters strings are stored as QString for saving to contest.

    int instanceNum;

    QString callsignFilterList;
    QString locatorFilterList;

    QList<bool*> bandFilters = { &bandFilter50Mhz, &bandFilter70Mhz, &bandFilter144Mhz,
                                 &bandFilter432Mhz, &bandFilter1296Mhz, &bandFilter2300Mhz,
                                 &bandFilter3_4Ghz, &bandFilter5_6Ghz, &bandFilter10Ghz};

    QList<bool*> modeFilters = { &modeFilterCW, &modeFilterPHONEMODE, &modeFilterRTTYMODE,
                                 &modeFilterPSKMODE, &modeFilterMGMMODE};


    bool bandFilter50Mhz;
    bool bandFilter70Mhz;
    bool bandFilter144Mhz;
    bool bandFilter432Mhz;
    bool bandFilter1296Mhz;
    bool bandFilter2300Mhz;
    bool bandFilter3_4Ghz;
    bool bandFilter5_6Ghz;
    bool bandFilter10Ghz;

    bool modeFilterCW;
    bool modeFilterPHONEMODE;
    bool modeFilterRTTYMODE;
    bool modeFilterPSKMODE;
    bool modeFilterMGMMODE;




void setCallSignFilterList(QString cfl)
{
    callsignFilterList = cfl;

}

QString getCallSignFilterList()
{
    return callsignFilterList;
}



void setAllBandFilters(QList<bool> bfl)
{
    for (int i = 0; i < bfl.count(); i++)
    {
        *bandFilters[i] = bfl[i];
    }
}


bool getBandFilter(int band)
{
    bool ok  = *bandFilters[band];
    trace(QString("getbandfilter - band = %1, state = %2").arg(band).arg(ok));
    return ok;
    // return *bandFilters[band];
}


void setBandFilter(bool setting, int band)
{
    *bandFilters[band] = setting;
}



void setAllModeFilters(QList<bool> mfl)
{
    for (int i = 0; i < mfl.count(); i++)
    {
        *modeFilters[i] = mfl[i];

    }
}


bool getModeFilter(int band)
{
    return *modeFilters[band];
}

void setModeFilter(bool setting, int band)
{
    *modeFilters[band] = setting;
}

void operator= (const ClusterClientFilterSettings& ccfs)
{
    callsignFilterList = ccfs.callsignFilterList;
    locatorFilterList = ccfs.locatorFilterList;
    bandFilter50Mhz = ccfs.bandFilter50Mhz;
    bandFilter70Mhz = ccfs.bandFilter70Mhz;
    bandFilter144Mhz = ccfs.bandFilter144Mhz;
    bandFilter432Mhz = ccfs.bandFilter432Mhz;
    bandFilter1296Mhz = ccfs.bandFilter1296Mhz;
    bandFilter2300Mhz = ccfs.bandFilter2300Mhz;
    bandFilter3_4Ghz = ccfs.bandFilter3_4Ghz;
    bandFilter5_6Ghz = ccfs.bandFilter5_6Ghz;
    bandFilter10Ghz = ccfs.bandFilter10Ghz;
    modeFilterCW = ccfs.modeFilterCW;
    modeFilterPHONEMODE = ccfs.modeFilterPHONEMODE;
    modeFilterRTTYMODE = ccfs.modeFilterRTTYMODE;
    modeFilterPSKMODE = ccfs.modeFilterPSKMODE;
    modeFilterMGMMODE = ccfs.modeFilterMGMMODE;

}


bool operator==( const ClusterClientFilterSettings& ccfs ) const
{
    if ( callsignFilterList == ccfs.callsignFilterList &&
         locatorFilterList == ccfs.locatorFilterList &&
         bandFilter50Mhz == ccfs.bandFilter50Mhz &&
         bandFilter70Mhz == ccfs.bandFilter70Mhz &&
         bandFilter144Mhz == ccfs.bandFilter144Mhz &&
         bandFilter432Mhz == ccfs.bandFilter432Mhz &&
         bandFilter1296Mhz == ccfs.bandFilter1296Mhz &&
         bandFilter2300Mhz == ccfs.bandFilter2300Mhz &&
         bandFilter3_4Ghz == ccfs.bandFilter3_4Ghz &&
         bandFilter5_6Ghz == ccfs.bandFilter5_6Ghz &&
         bandFilter10Ghz == ccfs.bandFilter10Ghz &&
         modeFilterCW == ccfs.modeFilterCW &&
         modeFilterPHONEMODE == ccfs.modeFilterPHONEMODE &&
         modeFilterRTTYMODE == ccfs.modeFilterRTTYMODE &&
         modeFilterPSKMODE == ccfs.modeFilterPSKMODE &&
         modeFilterMGMMODE == ccfs.modeFilterMGMMODE)
    {
        return true;
    }

    return false;

}


bool operator!=( const ClusterClientFilterSettings& ccfs ) const
{
    return !(*this == ccfs);
}

QString packFilterList(QStringList l)
{
    QString s;
    for (int i = 0; i < l.count(); i++)
    {
        if (i != l.count() - 1)
        {
            QString t = l[i].append(FILTER_DELIMITER);
            s.append(t);
        }
        else
        {
            s.append(l[i]);  // last string
        }
    }
    return s;
}


QStringList unpackFilterList(QString &sl)
{
    QStringList fl;
    if (sl.isEmpty())
    {
        return fl;
    }
    else
    {
       fl = sl.split(FILTER_DELIMITER);
    }
    return fl;
}

};

#endif // CLUSTERCOMMON_H
