#include "clustercommon.h"
#include "checkmodeagainstfreq.h"
#include "BandList.h"

const char * clusterStateList[] =
{
   QT_TRANSLATE_NOOP("clusterState", "Available"),
   QT_TRANSLATE_NOOP("clusterState", "Not Available"),
   QT_TRANSLATE_NOOP("clusterState", "No Contact")
};


ClusterClientFilterSettings::ClusterClientFilterSettings()
{

}


void ClusterClientFilterSettings::initFilterSettings(const QVector<QSharedPointer<BandInfo> > &bands, const QStringList &modes)
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

   for (auto const &m: modes)
   {
       modeFilterFlag.insert(m, false);
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





bool ClusterClientFilterSettings::getBandFilter(QString band)
{
    if (bandFilterSettings.contains(band))
    {
        return bandFilterSettings.value(band).bandFilterFlag;
    }
    else
    {
        return false;
    }
}




void ClusterClientFilterSettings::setBandFilter(QString band, bool setting)
{
    if (bandFilterSettings.contains(band))
    {
        BandFilterSettings bfs = bandFilterSettings.value(band);
        bfs.bandFilterFlag = setting;
        bandFilterSettings.insert(band, bfs);
    }

}






bool ClusterClientFilterSettings::getModeFilter(QString mode)
{

    if (modeFilterFlag.contains(mode))
    {
        return modeFilterFlag.value(mode);
    }



    return false;

}

void ClusterClientFilterSettings::setModeFilter(QString mode, bool setting)
{
    modeFilterFlag.insert(mode, setting);
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

    return false;
}

int ClusterClientFilterSettings::getDistanceFilter(QString band)
{
    if (bandFilterSettings.contains(band))
    {
        return bandFilterSettings.value(band).distanceFilter;
    }

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


    QMutableMapIterator<QString, BandFilterSettings> i(this->bandFilterSettings);
    while (i.hasNext())
    {
        i.next();
        i.value().bandFilterFlag = ccfs.bandFilterSettings.value(i.key()).bandFilterFlag;
        i.value().distanceFilter = ccfs.bandFilterSettings.value(i.key()).distanceFilter;
        i.value().ignoreDistanceFlag = ccfs.bandFilterSettings.value(i.key()).ignoreDistanceFlag;
        i.value().ignoreEmptyDistanceFlag = ccfs.bandFilterSettings.value(i.key()).ignoreEmptyDistanceFlag;
        i.value().bandType = ccfs.bandFilterSettings.value(i.key()).bandType;
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
