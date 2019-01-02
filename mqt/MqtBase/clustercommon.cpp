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
    qint64 curTime = QDateTime::currentSecsSinceEpoch();
    if ((curTime - spotTime) >= timeToLive)
    {
        trace(QString("spotTimedOut: Spot Purged"));
        return true;
    }
    return false;
}
