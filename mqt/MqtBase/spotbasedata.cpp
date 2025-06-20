/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2020 - 2025
//
//
//
//
/////////////////////////////////////////////////////////////////////////////

#include "cutils.h"
#include "clustercommon.h"
#include "contest.h"
#include "MTrace.h"

#include "spotbasedata.h"

ClusterSpotData::ClusterSpotData()
{
    qRegisterMetaType< QSharedPointer<ClusterSpotData> > ( "QSharedPointer<ClusterSpotData>" );
}
ClusterSpotData::ClusterSpotData(bandmapSpotType::SPOT_TYPE spotType_)
{
    spotType = spotType_;
}
// --------------------------------------------------------------------------------------------
bool ClusterSpotData::sameSpotAs(QSharedPointer<ClusterSpotData> cpd)
{
    // to say two spots are the same we don't need a full equality operator
    return rxTime == cpd->rxTime &&
            showSpotType == cpd->showSpotType &&
            spotDateTime == cpd->spotDateTime &&
            band == cpd->band &&
            bandType == cpd->bandType &&
            mode == cpd->mode &&
            dxCall == cpd->dxCall &&
            freq == cpd->freq &&
            dxLocator == cpd->dxLocator &&
            spotterCall == cpd->spotterCall &&
            dxPropMode == cpd->dxPropMode &&
            spotComment == cpd->spotComment &&
            dxCallWorked == cpd->dxCallWorked &&
            dxLocatorWorked == cpd->sentToMemory;
}

//---------------------------------------------------------------------------



bool ClusterSpotData::spotMatchesDxCallsignFreqAndTime(const QSharedPointer<ClusterSpotData> &other, int freqTolerance, int timeToleranceMins) const
{
    if (!other)
    {
       return false;
    }


    trace(QString("[repeat check] List call = %1, Spot Call = %2").arg(getDxCallStr().trimmed().toUpper()).arg(other->getDxCallStr().trimmed().toUpper()));
    if (getDxCallStr().trimmed().toUpper() != other->getDxCallStr().trimmed().toUpper())
    {
        trace(QString("[repeat check] No Callsign Match"));
        return false;
    }

    if (getBand().trimmed().toUpper() != other->getBand().trimmed().toUpper())
    {
       trace(QString("[repeat check] No Band Match"));
       return false;
    }


    qint64 deltaHz = qAbs(qint64(getFreq() - other->getFreq()));
    if (deltaHz > freqTolerance)
    {
        trace(QString("[repeat check] No Freq Match"));
        return false;
    }

    if (timeToleranceMins)      // if 0 mins ignore time
    {
        qint64 timeDiff = qAbs(spotDateTime.secsTo(other->spotDateTime));
        if (timeDiff > timeToleranceMins * 60)
        {
            trace(QString("[repeat check] Out of time"));
            return false;
        }

    }

    trace(QString("[repeat check] Spot is a repeat (callsign, band, freq, time match)"));
    return true;
}




QString ClusterSpotData::spotName()
{
    return spotName(spotType);
}

/*static*/ QString ClusterSpotData::spotName(bandmapSpotType::SPOT_TYPE _spotType)
{
    switch(_spotType)
    {
        default:
        case bandmapSpotType::NONE:
        return tr("NONE");
        break;
        case bandmapSpotType::CLUSTER:
        return tr("CLUSTER");
        break;
        case bandmapSpotType::CLUSTER_MARKED:
        return tr("CLUSTER MARKED");
        break;
        case bandmapSpotType::LOGGED:
        return tr("LOGGED");
        break;
        case bandmapSpotType::MARKED:
        return tr("MARKED");
        break;
        case bandmapSpotType::SAVED:
        return tr("SAVED");
        break;
        case bandmapSpotType::CQ:
        return tr("CQ");
        break;
        case bandmapSpotType::DELETED:
        return tr("DELETED");
        break;

    };

}

//======================================================================================================

QSharedPointer<ClusterSpotData> stringToDxSpot(QString spot, BaseContestLog *ct, qlonglong &timeToLive)
{
    QSharedPointer<ClusterSpotData> res;
    QDateTime spotDateTime = QDateTime::currentDateTimeUtc();

    QStringList sl;
    if (spot.contains(DXSPOT))
    {
       sl = spot.split(DXSPOT);
    }
    else if (spot.contains(RESENTSPOT))
    {
       sl = spot.split(RESENTSPOT);
    }
    if (sl.count() == 2)
    {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        QStringList spotlist = sl[1].split(':', Qt::KeepEmptyParts);
#else
        QStringList spotlist = sl[1].split(':', QString::KeepEmptyParts);
#endif

        if (spotlist.count() == TTLVALUE +1)
        {
            bool ok = false;
            int ttl = spotlist[TTLVALUE].toInt(&ok);
            if (ok)
            {
                if (ttl >= MIN_TTL && ttl <= MAX_TTL)
                {
                    timeToLive = ttl * 60; // seconds
                }
            }

            //-------------------------------------------------------

            //timeToLive = 120; // for testing.....

            //--------------------------------------------------------

            // check to see if spot is for this contest band

            QString band = spotlist[DXBANDSTR];
            if (ct->isHF())
            {
                BandList &blist = BandList::getBandList();
                QSharedPointer<BandInfo>  bi;
                bool bandOK = blist.findBand(band, bi);
                if (!bandOK || bi->getType() != HF_BANDTYPE)
                {
                   return res;
                }

            }
            else if (spotlist[DXBANDSTR] != ct->currentBand.getValue())
            {
                return res;  // not for this contest band
            }

            // check to see if call or locator worked
            bool callWorked = false;
            bool locWorked = false;

            Callsign cs;
            cs.setFullCall(spotlist[DXCALL]);
            ct->checkSpotWorked(cs, spotlist[DXLOCATOR], spotlist[DXFREQ], &callWorked, &locWorked);

            QString distance;
            QString bearing;
            if (!spotlist[DXLOCATOR].isEmpty())
            {
                double dist = 0;
                int brg = 0;
                ct->calcDistanceBearing(spotlist[DXLOCATOR], &dist, &brg);
                distance = QString::number(static_cast< int> ( dist));
                bearing =  QString::number(brg);
            }

            bool dxLocFromNodeFlag = extractDxLocFromNodeFlag(spotlist[DXLOC_FROM_NODE_FLAG]);

            spotDateTime = QDateTime::fromString(spotlist[SPOTDATETIME], "yyyyMMMddHHmmss" );
            spotDateTime = toUTC(spotDateTime);

            if (!spotDateTime.isValid())
            {
                spotDateTime = QDateTime::currentDateTimeUtc();
            }
            qint64 rxTime = spotDateTime.toMSecsSinceEpoch() / 1000;

            trace(QString("Create Cluster Spot %1, %2, %3, %4").arg(spotlist[DXCALL], spotlist[DXFREQ], spotlist[DXMODESTR], spotlist[DXLOCATOR]));

            res = QSharedPointer<ClusterSpotData>(new ClusterSpotData());

            res->setShowSpotType(false);
            res->setRxTime(rxTime);
            res->setSpotDateTime(spotDateTime);
            res->setFreq(spotlist[DXFREQ]);
            res->setBand(spotlist[DXBANDSTR]);
            res->setMode(spotlist[DXMODESTR]);
            res->setDxCall(spotlist[DXCALL]);
            res->setDxCallWorked(callWorked);
            res->setDxLocator(spotlist[DXLOCATOR]);
            res->setDxLocatorIsFromNode(dxLocFromNodeFlag);
            res->setDxLocatorWorked(locWorked);
            res->setDistrict(QString());
            res->setDistrictWorked(false);
            res->setDxDist(distance);
            res->setDxBrg(bearing);
            res->setSpotterCall(spotlist[SPOTCALL]);
            res->setSpotterLocator(spotlist[SPOTLOCATOR]);
            res->setSpotComment(spotlist[SPOTCOMMENT]);
            res->setSpotType(bandmapSpotType::SPOT_TYPE::CLUSTER);
       }
    }
    return res;
}

