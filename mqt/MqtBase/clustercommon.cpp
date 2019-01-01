#include "clustercommon.h"




QDateTime getSpotDateTime(const QString spotDate, const QString spotTime)
{
    QDateTime dt = QDateTime();
    QStringList dl = spotDate.split('-');
    if (dl.count() == 3)
    {
        dt = QDateTime::fromString(dl[2] + dl[1] + dl[0] + spotTime, "yyyyMMMddHHmm" );
        dt.setTimeSpec(Qt::UTC);

    }

    return dt;
}

bool spotTimedOut(qlonglong spotTime, qlonglong timeToLive)
{
    qint64 curTime = QDateTime::currentMSecsSinceEpoch()/1000;  // currentSecsSinceEpoch only available since 5.8
    trace(QString("spotTimedOut: Difference = %1").arg(curTime - spotTime));
    if ((curTime - spotTime) >= timeToLive)
    {
        trace(QString("spotTimedOut: Spot Purged"));
        return true;
    }
    return false;
}
