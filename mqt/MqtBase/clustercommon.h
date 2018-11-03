#ifndef CLUSTERCOMMON_H
#define CLUSTERCOMMON_H

#include <QList>
#include <QColor>



// Band Filter Data

const unsigned int _50M = 1;
const unsigned int _70M = 1 << 1;
const unsigned int _144M = 1 << 2;
const unsigned int _432M = 1 << 3;
const unsigned int _1296M = 1 << 4;
const unsigned int _2300M = 1 << 5;
const unsigned int _3_4G = 1 << 6;
const unsigned int _5_6G = 1 << 7;
const unsigned int _10G = 1 << 8;

// Mode Filter Data

const unsigned int CWMODE = 1;
const unsigned int PHONEMODE = 1 << 1;
const unsigned int RTTYMODE = 1 << 2;
const unsigned int PSKMODE = 1 << 3;
const unsigned int MGMMODE = 1 << 4;


const unsigned int vhfBandMasks[] = {_50M, _70M, _144M, _432M};
const int NUM_VHFMASKS = 4;
const unsigned int mWaveBandMasks[] = {_1296M, _2300M, _3_4G, _5_6G, _10G};
const int NUM_MWAVEMASKS = 5;
const unsigned int allBandMasks[] = {_50M, _70M, _144M, _432M, _1296M, _2300M, _3_4G, _5_6G, _10G};
const int NUM_ALLBANDMASKS = NUM_VHFMASKS + NUM_MWAVEMASKS;
const unsigned int modeMasks[] = {CWMODE, PHONEMODE, RTTYMODE, PSKMODE, MGMMODE};
const int NUM_MODEMASKS = 5;

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


// Cluster List Table

const int TIME_COL_NUM = 0;
const int FREQ_COL_NUM = 1;
const int DXSPOT_CALL_COL_NUM = 2;
const int DXSPOT_CALL_WORKED_COL_NUM = 3;
const int DXLOC_COL_NUM = 4;
const int DXLOC_WORKED_COL_NUM = 5;
const int SPOT_CALL_COL_NUM = 6;
const int SPOTLOC_COL_NUM = 7;
const int COMMENT_COL_NUM = 8;

const int TIME_COL_WIDTH = 40;
const int FREQ_COL_WIDTH = 80;
const int DXSPOT_CALL_COL_WIDTH = 80;
const int DXSPOT_CALL_WKD_COL_WIDTH = 50;
const int DXLOC_COL_WIDTH = 60;
const int DXLOC_WKD_COL_WIDTH = 50;
const int SPOT_CALL_COL_WIDTH = 80;
const int SPOTLOC_COL_WIDTH = 60;
const int COMMENT_COL_WIDTH = 170;

const QString BOOL_YES = "Yes";
const QString BOOL_NO = "";

const QChar SPOT_DATA_SEPERATOR = ':';
const QRegExp FULL_LOC_EXP = QRegExp("[A-Za-z][A-Za-z]\\d\\d[A-Za-z][A-Za-z]");
const QRegExp PART_LOC_EXP = QRegExp("[A-Za-z][A-Za-z]\\d\\d");

const int MIN_TTL = 10;
const int MAX_TTL = 180;

// cluster tab text colours
const QColor CLUSTER_TAB_SELECT_COLOR = Qt::red;
const QColor CLUSTER_TAB_NOT_SELECT_COLOR = Qt::black;

// Message headers
const QString DXSPOT = "DXSPOT:";
const QString TIMETOLIVE = "TIMETOLIVE:";



const int PURGE_TIME = 1000 * 60 * 1;   // mins

#endif // CLUSTERCOMMON_H
