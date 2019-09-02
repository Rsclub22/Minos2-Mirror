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
const QString CLUSTER_END_FILE = "cluster_end.txt";
const QString CLUSTER_LOCATORLIST_DIR = "locatorFilterLists/";
const QString CLUSTER_CALLSIGNLIST_DIR = "callsignFilterLists/";
const QString CLUSTER_SETTINGS_FILE = "./Configuration/Cluster/ClusterSettings.ini";

const int NO_BANDS = -1;
const int NUMBANDS = 9;
const int VHFBANDSTART = 0;
const int VHFBANDEND = 4;
const int MWBANDSTART = 4;
const int MWBANDEND = 9;

enum allBandOffsets {_50M, _70M, _144M, _432M, _1296M, _2300M, _3_4G, _5_6G, _10G};
const QStringList clusterBands = QStringList() << "50 MHz" << "70 MHz" << "144 MHz" << "432 MHz" << "1296 MHz" << "2300 MHz" << "3.4 GHz" << "5.6 GHz" << "10 GHz";


enum allModeOffsets {CWMODE, PHONEMODE, FM_MODE, RTTYMODE, PSKMODE, FT8_MODE, MSK144_MODE, JT65_MODE};
const QStringList clusterModes = QStringList() << "CW" << "USB" << "FM" << "RTTY" << "PSK31" << "FT8" << "MSK144" << "JT65";



const int NUM_MODES = clusterModes.count();

const QStringList clusterPropModes = QStringList() << "TR" << "ES" << "MS" << "EME";
enum bandPlanModeError {MODE_FREQ_MATCH, NO_MODE_FREQ_MATCH, MODE_NOT_FOUND, BAND_NOT_FOUND};

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
const int SPOTDATE = 10;
const int SPOTCOMMENT = 11;
const int DXPROPMODE = 12;
const int TTLVALUE = 13;


// Cluster Data and View Columns



const int TIME_COL_NUM = 0;
const int FREQ_COL_NUM = 1;
const int DXSPOT_CALL_COL_NUM = 2;
const int DXSPOT_CALL_WORKED_COL_NUM = 3;
const int DXSPOT_MODE_COL_NUM = 4;
const int DXLOC_COL_NUM = 5;
const int DXDIST_COL_NUM = 6;
const int DXBRG_COL_NUM = 7;
const int DXLOC_WORKED_COL_NUM = 8;
const int SPOT_CALL_COL_NUM = 9;
const int SPOTLOC_COL_NUM = 10;
const int COMMENT_COL_NUM = 11;
const int DXBANDMASK_COL_NUM = 12;
const int DXMODEMASK_COL_NUM = 13;
const int DXSPOT_TO_MEMORY_FLAG_COL_NUM = 14;
const int DXSPOT_PROP_MODE_COL_NUM = 15;
const int RXTIME_COL_NUM = 16;
const int SPOT_TYPE_COL_NUM = 17;       // used in bandmap
const int SPOT_IS_SELECTED_COL_NUM = 18;    // used in bandmap




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
//const QString TIMETOLIVE = "TIMETOLIVE:";
const QString CLUSTER_STATUS = "CLUSTER_STATUS!";
const QChar CLUSTER_STAT_DELIMITER = '!';

// Filter entry delimiter
const QChar FILTER_DELIMITER = ',';

const int PURGE_TIME = 1000 * 60 * 1;   // mins
const int CHECKSPOTS_DURATION = 1000;
const int CHECK_NEWFILTERS_DURATION = 1000;
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
const QColor NOT_WORKED_COLOUR = Qt::black;

// Callsign Edit Shortcuts
const QString ADD_CALLSIGN_KEY = "Ctrl-a";
const QString EDIT_CALLSIGN_KEY = "Ctrl-e";
const QString DEL_CALLSIGN_KEY = "Ctrl-d";

const QChar CLUSTER_START_COMMENT_DELIMTER = '#';

const QString NEWSPOT_INDICATOR_OFF_STYLE = QString("background-color: yellow ;\n");
const QString NEWSPOT_INDICATOR_ON_STYLE = QString("background-color: orange ;\n");
const QString STATUS_INDICATOR_DISCONNECT_STYLE = QString("background-color: yellow;\n");
const QString STATUS_INDICATOR_CONNECT_STYLE = QString("background-color: orange;\n");


QDateTime getSpotDateTime(const QString spotDate, const QString spotTime);

bool spotTimedOut(qlonglong spotTime, qlonglong timeToLive);

class ClusterClientFilterSettings
{

public:
    ClusterClientFilterSettings() :

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
        modeFilterUSBMODE(false),
        modeFilterFMMODE(false),
        modeFilterRTTYMODE(false),
        modeFilterPSK31MODE(false),
        modeFilterFT8MODE(false),
        modeFilterMSK144MODE(false),
        modeFilterJT65MODE(false)
    {


    }

// search for data in cluster comments
    QStringList propFilterStrings = {"TR", "ES", "MS", "EME"};
    QStringList modeFilterStrings = {"CW","USB", "RTTY", "PSK31", "FM", "FT8", "MSK144", "FSK441"};


    // note the list of callsign and locator filters strings are stored as QString for saving to contest.



    QString callsignFilterList;
    QString locatorFilterList;

    QList<bool*> bandFilters = { &bandFilter50Mhz, &bandFilter70Mhz, &bandFilter144Mhz,
                                 &bandFilter432Mhz, &bandFilter1296Mhz, &bandFilter2300Mhz,
                                 &bandFilter3_4Ghz, &bandFilter5_6Ghz, &bandFilter10Ghz};

    QList<bool*> modeFilters = { &modeFilterCW, &modeFilterUSBMODE, &modeFilterFMMODE,
                                 &modeFilterRTTYMODE, &modeFilterPSK31MODE, &modeFilterFT8MODE,
                                &modeFilterMSK144MODE, &modeFilterJT65MODE};


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
    bool modeFilterUSBMODE;
    bool modeFilterFMMODE;
    bool modeFilterRTTYMODE;
    bool modeFilterPSK31MODE;
    bool modeFilterFT8MODE;
    bool modeFilterMSK144MODE;
    bool modeFilterJT65MODE;




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
    if (band >= 0 && band < bandFilters.count())
    {
        return *bandFilters[band];
    }
    else
    {
        return false;
    }
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


bool getModeFilter(int mode)
{
    if (mode >= 0 && mode < modeFilters.count())
    {
        return *modeFilters[mode];
    }
    else
    {
        return false;
    }
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
    modeFilterUSBMODE = ccfs.modeFilterUSBMODE;
    modeFilterFMMODE = ccfs.modeFilterFMMODE;
    modeFilterRTTYMODE = ccfs.modeFilterRTTYMODE;
    modeFilterPSK31MODE = ccfs.modeFilterPSK31MODE;
    modeFilterFT8MODE = ccfs.modeFilterFT8MODE;
    modeFilterMSK144MODE = ccfs.modeFilterMSK144MODE;
    modeFilterJT65MODE = ccfs.modeFilterJT65MODE;

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
         modeFilterUSBMODE == ccfs.modeFilterUSBMODE &&
         modeFilterFMMODE == ccfs.modeFilterFMMODE &&
         modeFilterRTTYMODE == ccfs.modeFilterRTTYMODE &&
         modeFilterPSK31MODE == ccfs.modeFilterPSK31MODE &&
         modeFilterFT8MODE == ccfs.modeFilterFT8MODE &&
         modeFilterMSK144MODE == ccfs.modeFilterMSK144MODE &&
         modeFilterJT65MODE == ccfs.modeFilterJT65MODE)
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
