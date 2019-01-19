#include "clustercommon.h"




QDateTime getSpotDateTime(const QString spotDate, const QString spotTime)
{
    QDateTime dt = QDateTime();
    int tn = QTime::currentTime().second();
    QString t = QString::number(tn);
    if (tn < 10)
    {
        t = "0" + t;
    }
    QStringList dl = spotDate.split('-');
    if (dl.count() == 3)
    {
        dt = QDateTime::fromString(dl[2] + dl[1] + dl[0] + spotTime + t, "yyyyMMMddHHmmss" );
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
