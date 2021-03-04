#ifndef CLUSTERCOMMON_H
#define CLUSTERCOMMON_H


#include "base_pch.h"
#include "BandList.h"
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



enum allBandOffsets {_1_8M, _3_5M, _7M, _14M, _21M, _28M, _50M, _70M, _144M, _432M, _1296M, _2300M, _3_4G, _5_6G, _10G};
const QStringList clusterBands = QStringList() << "1.8 MHz" << "3.5 MHz" << "7 MHz" << "14 MHz" << "21 MHz" << "28 Mhz" << "50 MHz" << "70 MHz" << "144 MHz" << "432 MHz" << "1296 MHz" << "2300 MHz" << "3.4 GHz" << "5.6 GHz" << "10 GHz";


//enum allModeOffsets {NO_MODE, CW_MODE, USB_MODE, FM_MODE, RTTY_MODE, PSK31_MODE, FT8_MODE, MSK144_MODE, JT65_MODE};
const QString NONE_MODE = "NONE";
const QString CW_MODE = "CW";
const QString USB_MODE = "USB";
const QString LSB_MODE = "LSB";
const QString FM_MODE = "FM";
const QString RTTY_MODE = "RTTY";
const QString PSK31_MODE = "PSK31";
const QString FT8_MODE = "FT8";
const QString FT4_MODE = "FT4";
const QString MSK144_MODE = "MSK144";
const QString JT65_MODE = "JT65";


const QStringList clusterModes = QStringList() << NONE_MODE << CW_MODE << USB_MODE << FM_MODE << RTTY_MODE << PSK31_MODE << FT8_MODE << MSK144_MODE << JT65_MODE;
const QStringList mgmModes = QStringList() << RTTY_MODE << PSK31_MODE << FT8_MODE << MSK144_MODE << JT65_MODE;

const QStringList clusterPropModes = QStringList() << "TR" << "ES" << "MS" << "EME";
enum bandPlanModeError {MODE_FREQ_MATCH, NO_MODE_FREQ_MATCH, MODE_NOT_FOUND, BAND_NOT_FOUND};

enum clusterErrorCode {SPOT_OK, NO_SPOT_TIME, SPOT_DATETIME_INVALID, SPOT_TOO_MANY_SECTIONS, DISCARD_HF_SPOT, DISCARD_SPOT_NOT_CONTEST_BAND, GET_PREFIX_FAILED, ASKQRZ_FAILED_QRA};
const QStringList clusterErrorMsg = QStringList() << "Spot OK" << "SpotTime not found" << "Spot DateTime Invalid" << "Spot too many sections"
                                                  << "Discard HF Spot" << "getPrefix failed to find QRA" << "AskQrz Failed to Find QRA";

// OffSet to items in spot message - Note! add 1 for raw message as that has "DXSPOT" as header
const int DX_CLUSTER_SPOT_TYPE = 0;
const int DXCALL = 1;
const int DXLOCATOR = 2;
const int DXLOC_FROM_NODE_FLAG = 3;
const int DXFREQ = 4;
const int DXBANDSTR = 5;
const int DXBANDTYPE = 6;
const int DXMODESTR = 7;
const int SPOTCALL = 8;
const int SPOTLOCATOR = 9;
const int SPOTDATETIME = 10;
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
const int DXLOC_FROM_NODE_FLAG_COL_NUM = 6;
const int DXDIST_COL_NUM = 7;
const int DXBRG_COL_NUM = 8;
const int DXLOC_WORKED_COL_NUM = 9;
const int SPOTTER_CALL_COL_NUM = 10;
const int SPOTTER_LOC_COL_NUM = 11;
const int COMMENT_COL_NUM = 12;
const int DXBANDSTR_COL_NUM = 13;
const int DXSPOT_TO_MEMORY_FLAG_COL_NUM = 14;
const int DXSPOT_PROP_MODE_COL_NUM = 15;
const int RXTIME_COL_NUM = 16;
const int DATE_COL_NUM = 17;
const int DATE_TIME_COL_NUM = 18;
const int DXCLUSTER_SPOT_TYPE = 19;
const int SPOT_TYPE_COL_NUM = 20;       // used in bandmap
const int SPOT_IS_SELECTED_COL_NUM = 21;    // used in bandmap
const int ROT_BEARING_COL_NUM = 22;     // used in bandmap
const int ROT_CONNECTED_COL_NUM = 23;   // used in bandmap
const int RUN_MODE_ON_COL_NUM = 24;     // used in bandmap
const int OFF_RUN_FREQ_COL_NUM = 25;    // used in bandmap
const int CQ_RESPONSE_COL = 26;    // used in bandmap
const int DX_DISTRICT_COL_NUM = 27;   // used in bandmap
const int DX_DISTRICT_WORKED_COL_NUM = 28; // used in bandmap

const bool BOOL_YES = true;
const bool BOOL_NO = false;

const QString SPOT_TX_ON = "TxSpotOn";
const QString SPOT_TX_OFF = "TxSpotOff";

const QChar SPOT_DATA_SEPERATOR = ':';
// Only locators in Europe
// use [I-Li-l][P-Lp-l] here??
const QRegularExpression FULL_LOC_EXP = QRegularExpression("[I,i,J,j,K,k,L,l][P,p,O,o,N,n,M,m,L,l]\\d\\d[A-Za-z][A-Za-z]");
const QRegularExpression PART_LOC_EXP = QRegularExpression("[I,i,J,j,K,k,L,l][P,p,O,o,N,n,M,m,L,l]\\d\\d\\W");
const QRegularExpression SEARCH_LOC_EXP = QRegularExpression("[I,i,J,j,K,k,L,l][P,p,O,o,N,n,M,m,L,l]\\d\\d");

const QRegularExpression FULL_LOC_EXP_HF = QRegularExpression("[A-Ra-r][A-Ra-r]\\d\\d[A-Za-z][A-Za-z]");
const QRegularExpression PART_LOC_EXP_HF = QRegularExpression("[A-Ra-r][A-Ra-r]\\d\\d\\W");
const QRegularExpression SEARCH_LOC_EXP_HF = QRegularExpression("[A-Ra-r][A-Ra-r]\\d\\d");

const int START_HF = 0; // index bandlist table
const int END_HF = 5;

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
const QString IGNORE_BANDMASK = "ignore_bandmask";


QDateTime getSpotDateTime(const QString spotDate, const QString spotTime);

bool spotTimedOut(qlonglong spotTime, qlonglong timeToLive);

qlonglong spotElapsedTime(qlonglong spotTime);

void getMode(checkModeAgainstFreq* modeBandPlan, Frequency freq, const QString &dxBand, QString &dxModeStr, QString &dxModeMask);

bool getBand(QVector<QSharedPointer<BandInfo> > &bands, Frequency fr, QString &band, QString &bandType);

bool extractDxLocFromNodeFlag(QString locFlagMsg);

class ClusterServer
{
public:
    QString routerName;
    QString app;
    PublishState state;
};



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


class BandFilterSettings
{
   public:

    BandFilterSettings()
    {
        bandFilterFlag = false;
        distanceFilter = 0;
        ignoreDistanceFlag = false;
        ignoreEmptyDistanceFlag = false;
        bandType = "";
    }


    BandFilterSettings& operator=(const BandFilterSettings &bfs)
    {
        bandFilterFlag = bfs.bandFilterFlag;
        distanceFilter = bfs.distanceFilter;
        ignoreDistanceFlag = bfs.ignoreDistanceFlag;
        ignoreEmptyDistanceFlag = bfs.ignoreEmptyDistanceFlag;
        bandType = bfs.bandType;

        return *this;
    }


    bool operator==( const BandFilterSettings& bfs ) const
    {
        if (bandFilterFlag == bfs.bandFilterFlag &&
                distanceFilter == bfs.distanceFilter &&
                ignoreDistanceFlag == bfs.ignoreDistanceFlag &&
                ignoreEmptyDistanceFlag == bfs.ignoreEmptyDistanceFlag &&
                bandType != bfs.bandType)
        {
            return true;
        }

        return false;
    }

    bool operator!=( const BandFilterSettings& bfs )
    {
        return !(*this == bfs);
    }


    bool bandFilterFlag;
    int distanceFilter;
    bool ignoreDistanceFlag;
    bool ignoreEmptyDistanceFlag;
    QString bandType;
};


class ModeFilterSettings
{

public:
    ModeFilterSettings();



    bool operator==(const ModeFilterSettings& mfs) const;


    bool testModeFilter(QString mode);
    bool getModeFilter(QString mode);
    void setModeFilter(QString mode, bool setting);


    bool contains(const QString mode);
private:

    QMap<QString, bool> modeFilterFlag;   // QMap<mode, flag>

};



class ClusterClientBandFilterDialogDetails
{

public:
    ClusterClientBandFilterDialogDetails()
    {
        bandChkBox = nullptr;
    }
    QCheckBox *bandChkBox;
    QString bandType;


};


class ClusterClientDistanceFilterDetails
{
public:

    ClusterClientDistanceFilterDetails()
    {
        bandLineEdit = nullptr;
        bandLabel = nullptr;
        distFilterIgnoreCheckBox = nullptr;
        distFilterIgnoreEmptyCheckBox = nullptr;

    }

    QLineEdit *bandLineEdit;
    QLabel *bandLabel;
    QCheckBox *distFilterIgnoreCheckBox;
    QCheckBox *distFilterIgnoreEmptyCheckBox;
    QString bandType;


};


class ClusterClientFilterSettings
{

public:
    ClusterClientFilterSettings();

    void initFilterSettings(const QVector<QSharedPointer<BandInfo> > &bands);

    ClusterClientFilterSettings (const ClusterClientFilterSettings& ccfs);
    ClusterClientFilterSettings & operator= (const ClusterClientFilterSettings &ccfs);
    bool operator==( const ClusterClientFilterSettings& ccfs ) const;
    bool operator!=( const ClusterClientFilterSettings& ccfs );

    void setCallSignFilterList(QString cfl);
    QString getCallSignFilterList();

    bool getBandFilter(QString band) const;
    void setBandFilter(QString band, bool setting);


    void setBandType(QString band, QString bandType);
    QString getBandType(QString band);

    bool getModeFilter(QString mode);
    void setModeFilter(QString mode, bool setting);



    bool testDistance(int distance, QString band, bool lessGreaterFlag);


    int getDistanceFilter(QString band);
    void setDistanceFilter(QString band, int distance);

    bool getIgnoreDistanceFlag(QString band);
    void setIgnoreDistanceFlag(QString band, bool state);

    bool getIgnoreEmptyDistanceFlag(QString band);
    void setIgnoreEmptyDistanceFlag(QString band, bool state);

    QString packFilterList(QStringList l);
    QStringList unpackFilterList(QString &sl);


    // note the list of callsign and locator filters strings are stored as QString for saving to contest.
    QString callsignFilterList;
    QString locatorFilterList;



private:


    QMap<QString, BandFilterSettings> bandFilterSettings;  // QMap<band, filterSettings>
    ModeFilterSettings modeFilterFlag;   // QMap<mode, flag>




};



class BandmapClientFilterSettings
{

public:
    BandmapClientFilterSettings();


    BandmapClientFilterSettings (const BandmapClientFilterSettings& bcfs);

    BandmapClientFilterSettings &operator= (const BandmapClientFilterSettings& bcfs);
    bool operator==( const BandmapClientFilterSettings& bcfs ) const;
    bool operator!=( const BandmapClientFilterSettings& ccfs ) const;

    void setFilterModeSettings(QStringList &clustermodes);



    void setModeFilter(QString mode, bool setting);
    bool getModeFilter(QString mode);

    bool testDistance(int distance, bool lessGreaterFlag);
    int getDistanceFilter();
    void setDistanceFilter(int distance);


    bool getIgnoreDistanceFlag();
    void setIgnoreDistanceFlag(bool state);

    bool getIgnoreEmptyDistanceFlag();
    void setIgnoreEmptyDistanceFlag(bool state);

    QString packFilterList(QStringList l);
    QStringList unpackFilterList(QString &sl);

private:


    ModeFilterSettings modeFilterFlag;   // QMap<mode, flag>

    int distanceFilter;

    bool ignoreDistanceFlag;
    bool ignoreEmptyDistanceFlag;

};

const int DEFAULT_FILTER_DISTANCE = 500;
const int MIN_FILTER_DISTANCE = 100;
const int MAX_FILTER_DISTANCE = 50000;
const int ADD_TUNING_BANDMAP_FREQ_DEFAULT_TOLERANCE = 5;   // khz
const int ADD_TUNING_BANDMAP_FREQ_DEFAULT_MIN_TOLERANCE = 1;   // khz
const int ADD_TUNING_BANDMAP_FREQ_DEFAULT_MAX_TOLERANCE = 10;   // khz

const QString LESS_GREATER_THAN_DISTANCE_FLAG_INI_NAME = "lessGreaterThanDistanceFlag";

class DefaultDistanceIniName
{

public:
    DefaultDistanceIniName()
    {

    };

    QString defaultDistanceName;
    QString bandType;
};


class ClusterFilterDefaultDistIniName
{
public:

    ClusterFilterDefaultDistIniName()
    {

    };

    DefaultDistanceIniName getDefaultDistIniName(const QString band){return defaultDistanceIniNames.value(band);}

    void initClusterFilterIdAndNames(const QVector<QSharedPointer<BandInfo> > &bands)
    {
        DefaultDistanceIniName ddin;
        QString defTxt = "defaultFilterDistance_";
        for (auto &b:bands)
        {

            QString band = b.data()->uk;
            QString iniBand = band.remove("\x20").replace(".", "_");
            ddin.defaultDistanceName = defTxt.append(iniBand);
            ddin.bandType = b.data()->getType();
            defaultDistanceIniNames.insert(band, ddin);
        }
    }

private:


    QMap <QString, DefaultDistanceIniName> defaultDistanceIniNames;




};


class LegacyClusterFilterDistanceId
{
public:
    LegacyClusterFilterDistanceId()
    {
        clusterLegacyFilterDistanceId.insert("1.8 MHz", elpDefaultFilterDistance_1_8MHz);
        clusterLegacyFilterDistanceId.insert("3.5 MHz", elpDefaultFilterDistance_3_5MHz);
        clusterLegacyFilterDistanceId.insert("7 MHz", elpDefaultFilterDistance_7MHz);
        clusterLegacyFilterDistanceId.insert("14 MHz", elpDefaultFilterDistance_14MHz);
        clusterLegacyFilterDistanceId.insert("21 MHz", elpDefaultFilterDistance_21MHz);
        clusterLegacyFilterDistanceId.insert("28 MHz", elpDefaultFilterDistance_28MHz);
        clusterLegacyFilterDistanceId.insert("50 MHz", elpDefaultFilterDistance_50MHz);
        clusterLegacyFilterDistanceId.insert("70 MHz", elpDefaultFilterDistance_70MHz);
        clusterLegacyFilterDistanceId.insert("144 MHz", elpDefaultFilterDistance_144MHz);
        clusterLegacyFilterDistanceId.insert("432 MHz", elpDefaultFilterDistance_432MHz);
        clusterLegacyFilterDistanceId.insert("1296 MHz", elpDefaultFilterDistance_1296MHz);
        clusterLegacyFilterDistanceId.insert("2300 MHz", elpDefaultFilterDistance_2300MHz);
        clusterLegacyFilterDistanceId.insert("3.4 GHz", elpDefaultFilterDistance_3_4GHz);
        clusterLegacyFilterDistanceId.insert("5.6 GHz", elpDefaultFilterDistance_5_6GHz);
        clusterLegacyFilterDistanceId.insert("10 GHz", elpDefaultFilterDistance_10GHz);
    }

    QMap<QString, LOGGERPROFILE> clusterLegacyFilterDistanceId;
};

/*
class ClusterFilterIdAndNames
{
public:
    LOGGERPROFILE getAllDefaultFilterId(int i){return allDefaultFilterId[i];}
    LOGGERPROFILE getHfDefaultFilterId(int i){return hfDefaultFilterId[i];}
    LOGGERPROFILE getVhfDefaultFilterId(int i){return vhfDefaultFilterId[i];}
    LOGGERPROFILE getMwDefaultFilterId(int i){return mwDefaultFilterId[i];}

    QString getAllDefaultFilterName(int i){return allDefaultFilterName[i];}
    int getAllDefaultFilterNameCount(){return allDefaultFilterName.count();}

    QString getHfDefaultFilterName(int i){return hfDefaultFilterName[i];}
    int getHfDefaultFilterNameCount(){return hfDefaultFilterName.count();}

    QString getVhfDefaultFilterName(int i){return vhfDefaultFilterName[i];}
    int getVhfDefaultFilterNameCount(){return vhfDefaultFilterName.count();}

    QString getMwDefaultFilterName(int i){return mwDefaultFilterName[i];}
    int getMwDefaultFilterNameCount(){return mwDefaultFilterName.count();}



private:


    const LOGGERPROFILE allDefaultFilterId [15] {
                                elpDefaultFilterDistance_1_8MHz,
                                elpDefaultFilterDistance_3_5MHz,
                                elpDefaultFilterDistance_7MHz,
                                elpDefaultFilterDistance_14MHz,
                                elpDefaultFilterDistance_21MHz,
                                elpDefaultFilterDistance_28MHz,
                                elpDefaultFilterDistance_50MHz,
                                elpDefaultFilterDistance_70MHz,
                                elpDefaultFilterDistance_144MHz,
                                elpDefaultFilterDistance_432MHz,
                                elpDefaultFilterDistance_1296MHz,
                                elpDefaultFilterDistance_2300MHz,
                                elpDefaultFilterDistance_3_4GHz,
                                elpDefaultFilterDistance_5_6GHz,
                                elpDefaultFilterDistance_10GHz};

    const LOGGERPROFILE hfDefaultFilterId [6] {
                                elpDefaultFilterDistance_1_8MHz,
                                elpDefaultFilterDistance_3_5MHz,
                                elpDefaultFilterDistance_7MHz,
                                elpDefaultFilterDistance_14MHz,
                                elpDefaultFilterDistance_21MHz,
                                elpDefaultFilterDistance_28MHz};

    const LOGGERPROFILE vhfDefaultFilterId [4] {
                                elpDefaultFilterDistance_50MHz,
                                elpDefaultFilterDistance_70MHz,
                                elpDefaultFilterDistance_144MHz,
                                elpDefaultFilterDistance_432MHz};

    const LOGGERPROFILE mwDefaultFilterId [5] {
                                elpDefaultFilterDistance_1296MHz,
                                elpDefaultFilterDistance_2300MHz,
                                elpDefaultFilterDistance_3_4GHz,
                                elpDefaultFilterDistance_5_6GHz,
                                elpDefaultFilterDistance_10GHz};




    const QStringList allDefaultFilterName  {
                                "defaultFilterDistance_1_8MHz",
                                "defaultFilterDistance_3_5MHz",
                                "defaultFilterDistance_7MHz",
                                "defaultFilterDistance_14MHz",
                                "defaultFilterDistance_21MHz",
                                "defaultFilterDistance_28MHz",
                                "defaultFilterDistance_50MHz",
                                "defaultFilterDistance_70MHz",
                                "defaultFilterDistance_144MHz",
                                "defaultFilterDistance_432MHz",
                                "defaultFilterDistance_1296MHz",
                                "defaultFilterDistance_2300MHz",
                                "defaultFilterDistance_3_4GHz",
                                "defaultFilterDistance_5_6GHz",
                                "defaultFilterDistance_10GHz"};

    const QStringList hfDefaultFilterName  {
                                "defaultFilterDistance_1_8MHz",
                                "defaultFilterDistance_3_5MHz",
                                "defaultFilterDistance_7MHz",
                                "defaultFilterDistance_14MHz",
                                "defaultFilterDistance_21MHz",
                                "defaultFilterDistance_28MHz"};

    const QStringList vhfDefaultFilterName  {
                                "defaultFilterDistance_50MHz",
                                "defaultFilterDistance_70MHz",
                                "defaultFilterDistance_144MHz",
                                "defaultFilterDistance_432MHz"};

    const QStringList mwDefaultFilterName  {
                                "defaultFilterDistance_1296MHz",
                                "defaultFilterDistance_2300MHz",
                                "defaultFilterDistance_3_4GHz",
                                "defaultFilterDistance_5_6GHz",
                                "defaultFilterDistance_10GHz"};


};

*/

#endif // CLUSTERCOMMON_H
