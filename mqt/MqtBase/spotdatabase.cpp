/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2020
//
//
//
//
/////////////////////////////////////////////////////////////////////////////


#include "base_pch.h"
#include "spotdatabase.h"



SpotdataBase::SpotdataBase()
{
    clear();
}


SpotdataBase::SpotdataBase(const QSharedPointer<SpotdataBase> sdp)
{
    rxTime = sdp->rxTime;
    spotDateTime = sdp->spotDateTime;
    band = sdp->band;
    bandType = sdp->bandType;
    mode = sdp->mode;
    dxCall = sdp->dxCall;
    dxCallValidateCode = sdp->dxCallValidateCode;
    freq = sdp->freq;
    dxLocator = sdp->dxLocator;
    spotterCall = sdp->spotterCall;
    spotterCallValidateCode = sdp->spotterCallValidateCode;
    spotterLocator = sdp->spotterLocator;
    dxPropMode = sdp->dxPropMode;
    spotComment = sdp->spotComment;
}












SpotdataBase::SpotdataBase(const SpotdataBase &sdp)
{
    rxTime = sdp.rxTime;
    spotDateTime = sdp.spotDateTime;
    band = sdp.band;
    bandType = sdp.bandType;
    mode = sdp.mode;
    dxCall = sdp.dxCall;
    dxCallValidateCode = sdp.dxCallValidateCode;
    freq = sdp.freq;
    dxLocator = sdp.dxLocator;
    spotterCall = sdp.spotterCall;
    spotterCallValidateCode = sdp.spotterCallValidateCode;
    spotterLocator = sdp.spotterLocator;
    dxPropMode = sdp.dxPropMode;
    spotComment = sdp.spotComment;
}




void SpotdataBase::clear()
{
    rxTime = 0;
    spotDateTime = QDateTime();
    band.clear();
    bandType.clear();
    mode.clear();
    freq.clear();
    dxCall = Callsign();
    dxCallValidateCode = 0;
    dxLocator.clear();
    spotterCall = Callsign();
    spotterCallValidateCode = 0;
    spotterLocator.clear();
    dxPropMode.clear();
    spotComment.clear();

}


// --------------------------------------------------------------------------------------------


ClusterSpotDataBase::ClusterSpotDataBase()
{
    clear();

}


ClusterSpotDataBase::ClusterSpotDataBase(const ClusterSpotDataBase &cpd)
{
    dxLocatorIsFromNode = cpd.dxLocatorIsFromNode;
    askQrzFailed = cpd.askQrzFailed;
    dxDist = cpd.dxDist;
    dxBrg = cpd.dxBrg;
    dxCallWorked = cpd.dxCallWorked;
    dxLocatorWorked = cpd.dxLocatorWorked;
    sentToMemory = cpd.sentToMemory;
}

ClusterSpotDataBase::ClusterSpotDataBase(const QSharedPointer<ClusterSpotDataBase> cpd)
{
    dxLocatorIsFromNode = cpd->dxLocatorIsFromNode;
    askQrzFailed = cpd->askQrzFailed;
    dxDist = cpd->dxDist;
    dxBrg = cpd->dxBrg;
    dxCallWorked = cpd->dxCallWorked;
    dxLocatorWorked = cpd->dxLocatorWorked;
    sentToMemory = cpd->sentToMemory;
}




void ClusterSpotDataBase::clear()
{

    dxLocatorIsFromNode = false;
    askQrzFailed = false;
    dxDist.clear();
    dxBrg.clear();
    dxCallWorked = false;
    dxLocatorWorked = false;
    sentToMemory = false;
}



//--------------------------------------------------------------------------------


ClusterSpotData::ClusterSpotData()
{
    clear();

}

void ClusterSpotData::clear()
{
    SpotdataBase::clear();
    ClusterSpotDataBase::clear();
}

ClusterSpotData::ClusterSpotData(const ClusterSpotData &csd) : SpotdataBase(csd), ClusterSpotDataBase(csd)
{
    dxLocatorIsFromNode = csd.dxLocatorIsFromNode;
    askQrzFailed = csd.askQrzFailed;
    dxDist = csd.dxDist;
    dxBrg = csd.dxBrg;
    dxCallWorked = csd.dxCallWorked;
    dxLocatorWorked = csd.sentToMemory;
}

ClusterSpotData::ClusterSpotData(const QSharedPointer<ClusterSpotData> csd) : SpotdataBase(csd), ClusterSpotDataBase(csd)
{
    dxLocatorIsFromNode = csd->dxLocatorIsFromNode;
    askQrzFailed = csd->askQrzFailed;
    dxDist = csd->dxDist;
    dxBrg = csd->dxBrg;
    dxCallWorked = csd->dxCallWorked;
    dxLocatorWorked = csd->sentToMemory;
}


ClusterSpotData& ClusterSpotData::operator = (const ClusterSpotData& csd)
{
    rxTime = csd.rxTime;
    spotDateTime = csd.spotDateTime;
    band = csd.band;
    bandType = csd.bandType;
    mode = csd.mode;
    dxCall = csd.dxCall;
    dxCallValidateCode = csd.dxCallValidateCode;
    freq = csd.freq;
    dxLocator = csd.dxLocator;
    spotterCall = csd.spotterCall;
    spotterCallValidateCode = csd.spotterCallValidateCode;
    dxPropMode = csd.dxPropMode;
    spotComment = csd.spotComment;
    dxLocatorIsFromNode = csd.dxLocatorIsFromNode;
    askQrzFailed = csd.askQrzFailed;
    dxDist = csd.dxDist;
    dxBrg = csd.dxBrg;
    dxCallWorked = csd.dxCallWorked;
    dxLocatorWorked = csd.sentToMemory;

    return *this;

}

QSharedPointer<ClusterSpotData> ClusterSpotData::operator = (const QSharedPointer<ClusterSpotData> csd)
{
    rxTime = csd->rxTime;
    spotDateTime = csd->spotDateTime;
    band = csd->band;
    bandType = csd->bandType;
    mode = csd->mode;
    dxCall = csd->dxCall;
    dxCallValidateCode = csd->dxCallValidateCode;
    freq = csd->freq;
    dxLocator = csd->dxLocator;
    spotterCall = csd->spotterCall;
    spotterCallValidateCode = csd->spotterCallValidateCode;
    dxPropMode = csd->dxPropMode;
    spotComment = csd->spotComment;
    dxLocatorIsFromNode = csd->dxLocatorIsFromNode;
    askQrzFailed = csd->askQrzFailed;
    dxDist = csd->dxDist;
    dxBrg = csd->dxBrg;
    dxCallWorked = csd->dxCallWorked;
    dxLocatorWorked = csd->sentToMemory;

    return QSharedPointer<ClusterSpotData>(this);

}


bool ClusterSpotData::operator==(const QSharedPointer<ClusterSpotData> cpd) const
{
    return rxTime == cpd->rxTime &&
            spotDateTime == cpd->spotDateTime &&
            band == cpd->band &&
            bandType == cpd->bandType &&
            mode == cpd->mode &&
            dxCall == cpd->dxCall &&
            dxCallValidateCode == cpd->dxCallValidateCode &&
            freq == cpd->freq &&
            dxLocator == cpd->dxLocator &&
            spotterCall == cpd->spotterCall &&
            spotterCallValidateCode == cpd->spotterCallValidateCode &&
            dxPropMode == cpd->dxPropMode &&
            spotComment == cpd->spotComment &&
            dxLocatorIsFromNode == cpd->dxLocatorIsFromNode &&
            askQrzFailed == cpd->askQrzFailed &&
            dxDist == cpd->dxDist &&
            dxBrg == cpd->dxBrg &&
            dxCallWorked == cpd->dxCallWorked &&
            dxLocatorWorked == cpd->sentToMemory;
}


bool ClusterSpotData::operator==(const ClusterSpotData &cpd) const
{
    return rxTime == cpd.rxTime &&
            spotDateTime == cpd.spotDateTime &&
            band == cpd.band &&
            bandType == cpd.bandType &&
            mode == cpd.mode &&
            dxCall == cpd.dxCall &&
            dxCallValidateCode == cpd.dxCallValidateCode &&
            freq == cpd.freq &&
            dxLocator == cpd.dxLocator &&
            spotterCall == cpd.spotterCall &&
            spotterCallValidateCode == cpd.spotterCallValidateCode &&
            dxPropMode == cpd.dxPropMode &&
            spotComment == cpd.spotComment &&
            dxLocatorIsFromNode == cpd.dxLocatorIsFromNode &&
            askQrzFailed == cpd.askQrzFailed &&
            dxDist == cpd.dxDist &&
            dxBrg == cpd.dxBrg &&
            dxCallWorked == cpd.dxCallWorked &&
            dxLocatorWorked == cpd.sentToMemory;
}

bool ClusterSpotData::operator!=(const QSharedPointer<ClusterSpotData> cpd) const
{

    return  rxTime != cpd->rxTime ||
            spotDateTime != cpd->spotDateTime ||
            band != cpd->band ||
            bandType != cpd->bandType ||
            mode != cpd->mode ||
            dxCall != cpd->dxCall ||
            dxCallValidateCode != cpd->dxCallValidateCode ||
            freq != cpd->freq ||
            dxLocator != cpd->dxLocator ||
            spotterCall != cpd->spotterCall ||
            spotterCallValidateCode != cpd->spotterCallValidateCode ||
            dxPropMode != cpd->dxPropMode ||
            spotComment != cpd->spotComment ||
            dxLocatorIsFromNode != cpd->dxLocatorIsFromNode ||
            askQrzFailed != cpd->askQrzFailed ||
            dxDist != cpd->dxDist ||
            dxBrg != cpd->dxBrg ||
            dxCallWorked != cpd->dxCallWorked ||
            dxLocatorWorked != cpd->sentToMemory;
}

bool ClusterSpotData::operator!=(const ClusterSpotData &cpd) const
{

    return  rxTime != cpd.rxTime ||
            spotDateTime != cpd.spotDateTime ||
            band != cpd.band ||
            bandType != cpd.bandType ||
            mode != cpd.mode ||
            dxCall != cpd.dxCall ||
            dxCallValidateCode != cpd.dxCallValidateCode ||
            freq != cpd.freq ||
            dxLocator != cpd.dxLocator ||
            spotterCall != cpd.spotterCall ||
            spotterCallValidateCode != cpd.spotterCallValidateCode ||
            dxPropMode != cpd.dxPropMode ||
            spotComment != cpd.spotComment ||
            dxLocatorIsFromNode != cpd.dxLocatorIsFromNode ||
            askQrzFailed != cpd.askQrzFailed ||
            dxDist != cpd.dxDist ||
            dxBrg != cpd.dxBrg ||
            dxCallWorked != cpd.dxCallWorked ||
            dxLocatorWorked != cpd.sentToMemory;
}


//---------------------------------------------------------------------------


BandmapSpotData::BandmapSpotData(bandmapSpotType::SPOT_TYPE spotType_)
{
    clear();
    spotType = spotType_;
}


BandmapSpotData::BandmapSpotData(const BandmapSpotData &bsd) : SpotdataBase(bsd), ClusterSpotDataBase(bsd)
{
    runModeOn = bsd.runModeOn;
    offRunFreq = bsd.offRunFreq;
    cqResponse = bsd.cqResponse;
    district = bsd.district;
    districtWorked = bsd.districtWorked;
    rotBrg = bsd.rotBrg;
    rotConnected = bsd.rotConnected;
    isSelected = bsd.isSelected;
    spotType = bsd.spotType;
}

void BandmapSpotData::clear()
{
    SpotdataBase::clear();
    ClusterSpotDataBase::clear();
    runModeOn = false;
    offRunFreq = false;
    cqResponse = false;
    district.clear();
    districtWorked = false;
    rotBrg.clear();
    rotConnected =false;
    isSelected = false;
    spotType = bandmapSpotType::SPOT_TYPE::NONE;
}
bool BandmapSpotData::operator==(const BandmapSpotData &bsd) const
{
    return rxTime == bsd.rxTime &&
            spotDateTime == bsd.spotDateTime &&
            band == bsd.band &&
            bandType == bsd.bandType &&
            mode == bsd.mode &&
            dxCall == bsd.dxCall &&
            dxCallValidateCode == bsd.dxCallValidateCode &&
            freq == bsd.freq &&
            dxLocator == bsd.dxLocator &&
            spotterCall == bsd.spotterCall &&
            spotterCallValidateCode == bsd.spotterCallValidateCode &&
            dxPropMode == bsd.dxPropMode &&
            spotComment == bsd.spotComment &&
            dxLocatorIsFromNode == bsd.dxLocatorIsFromNode &&
            askQrzFailed == bsd.askQrzFailed &&
            dxDist == bsd.dxDist &&
            dxBrg == bsd.dxBrg &&
            dxCallWorked == bsd.dxCallWorked &&
            dxLocatorWorked == bsd.sentToMemory &&
            runModeOn == bsd.runModeOn &&
            offRunFreq == bsd.offRunFreq &&
            cqResponse == bsd.cqResponse &&
            district == bsd.district &&
            districtWorked == bsd.districtWorked &&
            rotBrg == bsd.rotBrg &&
            rotConnected == bsd.rotConnected &&
            isSelected == bsd.isSelected &&
            spotType == bsd.spotType;
}

bool BandmapSpotData::operator!=(const BandmapSpotData &bsd) const
{
    return  rxTime != bsd.rxTime ||
            spotDateTime != bsd.spotDateTime ||
            band != bsd.band ||
            bandType != bsd.bandType ||
            mode != bsd.mode ||
            dxCall != bsd.dxCall ||
            dxCallValidateCode != bsd.dxCallValidateCode ||
            freq != bsd.freq ||
            dxLocator != bsd.dxLocator ||
            spotterCall != bsd.spotterCall ||
            spotterCallValidateCode != bsd.spotterCallValidateCode ||
            dxPropMode != bsd.dxPropMode ||
            spotComment != bsd.spotComment ||
            dxLocatorIsFromNode != bsd.dxLocatorIsFromNode ||
            askQrzFailed != bsd.askQrzFailed ||
            dxDist != bsd.dxDist ||
            dxBrg != bsd.dxBrg ||
            dxCallWorked != bsd.dxCallWorked ||
            dxLocatorWorked != bsd.sentToMemory ||
            runModeOn != bsd.runModeOn ||
            offRunFreq != bsd.offRunFreq ||
            cqResponse != bsd.cqResponse ||
            district != bsd.district ||
            districtWorked != bsd.districtWorked ||
            rotBrg != bsd.rotBrg ||
            rotConnected != bsd.rotConnected ||
            isSelected != bsd.isSelected ||
            spotType != bsd.spotType;
}

QString BandmapSpotData::spotName()
{
    switch(spotType)
    {
        case bandmapSpotType::NONE:
        return "NONE";
        break;
        case bandmapSpotType::CLUSTER:
        return "CLUSTER";
        break;
        case bandmapSpotType::CLUSTER_MARKED:
        return "CLUSTER MARKED";
        break;
        case bandmapSpotType::LOGGED:
        return "LOGGED";
        break;
        case bandmapSpotType::MARKED:
        return "MARKED";
        break;
        case bandmapSpotType::SAVED:
        return "SAVED";
        break;
        case bandmapSpotType::CQ:
        return "CQ";
        break;
        case bandmapSpotType::DELETED:
        return "DELETED";
        break;

    };

}
