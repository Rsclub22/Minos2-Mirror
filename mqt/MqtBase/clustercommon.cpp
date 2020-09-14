#include "clustercommon.h"
#include "checkmodeagainstfreq.h"
#include "BandList.h"

const char * clusterStateList[] =
{
   QT_TRANSLATE_NOOP("clusterState", "Available"),
   QT_TRANSLATE_NOOP("clusterState", "Not Available"),
   QT_TRANSLATE_NOOP("clusterState", "No Contact")
};




void getMode(checkModeAgainstFreq* modeBandPlan, QString freq, const QString &dxBand, QString &dxModeStr, QString &dxModeMask)
{
    trace(QString("getMode: freq %1, dxBand %2, dxModeStr %3, dxModeMask %4").arg(freq).arg(dxBand).arg(dxModeStr).arg(dxModeMask));
    trace(QString("getMode: modeBandPlan loaded Ok - %1").arg(modeBandPlan->checkLoadedOk() ? "true" : "false"));
    if (dxBand != "")
    {

        if (modeBandPlan->checkLoadedOk() )
        {
            QString f = freq;
            QString b = dxBand;

            dxModeStr = modeBandPlan->getMode(b, f.remove('.').toDouble());
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


void getBand(QVector<QSharedPointer<BandInfo> > &bands, QString freq, QString &band, QString &bandMask)
{
    //double f = freq.append("000").remove('.').toDouble();
    double f = freq.remove('.').toDouble();

    for (int i = 0; i < bands.count(); i++)
    {
        if (f <= bands[i]->fHigh && f >= bands[i]->fLow)
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
    int tn = QTime::currentTime().second();
    QString t = QString::number(tn);
    if (tn < 10)
    {
        t = "0" + t;
    }
    QStringList dl = spotDate.split('-');
    if (dl.count() == 3)
    {
        if (dl[0].toInt() < 10 && dl[0].toInt() >= 0)
        {
            dl[0].prepend('0');
        }


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

qlonglong spotElapsedTime(qlonglong spotTime)
{
    qint64 curTime = QDateTime::currentMSecsSinceEpoch()/1000;  // currentSecsSinceEpoch only available since 5.8
    qlonglong elapsedTime = curTime - spotTime;
    return elapsedTime;
}
