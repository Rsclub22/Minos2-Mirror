#ifndef CLUSTERCOMMON_H
#define CLUSTERCOMMON_H

#include <QList>

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

// Spot Offset
const int DXCALL = 0;
const int DXFREQ = 1;
const int DXBANDSTR = 2;
const int DXBANDMASK = 3;
const int DXMODESTR = 4;
const int DXMODEMASK = 5;
const int SPOTCALL = 6;
const int DXLOCATOR = 7;
const int SPOTTIME = 8;
const int SPOTCOMMENT = 9;


// Cluster List Table

const int TIME_COL_NUM = 0;
const int FREQ_COL_NUM = 1;
const int DXSPOT_CALL_COL_NUM = 2;
const int LOC_COL_NUM = 3;
const int SPOT_CALL_COL_NUM = 4;
const int COMMENT_COL_NUM = 5;

const int TIME_COL_WIDTH = 40;
const int FREQ_COL_WIDTH = 80;
const int DXSPOT_CALL_COL_WIDTH = 80;
const int LOC_COL_WIDTH = 40;
const int SPOT_CALL_COL_WIDTH = 80;
const int COMMENT_COL_WIDTH = 170;

#endif // CLUSTERCOMMON_H
