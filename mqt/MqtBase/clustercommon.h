#ifndef CLUSTERCOMMON_H
#define CLUSTERCOMMON_H


#include "base_pch.h"
#include <QList>
#include <QColor>

extern const char * clusterStateList[];

class checkModeAgainstFreq;
class BandInfo;

// Files
const QString CLUSTER_PATH = "./Configuration/Cluster/";
const QString CLUSTER_SITES = "ClusterSites.ini";
const QString CLUSTER_COMMANDS = "ClusterCommands.ini";
const QString CLUSTER_START_FILE = "cluster_start.txt";
const QString CLUSTER_END_FILE = "cluster_end.txt";
const QString CLUSTER_LOCATORLIST_DIR = "locatorFilterLists/";
const QString CLUSTER_CALLSIGNLIST_DIR = "callsignFilterLists/";
const QString CLUSTER_SETTINGS_FILE = "./Configuration/Cluster/ClusterSettings.ini";
const QString CLUSTER_FILTER_FILE = "./Configuration/clusterBandmapFilter.ini";



const int NO_BANDS = -1;
const int NUMBANDS = 9;
const int VHFBANDSTART = 0;
const int VHFBANDEND = 4;
const int MWBANDSTART = 4;
const int MWBANDEND = 9;

enum allBandOffsets {_50M, _70M, _144M, _432M, _1296M, _2300M, _3_4G, _5_6G, _10G};
const QStringList clusterBands = QStringList() << "50 MHz" << "70 MHz" << "144 MHz" << "432 MHz" << "1296 MHz" << "2300 MHz" << "3.4 GHz" << "5.6 GHz" << "10 GHz";


enum allModeOffsets {NO_MODE, CW_MODE, USB_MODE, FM_MODE, RTTY_MODE, PSK31_MODE, FT8_MODE, MSK144_MODE, JT65_MODE};
const QStringList clusterModes = QStringList() << "None" << "CW" << "USB" << "FM" << "RTTY" << "PSK31" << "FT8" << "MSK144" << "JT65";


const QStringList clusterPropModes = QStringList() << "TR" << "ES" << "MS" << "EME";
enum bandPlanModeError {MODE_FREQ_MATCH, NO_MODE_FREQ_MATCH, MODE_NOT_FOUND, BAND_NOT_FOUND};

enum clusterErrorCode {SPOT_OK, NO_SPOT_TIME, SPOT_DATETIME_INVALID, SPOT_TOO_MANY_SECTIONS, DISCARD_HF_SPOT, GET_PREFIX_FAILED, ASKQRZ_FAILED_QRA};
const QStringList clusterErrorMsg = QStringList() << "Spot OK" << "SpotTime not found" << "Spot DateTime Invalid" << "Spot too many sections"
                                                  << "Discard HF Spot" << "getPrefix failed to find QRA" << "AskQrz Failed to Find QRA";
// OffSet to items in spot message - Note! add 1 for raw message as that has "DXSPOT" as header
const int DXCALL = 0;
const int DXLOCATOR = 1;
const int DXLOC_FROM_NODE_FLAG = 2;
const int DXFREQ = 3;
const int DXBANDSTR = 4;
const int DXBANDMASK = 5;
const int DXMODESTR = 6;
const int DXMODEMASK = 7;
const int SPOTCALL = 8;
const int SPOTLOCATOR = 9;
const int SPOTTIME = 10;
const int SPOTDATE = 11;
const int SPOTCOMMENT = 12;
const int DXPROPMODE = 13;
const int TTLVALUE = 14;


// Cluster Data and View Columns

const int TIME_COL_NUM = 0;
const int FREQ_COL_NUM = 1;
const int DXSPOT_CALL_COL_NUM = 2;
const int DXSPOT_CALL_WORKED_COL_NUM = 3;
const int DXSPOT_MODE_COL_NUM = 4;
const int DXLOC_COL_NUM = 5;
const int DXLOC_FROM_NODE_FLAG_COL_NUM = 6;
const int DXDIST_COL_NUM = 7;
const int DXBRG_COL_NUM = 8;
const int DXLOC_WORKED_COL_NUM = 9;
const int SPOTTER_CALL_COL_NUM = 10;
const int SPOTTER_LOC_COL_NUM = 11;
const int COMMENT_COL_NUM = 12;
const int DXBANDSTR_COL_NUM = 13;
const int DXBANDMASK_COL_NUM = 14;
const int DXMODEMASK_COL_NUM = 15;
const int DXSPOT_TO_MEMORY_FLAG_COL_NUM = 16;
const int DXSPOT_PROP_MODE_COL_NUM = 17;
const int RXTIME_COL_NUM = 18;
const int DATE_COL_NUM = 19;
const int DATE_TIME_COL_NUM = 20;
const int SPOT_TYPE_COL_NUM = 21;       // used in bandmap
const int SPOT_IS_SELECTED_COL_NUM = 22;    // used in bandmap
const int ROT_BEARING_COL_NUM = 23;     // used in bandmap
const int ROT_CONNECTED_COL_NUM = 24;   // used in bandmap
const int RUN_MODE_ON_COL_NUM = 25;     // used in bandmap
const int OFF_RUN_FREQ_COL_NUM = 26;    // used in bandmap
const int DX_DISTRICT_COL_NUM = 27;   // used in bandmap
const int DX_DISTRICT_WORKED_COL_NUM = 28; // used in bandmap

const bool BOOL_YES = true;
const bool BOOL_NO = false;

const QString SPOT_TX_ON = "TxSpotOn";
const QString SPOT_TX_OFF = "TxSpotOff";

const QChar SPOT_DATA_SEPERATOR = ':';
// Only locators in Europe
const QRegularExpression FULL_LOC_EXP = QRegularExpression("[I,i,J,j,K,k,L,l][P,p,O,o,N,n,M,m,L,l]\\d\\d[A-Za-z][A-Za-z]");
const QRegularExpression PART_LOC_EXP = QRegularExpression("[I,i,J,j,K,k,L,l][P,p,O,o,N,n,M,m,L,l]\\d\\d\\W");
const QRegularExpression SEARCH_LOC_EXP = QRegularExpression("[I,i,J,j,K,k,L,l][P,p,O,o,N,n,M,m,L,l]\\d\\d");

const int MIN_TTL = 10;
const int MAX_TTL = 180;

const int NEW_SPOT_TIME = 3 * 60;  //seconds

// cluster tab text colours
const QColor CLUSTER_TAB_SELECT_COLOR = Qt::red;
const QColor CLUSTER_TAB_NOT_SELECT_COLOR = Qt::black;

// Message headers
const QString DXSPOT = "DXSPOT:";
//const QString TIMETOLIVE = "TIMETOLIVE:";
const QString CLUSTER_STATUS = "CLUSTER_STATUS!";
const QChar CLUSTER_STAT_DELIMITER = '!';
const QString RESENTSPOT = "RESENTSPOT:";

// Filter entry delimiter
const QChar FILTER_DELIMITER = ',';

const int PURGE_TIME = 1000 * 60 * 1;   // mins
const int CHECKSPOTS_DURATION = 1000;
const int CHECK_NEWFILTERS_DURATION = 1000;
//const QStringList locatorSeperators = {"<", "tr", "-"};
const int MOUSE_IN_FRAME_TIMEOUT = 10000;


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
const QColor MARKED_SPOT_COLOUR = Qt::magenta;
const QColor CLUSTER_SPOT_COLOUR = Qt::darkGreen;
const QColor BANDMAP_NEW_COLOUR = Qt::blue;
const QColor CQ_FREQ_COLOUR = Qt::red;

// Callsign Edit Shortcuts
const QString ADD_CALLSIGN_KEY = "Ctrl-a";
const QString EDIT_CALLSIGN_KEY = "Ctrl-e";
const QString DEL_CALLSIGN_KEY = "Ctrl-d";

const QChar CLUSTER_START_COMMENT_DELIMTER = '#';

const QString NEWSPOT_INDICATOR_OFF_STYLE = QString("background-color: yellow ;\n");
const QString NEWSPOT_INDICATOR_ON_STYLE = QString("background-color: orange ;\n");
const QString STATUS_INDICATOR_DISCONNECT_STYLE = QString("background-color: yellow;\n");
const QString STATUS_INDICATOR_CONNECT_STYLE = QString("background-color: orange;\n");

// resend clusterspots to client commands
enum resendFrameId {ALL_CLIENTS = -1, CLUSTER_CLIENT = 0, BANDMAP_CLIENT};
const QString RESEND_ALL_SPOTS = "resendAll";



QDateTime getSpotDateTime(const QString spotDate, const QString spotTime);

bool spotTimedOut(qlonglong spotTime, qlonglong timeToLive);

qlonglong spotElapsedTime(qlonglong spotTime);

void getMode(checkModeAgainstFreq* modeBandPlan, Frequency freq, const QString &dxBand, QString &dxModeStr, QString &dxModeMask);

void getBand(QVector<QSharedPointer<BandInfo> > &bands, Frequency freq, QString &band, QString &bandMask);

bool extractDxLocFromNodeFlag(QString locFlagMsg);


class ClusterMessage
{
public:
    void setMessage(QString msg){message = msg;}
    QString getMessage(){return message;}

    void setLoggerUuid(QString uuid){loggerUuid = uuid;}
    QString getLoggerUuid(){return loggerUuid;}

    void setFrameId(int frameId_){frameId = static_cast<resendFrameId>(frameId_);}
    resendFrameId getFrameId(){return frameId;}

private:
    QString message;
    QString loggerUuid;
    resendFrameId frameId;
};


class ClusterClientFilterSettings
{

public:
    ClusterClientFilterSettings()
{

        // init class values

        for (int i = 0; i < bandFilters.count(); i++)
        {
            *bandFilters[i] = false;
        }

        for (int i = 0; i < modeFilters.count(); i++)
        {
            *modeFilters[i] = false;
        }

        for (int i = 0; i < distanceFilters.count(); i++)
        {
            *distanceFilters[i] = 0;
        }


        for (int i = 0; i < ignoreDistanceFlags.count(); i++)
        {
            *ignoreDistanceFlags[i] = false;
        }

        for (int i = 0; i < ignoreEmptyDistanceFlags.count(); i++)
        {
            *ignoreEmptyDistanceFlags[i] = false;
        }




    }


    // note the list of callsign and locator filters strings are stored as QString for saving to contest.



    QString callsignFilterList;
    QString locatorFilterList;

    QList<bool*> bandFilters = { &bandFilter50Mhz, &bandFilter70Mhz, &bandFilter144Mhz,
                                 &bandFilter432Mhz, &bandFilter1296Mhz, &bandFilter2300Mhz,
                                 &bandFilter3_4Ghz, &bandFilter5_6Ghz, &bandFilter10Ghz};

    QList<bool*> modeFilters = { &modeFilterNONE, &modeFilterCW, &modeFilterUSBMODE, &modeFilterFMMODE,
                                 &modeFilterRTTYMODE, &modeFilterPSK31MODE, &modeFilterFT8MODE,
                                &modeFilterMSK144MODE, &modeFilterJT65MODE};

    QList<int*> distanceFilters = {&distanceFilter50MHz, &distanceFilter70MHz, &distanceFilter144MHz,
                                   &distanceFilter432MHz, &distanceFilter1296MHz, &distanceFilter2300MHz,
                                   &distanceFilter3_4GHz, &distanceFilter5_6GHz, &distanceFilter10GHz};

    QList<bool*> ignoreDistanceFlags = {&ignoreDistanceFlag_50MHz, &ignoreDistanceFlag_70MHz, &ignoreDistanceFlag_144MHz,
                                        &ignoreDistanceFlag_432MHz, &ignoreDistanceFlag_1296MHz, &ignoreDistanceFlag_2300MHz,
                                        &ignoreDistanceFlag_3_4GHz, &ignoreDistanceFlag_5_6GHz, &ignoreDistanceFlag_10GHz};

    QList<bool*> ignoreEmptyDistanceFlags = {&ignoreEmptyDistanceFlag_50MHz, &ignoreEmptyDistanceFlag_70MHz, &ignoreEmptyDistanceFlag_144MHz,
                                             &ignoreEmptyDistanceFlag_432MHz, &ignoreEmptyDistanceFlag_1296MHz, &ignoreEmptyDistanceFlag_2300MHz,
                                             &ignoreEmptyDistanceFlag_3_4GHz, &ignoreEmptyDistanceFlag_5_6GHz, &ignoreEmptyDistanceFlag_10GHz};


    bool bandFilter50Mhz;
    bool bandFilter70Mhz;
    bool bandFilter144Mhz;
    bool bandFilter432Mhz;
    bool bandFilter1296Mhz;
    bool bandFilter2300Mhz;
    bool bandFilter3_4Ghz;
    bool bandFilter5_6Ghz;
    bool bandFilter10Ghz;

    bool modeFilterNONE;
    bool modeFilterCW;
    bool modeFilterUSBMODE;
    bool modeFilterFMMODE;
    bool modeFilterRTTYMODE;
    bool modeFilterPSK31MODE;
    bool modeFilterFT8MODE;
    bool modeFilterMSK144MODE;
    bool modeFilterJT65MODE;

    int distanceFilter50MHz;
    int distanceFilter70MHz;
    int distanceFilter144MHz;
    int distanceFilter432MHz;
    int distanceFilter1296MHz;
    int distanceFilter2300MHz;
    int distanceFilter3_4GHz;
    int distanceFilter5_6GHz;
    int distanceFilter10GHz;

    bool ignoreDistanceFlag_50MHz;
    bool ignoreDistanceFlag_70MHz;
    bool ignoreDistanceFlag_144MHz;
    bool ignoreDistanceFlag_432MHz;
    bool ignoreDistanceFlag_1296MHz;
    bool ignoreDistanceFlag_2300MHz;
    bool ignoreDistanceFlag_3_4GHz;
    bool ignoreDistanceFlag_5_6GHz;
    bool ignoreDistanceFlag_10GHz;

    bool ignoreEmptyDistanceFlag_50MHz;
    bool ignoreEmptyDistanceFlag_70MHz;
    bool ignoreEmptyDistanceFlag_144MHz;
    bool ignoreEmptyDistanceFlag_432MHz;
    bool ignoreEmptyDistanceFlag_1296MHz;
    bool ignoreEmptyDistanceFlag_2300MHz;
    bool ignoreEmptyDistanceFlag_3_4GHz;
    bool ignoreEmptyDistanceFlag_5_6GHz;
    bool ignoreEmptyDistanceFlag_10GHz;




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

void setModeFilter(bool setting, int mode)
{
    *modeFilters[mode] = setting;
}

bool testDistanceFilter(int distance, int band)
{

    if (distance < *distanceFilters[band] || *distanceFilters[band] == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void setDistanceFilter(int distance, int band)
{
    *distanceFilters[band] = distance;
}


bool getIgnoreDistanceFlag(int band)
{
    return *ignoreDistanceFlags[band];
}

void setIgnoreDistanceFlag(bool state, int band)
{
    *ignoreDistanceFlags[band] = state;
}


bool getIgnoreEmptyDistanceFlag(int band)
{
    return *ignoreEmptyDistanceFlags[band];
}

void setIgnoreEmptyDistanceFlag(bool state, int band)
{
    *ignoreEmptyDistanceFlags[band] = state;
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
    modeFilterNONE = ccfs.modeFilterNONE;
    modeFilterCW = ccfs.modeFilterCW;
    modeFilterUSBMODE = ccfs.modeFilterUSBMODE;
    modeFilterFMMODE = ccfs.modeFilterFMMODE;
    modeFilterRTTYMODE = ccfs.modeFilterRTTYMODE;
    modeFilterPSK31MODE = ccfs.modeFilterPSK31MODE;
    modeFilterFT8MODE = ccfs.modeFilterFT8MODE;
    modeFilterMSK144MODE = ccfs.modeFilterMSK144MODE;
    modeFilterJT65MODE = ccfs.modeFilterJT65MODE;
    distanceFilter50MHz = ccfs.distanceFilter50MHz;
    distanceFilter70MHz = ccfs.distanceFilter70MHz;
    distanceFilter144MHz = ccfs.distanceFilter144MHz;
    distanceFilter432MHz = ccfs.distanceFilter432MHz;
    distanceFilter1296MHz = ccfs.distanceFilter1296MHz;
    distanceFilter2300MHz = ccfs.distanceFilter2300MHz;
    distanceFilter3_4GHz = ccfs.distanceFilter3_4GHz;
    distanceFilter5_6GHz = ccfs.distanceFilter5_6GHz;
    distanceFilter10GHz = ccfs.distanceFilter10GHz;
    ignoreDistanceFlag_50MHz = ccfs.ignoreDistanceFlag_50MHz;
    ignoreDistanceFlag_70MHz = ccfs.ignoreDistanceFlag_70MHz;
    ignoreDistanceFlag_144MHz = ccfs.ignoreDistanceFlag_144MHz;
    ignoreDistanceFlag_432MHz = ccfs.ignoreDistanceFlag_432MHz;
    ignoreDistanceFlag_1296MHz = ccfs.ignoreDistanceFlag_1296MHz;
    ignoreDistanceFlag_2300MHz = ccfs.ignoreDistanceFlag_2300MHz;
    ignoreDistanceFlag_3_4GHz = ccfs.ignoreDistanceFlag_3_4GHz;
    ignoreDistanceFlag_5_6GHz = ccfs.ignoreDistanceFlag_5_6GHz;
    ignoreDistanceFlag_10GHz = ccfs.ignoreDistanceFlag_10GHz;
    ignoreEmptyDistanceFlag_50MHz = ccfs.ignoreEmptyDistanceFlag_50MHz;
    ignoreEmptyDistanceFlag_70MHz = ccfs.ignoreEmptyDistanceFlag_70MHz;
    ignoreEmptyDistanceFlag_144MHz = ccfs.ignoreEmptyDistanceFlag_144MHz;
    ignoreEmptyDistanceFlag_432MHz = ccfs.ignoreEmptyDistanceFlag_432MHz;
    ignoreEmptyDistanceFlag_1296MHz = ccfs.ignoreEmptyDistanceFlag_1296MHz;
    ignoreEmptyDistanceFlag_2300MHz = ccfs.ignoreEmptyDistanceFlag_2300MHz;
    ignoreEmptyDistanceFlag_3_4GHz = ccfs.ignoreEmptyDistanceFlag_3_4GHz;
    ignoreEmptyDistanceFlag_5_6GHz = ccfs.ignoreEmptyDistanceFlag_5_6GHz;
    ignoreEmptyDistanceFlag_10GHz = ccfs.ignoreEmptyDistanceFlag_10GHz;


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
         modeFilterNONE == ccfs.modeFilterNONE &&
         modeFilterCW == ccfs.modeFilterCW &&
         modeFilterUSBMODE == ccfs.modeFilterUSBMODE &&
         modeFilterFMMODE == ccfs.modeFilterFMMODE &&
         modeFilterRTTYMODE == ccfs.modeFilterRTTYMODE &&
         modeFilterPSK31MODE == ccfs.modeFilterPSK31MODE &&
         modeFilterFT8MODE == ccfs.modeFilterFT8MODE &&
         modeFilterMSK144MODE == ccfs.modeFilterMSK144MODE &&
         modeFilterJT65MODE == ccfs.modeFilterJT65MODE &&
         distanceFilter50MHz == ccfs.distanceFilter50MHz &&
         distanceFilter70MHz == ccfs.distanceFilter70MHz &&
         distanceFilter144MHz == ccfs.distanceFilter144MHz &&
         distanceFilter432MHz == ccfs.distanceFilter432MHz &&
         distanceFilter1296MHz == ccfs.distanceFilter1296MHz &&
         distanceFilter2300MHz == ccfs.distanceFilter2300MHz &&
         distanceFilter3_4GHz == ccfs.distanceFilter3_4GHz &&
         distanceFilter5_6GHz == ccfs.distanceFilter5_6GHz &&
         distanceFilter10GHz == ccfs.distanceFilter10GHz &&
         ignoreDistanceFlag_50MHz == ccfs.ignoreDistanceFlag_50MHz &&
         ignoreDistanceFlag_70MHz == ccfs.ignoreDistanceFlag_70MHz &&
         ignoreDistanceFlag_144MHz == ccfs.ignoreDistanceFlag_144MHz &&
         ignoreDistanceFlag_432MHz == ccfs.ignoreDistanceFlag_432MHz &&
         ignoreDistanceFlag_1296MHz == ccfs.ignoreDistanceFlag_1296MHz &&
         ignoreDistanceFlag_2300MHz == ccfs.ignoreDistanceFlag_2300MHz &&
         ignoreDistanceFlag_3_4GHz == ccfs.ignoreDistanceFlag_3_4GHz &&
         ignoreDistanceFlag_5_6GHz == ccfs.ignoreDistanceFlag_5_6GHz &&
         ignoreDistanceFlag_10GHz == ccfs.ignoreDistanceFlag_10GHz &&
         ignoreEmptyDistanceFlag_50MHz == ccfs.ignoreEmptyDistanceFlag_50MHz &&
         ignoreEmptyDistanceFlag_70MHz == ccfs.ignoreEmptyDistanceFlag_70MHz &&
         ignoreEmptyDistanceFlag_144MHz == ccfs.ignoreEmptyDistanceFlag_144MHz &&
         ignoreEmptyDistanceFlag_432MHz == ccfs.ignoreEmptyDistanceFlag_432MHz &&
         ignoreEmptyDistanceFlag_1296MHz == ccfs.ignoreEmptyDistanceFlag_1296MHz &&
         ignoreEmptyDistanceFlag_2300MHz == ccfs.ignoreEmptyDistanceFlag_2300MHz &&
         ignoreEmptyDistanceFlag_3_4GHz == ccfs.ignoreEmptyDistanceFlag_3_4GHz &&
         ignoreEmptyDistanceFlag_5_6GHz == ccfs.ignoreEmptyDistanceFlag_5_6GHz &&
         ignoreEmptyDistanceFlag_10GHz == ccfs.ignoreEmptyDistanceFlag_10GHz)
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



class BandmapClientFilterSettings
{

public:
    BandmapClientFilterSettings() :

        modeFilterNONE(false),
        modeFilterCW(false),
        modeFilterUSBMODE(false),
        modeFilterFMMODE(false),
        modeFilterRTTYMODE(false),
        modeFilterPSK31MODE(false),
        modeFilterFT8MODE(false),
        modeFilterMSK144MODE(false),
        modeFilterJT65MODE(false),
        distanceFilter(0),
        ignoreDistanceFlag(false),
        ignoreEmptyDistanceFlag(false)

    {


    }




    QList<bool*> modeFilters = { &modeFilterNONE, &modeFilterCW, &modeFilterUSBMODE, &modeFilterFMMODE,
                                 &modeFilterRTTYMODE, &modeFilterPSK31MODE, &modeFilterFT8MODE,
                                &modeFilterMSK144MODE, &modeFilterJT65MODE};



    bool modeFilterNONE;
    bool modeFilterCW;
    bool modeFilterUSBMODE;
    bool modeFilterFMMODE;
    bool modeFilterRTTYMODE;
    bool modeFilterPSK31MODE;
    bool modeFilterFT8MODE;
    bool modeFilterMSK144MODE;
    bool modeFilterJT65MODE;

    int distanceFilter;

    bool ignoreDistanceFlag;
    bool ignoreEmptyDistanceFlag;





void setAllModeFilters(QList<bool> mfl)
{
    for (int i = 0; i < mfl.count(); i++)
    {
        *modeFilters[i] = mfl[i];

    }
}


bool testModeFilter(int mode)
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

void setModeFilter(bool setting, int mode)
{
    *modeFilters[mode] = setting;
}

bool testDistanceFilter(int distance)
{
    if (distance < distanceFilter || distanceFilter == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void setDistanceFilter(int distance)
{
    distanceFilter = distance;
}

bool getIgnoreDistanceFlag()
{
    return ignoreDistanceFlag;
}

void setIgnoreDistanceFlag(bool state)
{
    ignoreDistanceFlag = state;
}


bool getIgnoreEmptyDistanceFlag()
{
    return ignoreEmptyDistanceFlag;
}

void setIgnoreEmptyDistanceFlag(bool state)
{
    ignoreDistanceFlag = state;
}

void operator= (const BandmapClientFilterSettings& bcfs)
{

    modeFilterNONE = bcfs.modeFilterNONE;
    modeFilterCW = bcfs.modeFilterCW;
    modeFilterUSBMODE = bcfs.modeFilterUSBMODE;
    modeFilterFMMODE = bcfs.modeFilterFMMODE;
    modeFilterRTTYMODE = bcfs.modeFilterRTTYMODE;
    modeFilterPSK31MODE = bcfs.modeFilterPSK31MODE;
    modeFilterFT8MODE = bcfs.modeFilterFT8MODE;
    modeFilterMSK144MODE = bcfs.modeFilterMSK144MODE;
    modeFilterJT65MODE = bcfs.modeFilterJT65MODE;
    distanceFilter = bcfs.distanceFilter;
    ignoreDistanceFlag = bcfs.ignoreDistanceFlag;
    ignoreEmptyDistanceFlag = bcfs.ignoreEmptyDistanceFlag;


}


bool operator==( const BandmapClientFilterSettings& bcfs ) const
{
    if ( modeFilterNONE == bcfs.modeFilterNONE &&
         modeFilterCW == bcfs.modeFilterCW &&
         modeFilterUSBMODE == bcfs.modeFilterUSBMODE &&
         modeFilterFMMODE == bcfs.modeFilterFMMODE &&
         modeFilterRTTYMODE == bcfs.modeFilterRTTYMODE &&
         modeFilterPSK31MODE == bcfs.modeFilterPSK31MODE &&
         modeFilterFT8MODE == bcfs.modeFilterFT8MODE &&
         modeFilterMSK144MODE == bcfs.modeFilterMSK144MODE &&
         modeFilterJT65MODE == bcfs.modeFilterJT65MODE &&
         distanceFilter == bcfs.distanceFilter &&
         ignoreDistanceFlag == bcfs.ignoreDistanceFlag &&
         ignoreEmptyDistanceFlag == bcfs.ignoreEmptyDistanceFlag)

    {
        return true;
    }

    return false;

}


bool operator!=( const BandmapClientFilterSettings& ccfs ) const
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

const int DEFAULT_FILTER_DISTANCE = 500;
const int MIN_FILTER_DISTANCE = 100;
const int MAX_FILTER_DISTANCE = 50000;
const int ADD_TUNING_BANDMAP_FREQ_DEFAULT_TOLERANCE = 5;   // khz
const int ADD_TUNING_BANDMAP_FREQ_DEFAULT_MIN_TOLERANCE = 1;   // khz
const int ADD_TUNING_BANDMAP_FREQ_DEFAULT_MAX_TOLERANCE = 10;   // khz


class ClusterFilterIdAndNames
{
public:
    LOGGERPROFILE getDefaultFilterId(int i){return defaultFilterId[i];}
    QString getDefaultFilterName(int i){return defaultFilterName[i];}
    int getDefaultFilterNameCount(){return defaultFilterName.count();}


private:

    const LOGGERPROFILE defaultFilterId [9] {elpDefaultFilterDistance_50MHz,
                                elpDefaultFilterDistance_70MHz,
                                elpDefaultFilterDistance_144MHz,
                                elpDefaultFilterDistance_432MHz,
                                elpDefaultFilterDistance_1296MHz,
                                elpDefaultFilterDistance_2300MHz,
                                elpDefaultFilterDistance_3_4GHz,
                                elpDefaultFilterDistance_5_6GHz,
                                elpDefaultFilterDistance_10GHz};

    const QStringList defaultFilterName  {"defaultFilterDistance_50MHz",
                                "defaultFilterDistance_70MHz",
                                "defaultFilterDistance_144MHz",
                                "defaultFilterDistance_432MHz",
                                "defaultFilterDistance_1296MHz",
                                "defaultFilterDistance_2300MHz",
                                "defaultFilterDistance_3_4GHz",
                                "defaultFilterDistance_5_6GHz",
                                "defaultFilterDistance_10GHz"};


};



#endif // CLUSTERCOMMON_H
