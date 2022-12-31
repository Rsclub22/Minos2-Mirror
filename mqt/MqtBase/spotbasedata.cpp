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


#include "clustercommon.h"
#include "contacts.h"
#include "contest.h"
#include "MinosParameters.h"
#include "calcs.h"
#include "MTrace.h"

#include "spotbasedata.h"

ClusterSpotData::ClusterSpotData()
{
}
//ClusterSpotData::ClusterSpotData(const ClusterSpotData &sdp)
//{
//    *this = sdp;
//}
ClusterSpotData::ClusterSpotData(bandmapSpotType::SPOT_TYPE spotType_)
{
    spotType = spotType_;
}
// --------------------------------------------------------------------------------------------
bool ClusterSpotData::sameSpotAs(QSharedPointer<ClusterSpotData> cpd)
{
    // to say two spots are the same we don't need a full equality operator
    return rxTime == cpd->rxTime &&
            clusterSpotType == cpd->clusterSpotType &&
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
void calcSpotDistanceBearing(BaseContestLog *ct, const QString& _locator, double* distance, int* bearing)
{
    bool locValid = true;
    QString locator = _locator;
    double latitude;
    double longitude;
    double dist;
    int brg = 0;

    if (ct && !locator.isEmpty())
    {
        if (locator.size() == 4)
        {
            locator.append("MM");
        }

        int locValres = lonlat( locator, longitude, latitude, MinosParameters::getMinosParameters() ->getAllowLoc4() );
        if ( ( locValres ) != LOC_OK )
        {
            locValid = false;
        }
        if (locValid)
        {
            ct->disbeara(longitude, latitude, dist, brg);
            *distance = dist;
            *bearing = brg;
        }
    }
}
void checkSpotWorked(BaseContestLog *ct, const Callsign &mcs, const QString &locator, const Frequency &freq, bool* callWorked, bool* locatorWorked)
{
    bool callfound = false;
    bool locfound = false;
    if (ct && !ct->isReadOnly())
    {
        for ( LogIterator i = ct->ctList.begin(); i != ct->ctList.end(); i++ )
        {
            if ((*i).wt->notValidContact() )
            {
                continue;
            }

            if (ct->isHF())
            {
                QSharedPointer<BandInfo> bandChanged = ct->checkBandChange(freq, (*i).wt->frequency.getValue().str());
                if (bandChanged)
                {
                    continue;
                }
            }
            if (!callfound)
            {
                if ((*i).wt->cs == mcs)
                {
                    *callWorked = true;
                    callfound = true;
                }
            }

            if (!locator.isEmpty())
            {
                QString loc = locator.mid(0,4);
                if ((*i).wt->loc.getLoc().mid(0,4) == loc)
                {
                    *locatorWorked = true;
                    locfound = true;
                }
            }

            if (callfound && locfound)
            {
                return;
            }
        }
    }
}

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
            QString bandlist = ct->contestBands.getValue();
            if (bandlist == allHF)
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
            checkSpotWorked(ct, cs, spotlist[DXLOCATOR], spotlist[DXFREQ], &callWorked, &locWorked);

            QString distance;
            QString bearing;
            if (!spotlist[DXLOCATOR].isEmpty())
            {
                double dist = 0;
                int brg = 0;
                calcSpotDistanceBearing(ct, spotlist[DXLOCATOR], &dist, &brg);
                distance = QString::number(static_cast< int> ( dist));
                bearing =  QString::number(brg);
            }

            bool dxLocFromNodeFlag = extractDxLocFromNodeFlag(spotlist[DXLOC_FROM_NODE_FLAG]);

            spotDateTime = QDateTime::fromString(spotlist[SPOTDATETIME], "yyyyMMMddHHmmss" );
            spotDateTime.setTimeSpec(Qt::UTC);
            if (!spotDateTime.isValid())
            {
                spotDateTime = QDateTime::currentDateTimeUtc();
            }
            qint64 rxTime = spotDateTime.toMSecsSinceEpoch() / 1000;

            trace(QString("Add Cluster Spot to Bandmap %1, %2, %3, %4").arg(spotlist[DXCALL], spotlist[DXFREQ], spotlist[DXMODESTR], spotlist[DXLOCATOR]));

            res = QSharedPointer<ClusterSpotData>(new ClusterSpotData(bandmapSpotType::CLUSTER));

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

