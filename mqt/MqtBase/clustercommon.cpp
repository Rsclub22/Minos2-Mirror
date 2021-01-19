#include "clustercommon.h"
#include "checkmodeagainstfreq.h"
#include "BandList.h"

const char * clusterStateList[] =
{
   QT_TRANSLATE_NOOP("clusterState", "Available"),
   QT_TRANSLATE_NOOP("clusterState", "Not Available"),
   QT_TRANSLATE_NOOP("clusterState", "No Contact")
};



ModeFilterSettings::ModeFilterSettings()
{
    for (auto &m: clusterModes)
    {
        modeFilterFlag.insert(m, false);
    }
}


bool ModeFilterSettings::operator==(const ModeFilterSettings& mfs) const
{

    bool state = false;
    QMapIterator<QString, bool> i(mfs.modeFilterFlag);
    while(i.hasNext())
    {
        i.next();
        if (modeFilterFlag.value(i.key()) == i.value())
        {
            state = true;
        }
        else
        {
            state = false;
        }
    }

    return state;


}


bool ModeFilterSettings::contains(const QString mode)
{
    return modeFilterFlag.contains(mode);
}

bool ModeFilterSettings::testModeFilter(QString mode)
{
    if (modeFilterFlag.contains(mode))
    {
        return modeFilterFlag.value(mode);
    }
    else
    {
        trace(QString("testModeFilter: mode not in modeFilterFlag map"));
        return false;
    }
}

bool ModeFilterSettings::getModeFilter(QString mode)
{
    if (modeFilterFlag.contains(mode))
    {
        return modeFilterFlag.value(mode);
    }
    else
    {
        trace(QString("getModeFilter: mode not in modeFilterFlag map"));
        return false;
    }
}


void ModeFilterSettings::setModeFilter(QString mode, bool setting)
{
    if (clusterModes.contains(mode))
    {
        modeFilterFlag.insert(mode, setting);
    }
    else
    {
        trace(QString("setModeFilter: mode not in list of clustermodes"));
    }

}



ClusterClientFilterSettings::ClusterClientFilterSettings()
{

}


void ClusterClientFilterSettings::initFilterSettings(const QVector<QSharedPointer<BandInfo> > &bands)
{
   BandFilterSettings bfs;
   bfs.bandFilterFlag = false;
   bfs.distanceFilter = 0;
   bfs.ignoreDistanceFlag = false;
   bfs.ignoreEmptyDistanceFlag = false;

   for (auto const &b: bands)
   {

       bfs.bandType = b.data()->getType();
       bandFilterSettings.insert(b.data()->uk, bfs);
   }


}


void ClusterClientFilterSettings::setCallSignFilterList(QString cfl)
{
    callsignFilterList = cfl;

}

QString ClusterClientFilterSettings::getCallSignFilterList()
{
    return callsignFilterList;
}





bool ClusterClientFilterSettings::getBandFilter(QString band) const
{
    if (bandFilterSettings.contains(band))
    {
        return bandFilterSettings.value(band).bandFilterFlag;
    }
    else
    {
        return false;
        trace(QString("getBandFilter: band %1 not in list of bands").arg(band));
    }
}



// assumes band has been already placed in bandFilterSettings
void ClusterClientFilterSettings::setBandFilter(QString band, bool setting)
{
    if (bandFilterSettings.contains(band))
    {
        BandFilterSettings bfs = bandFilterSettings.value(band);
        bfs.bandFilterFlag = setting;
        bandFilterSettings.insert(band, bfs);
    }

}



void ClusterClientFilterSettings::setBandType(QString band, QString bandType)
{
    if (bandFilterSettings.contains(band))
    {
        BandFilterSettings bfs = bandFilterSettings.value(band);
        bfs.bandType = bandType;
        bandFilterSettings.insert(band, bfs);
    }
}

QString ClusterClientFilterSettings::getBandType(QString band)
{
    if (bandFilterSettings.contains(band))
    {
         return bandFilterSettings.value(band).bandType;
    }

    trace(QString("getBandType: band %1 missing").arg(band));
    return "";
}

bool ClusterClientFilterSettings::getModeFilter(QString mode)
{

    if (modeFilterFlag.contains(mode))
    {
        return modeFilterFlag.getModeFilter(mode);
    }


    trace(QString("getModeFilter: mode = %1 missing").arg(mode));
    return false;

}

void ClusterClientFilterSettings::setModeFilter(QString mode, bool setting)
{
    modeFilterFlag.setModeFilter(mode, setting);
}





bool ClusterClientFilterSettings::testDistanceFilter(int distance, QString band)
{

    if (bandFilterSettings.contains(band))
    {

        if (distance < bandFilterSettings.value(band).distanceFilter || bandFilterSettings.value(band).distanceFilter == 0)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    trace(QString("testDistanceFilter: band = %1 missing").arg(band));
    return false;
}

int ClusterClientFilterSettings::getDistanceFilter(QString band)
{
    if (bandFilterSettings.contains(band))
    {
        return bandFilterSettings.value(band).distanceFilter;
    }

    trace(QString("getDistanceFilter: band = %1 is missing").arg(band));
    return 0;
}


void ClusterClientFilterSettings::setDistanceFilter(QString band, int distance)
{
    if (bandFilterSettings.contains(band))
    {
        BandFilterSettings bfs = bandFilterSettings.value(band);
        bfs.distanceFilter = distance;
        bandFilterSettings.insert(band, bfs);
    }
}


bool ClusterClientFilterSettings::getIgnoreDistanceFlag(QString band)
{
    if (bandFilterSettings.contains(band))
    {
        return bandFilterSettings.value(band).ignoreDistanceFlag;
    }

    trace(QString("getIgnoreDistanceFlag: band = %1 is missing").arg(band));
    return false;
}

void ClusterClientFilterSettings::setIgnoreDistanceFlag(QString band, bool state)
{
    if (bandFilterSettings.contains(band))
    {
        BandFilterSettings bfs = bandFilterSettings.value(band);
        bfs.ignoreDistanceFlag = state;
        bandFilterSettings.insert(band, bfs);
    }
}


bool ClusterClientFilterSettings::getIgnoreEmptyDistanceFlag(QString band)
{
    if (bandFilterSettings.contains(band))
    {
        return bandFilterSettings.value(band).ignoreEmptyDistanceFlag;
    }

    trace(QString("getIgnoreEmptyDistanceFlag: band = %1 is missing").arg(band));
    return false;
}

void ClusterClientFilterSettings::setIgnoreEmptyDistanceFlag(QString band, bool state)
{
    if (bandFilterSettings.contains(band))
    {
        BandFilterSettings bfs = bandFilterSettings.value(band);
        bfs.ignoreEmptyDistanceFlag = state;
        bandFilterSettings.insert(band, bfs);
    }
}

ClusterClientFilterSettings::ClusterClientFilterSettings (const ClusterClientFilterSettings& ccfs)
{
    *this = ccfs;
}



ClusterClientFilterSettings& ClusterClientFilterSettings::operator= (const ClusterClientFilterSettings &ccfs)
{

    callsignFilterList = ccfs.callsignFilterList;
    locatorFilterList = ccfs.locatorFilterList;

    BandFilterSettings bfs;
    QMapIterator<QString, BandFilterSettings> i(ccfs.bandFilterSettings);
    while (i.hasNext())
    {
        i.next();
        bfs = i.value();
        bandFilterSettings.insert(i.key(), bfs);

    }

    modeFilterFlag = ccfs.modeFilterFlag;

    return *this;
}



bool ClusterClientFilterSettings::operator==( const ClusterClientFilterSettings& ccfs ) const
{
    if ( callsignFilterList == ccfs.callsignFilterList &&
         locatorFilterList == ccfs.locatorFilterList &&
         bandFilterSettings == ccfs.bandFilterSettings &&
         modeFilterFlag == ccfs.modeFilterFlag)

    {
        return true;
    }

    return false;

}

bool ClusterClientFilterSettings::operator!=( const ClusterClientFilterSettings& ccfs )
{
    return !(*this == ccfs);
}




QString ClusterClientFilterSettings::packFilterList(QStringList l)
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


QStringList ClusterClientFilterSettings::unpackFilterList(QString &sl)
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


BandmapClientFilterSettings::BandmapClientFilterSettings() :

    distanceFilter(0),
    ignoreDistanceFlag(false),
    ignoreEmptyDistanceFlag(false)

{


}


void BandmapClientFilterSettings::setModeFilter(QString mode, bool setting)
{
    if (clusterModes.contains(mode))
    {
        modeFilterFlag.setModeFilter(mode, setting);
    }
}

bool BandmapClientFilterSettings::getModeFilter(QString mode)
{
    if (modeFilterFlag.contains(mode))
    {
        return modeFilterFlag.getModeFilter(mode);
    }

    return false;
}
void BandmapClientFilterSettings::setDistanceFilter(int distance)
{
    distanceFilter = distance;
}

bool BandmapClientFilterSettings::testDistanceFilter(int distance)
{
    if (distance < distanceFilter || distanceFilter == 0)
    {
            return true;
    }
    else
    {
            return false;
    }


    return false;
}

int BandmapClientFilterSettings::getDistanceFilter()
{
    return distanceFilter;
}

bool BandmapClientFilterSettings::getIgnoreDistanceFlag()
{
    return ignoreDistanceFlag;
}

void BandmapClientFilterSettings::setIgnoreDistanceFlag(bool state)
{
    ignoreDistanceFlag = state;
}


bool BandmapClientFilterSettings::getIgnoreEmptyDistanceFlag()
{
    return ignoreEmptyDistanceFlag;
}

void BandmapClientFilterSettings::setIgnoreEmptyDistanceFlag(bool state)
{
    ignoreDistanceFlag = state;
}

BandmapClientFilterSettings::BandmapClientFilterSettings (const BandmapClientFilterSettings& bcfs)
{
    *this = bcfs;
}
BandmapClientFilterSettings& BandmapClientFilterSettings::operator= (const BandmapClientFilterSettings& bcfs)
{

    modeFilterFlag = bcfs.modeFilterFlag;
    distanceFilter = bcfs.distanceFilter;
    ignoreDistanceFlag = bcfs.ignoreDistanceFlag;
    ignoreEmptyDistanceFlag = bcfs.ignoreEmptyDistanceFlag;

    return *this;
}


bool BandmapClientFilterSettings::operator==( const BandmapClientFilterSettings& bcfs ) const
{
    if ( modeFilterFlag == bcfs.modeFilterFlag &&
         distanceFilter == bcfs.distanceFilter &&
         ignoreDistanceFlag == bcfs.ignoreDistanceFlag &&
         ignoreEmptyDistanceFlag == bcfs.ignoreEmptyDistanceFlag)

    {
        return true;
    }

    return false;

}


bool BandmapClientFilterSettings::operator!=( const BandmapClientFilterSettings& ccfs ) const
{
    return !(*this == ccfs);
}

QString BandmapClientFilterSettings::packFilterList(QStringList l)
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


QStringList BandmapClientFilterSettings::unpackFilterList(QString &sl)
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





void getMode(checkModeAgainstFreq* modeBandPlan, Frequency freq, const QString &dxBand, QString &dxModeStr, QString &dxModeMask)
{
    trace(QString("getMode: freq %1, dxBand %2, dxModeStr %3, dxModeMask %4").arg(freq.traceStr()).arg(dxBand).arg(dxModeStr).arg(dxModeMask));
    trace(QString("getMode: modeBandPlan loaded Ok - %1").arg(modeBandPlan->checkLoadedOk() ? "true" : "false"));
    if (dxBand != "")
    {

        if (modeBandPlan->checkLoadedOk() )
        {
            Frequency f = freq;
            QString b = dxBand;

            dxModeStr = modeBandPlan->getMode(b, f);
            trace(QString("getMode: found dxModeStr - %1").arg(dxModeStr));

            int modeMask = clusterModes.indexOf(dxModeStr);
            trace(QString("getMode: modeMask from clusterModes - %1").arg(QString::number(modeMask)));

            if (modeMask == -1)
            {
                trace(QString("getmode: mode will be none"));
                dxModeStr = "None";
                dxModeMask = "0";
            }
            else
            {
                dxModeMask = QString::number(modeMask);
            }

        }
        else
        {
            // modeplan file missing
            trace(QString("getMode - modeplan missing"));
            dxModeStr = "None";
            dxModeMask = "0";

        }
    }
}


void getBand(QVector<QSharedPointer<BandInfo> > &bands, Frequency fr, QString &band, QString &bandMask)
{
    for (int i = 0; i < bands.count(); i++)
    {
        if (fr <= bands[i]->fHigh && fr >= bands[i]->fLow)
        {
            band = bands[i]->name();
            bandMask = QString::number(i);
            break;
        }
    }
}






QDateTime getSpotDateTime(const QString spotDate, const QString spotTime)
{
    QDateTime dt = QDateTime();
    //int tn = QTime::currentTime().second();
    //QString t = QString::number(tn);
    //if (tn < 10)
    //{
    //    t = "0" + t;
    //}

    QStringList dl = spotDate.split('-');
    if (dl.count() == 3)
    {
        dl[0].prepend('0');         // correct if date isn't 0x for < 10
        dl[0] = dl[0].right(2);

        QString time = dl[2] + dl[1] + dl[0] + spotTime + "00";
        trace(QString("getSpotDateTime: spotTime is %1").arg(time));
        dt = QDateTime::fromString(time, "yyyyMMMddHHmmss" );
        dt.setTimeSpec(Qt::UTC);

    }

    return dt;
}



bool spotTimedOut(qlonglong spotTime, qlonglong timeToLive)
{
    qint64 curTime = QDateTime::currentMSecsSinceEpoch()/1000;  // currentSecsSinceEpoch only available since 5.8
    if ((curTime - spotTime) >= timeToLive)
    {
        return true;
    }
    return false;
}

qlonglong spotElapsedTime(qlonglong spotTime)
{
    qint64 curTime = QDateTime::currentMSecsSinceEpoch()/1000;  // currentSecsSinceEpoch only available since 5.8
    qlonglong elapsedTime = curTime - spotTime;
    return elapsedTime;
}

bool extractDxLocFromNodeFlag(QString locFlagMsg)
{
    if (locFlagMsg.contains("true"))
    {
        return true;
    }

    return false;
}
