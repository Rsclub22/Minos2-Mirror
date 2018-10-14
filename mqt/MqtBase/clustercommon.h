#ifndef CLUSTERCOMMON_H
#define CLUSTERCOMMON_H


// Band Filter Data

const unsigned int _50M = 1 << 1;
const unsigned int _70M = 1 << 2;
const unsigned int _144M = 1 << 3;
const unsigned int _432M = 1 << 4;
const unsigned int _1296M = 1 << 5;
const unsigned int _2300M = 1 << 6;
const unsigned int _3_4G = 1 << 7;
const unsigned int _5_6G = 1 << 8;
const unsigned int _10G = 1 << 9;

// Mode Filter Data

const unsigned int CWMODE = 1 << 1;
const unsigned int PHONEMODE = 1 << 2;
const unsigned int RTTYMODE = 1 << 3;
const unsigned int PSKMODE = 1 << 4;
const unsigned int MGMMODE = 1 << 5;

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
