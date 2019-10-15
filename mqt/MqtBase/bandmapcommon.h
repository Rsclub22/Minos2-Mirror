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



const int BMP_TIME_COL_NUM = 0;
const int BMP_FREQ_COL_NUM = 1;
const int BMP_DXSPOT_CALL_COL_NUM = 2;
const int BMP_DXSPOT_CALL_WORKED_COL_NUM = 3;
const int BMP_DXLOC_COL_NUM = 4;
const int BMP_DXDIST_COL_NUM = 5;
const int BMP_DXBRG_COL_NUM = 6;
const int BMP_DXLOC_WORKED_COL_NUM = 7;
const int BMP_SPOT_CALL_COL_NUM = 8;
const int BMP_SPOTLOC_COL_NUM = 9;
const int BMP_COMMENT_COL_NUM = 10;
const int BMP_DXBANDMASK_COL_NUM = 11;
const int BMP_MODEMASK_COL_NUM = 12;
const int BMP_DXSPOT_TO_MEMORY_FLAG_COL_NUM = 13;
const int BMP_RXTIME_COL_NUM = 14;

const int BMP_TIME_COL_WIDTH = 40;
const int BMP_FREQ_COL_WIDTH = 60;
const int BMP_DXSPOT_CALL_COL_WIDTH = 60;
const int BMP_DXSPOT_CALL_WKD_COL_WIDTH = 30;
const int BMP_DXLOC_COL_WIDTH = 50;
const int BMP_DXDIST_COL_WIDTH = 40;
const int BMP_DXBRG_COL_WIDTH = 30;
const int BMP_DXLOC_WKD_COL_WIDTH = 30;
const int BMP_SPOT_CALL_COL_WIDTH = 60;
const int BMP_SPOTLOC_COL_WIDTH = 50;
const int BMP_COMMENT_COL_WIDTH = 170;

const bool BMP_BOOL_YES = true;
const bool BMP_BOOL_NO = false;

const int SPOTMARKER_XOFFSET = 20;
const int FREQ_SEL_WIDTH = 20;
const int NO_OP_FREQ_WIDTH = 5;



#endif // BANDMAPCOMMON_H
