#include "clustercommon.h"
#include "checkmodeagainstfreq.h"
#include "BandList.h"

const char * clusterStateList[] =
{
   QT_TRANSLATE_NOOP("clusterState", "Available"),
   QT_TRANSLATE_NOOP("clusterState", "Not Available"),
   QT_TRANSLATE_NOOP("clusterState", "No Contact")
};




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
    int tn = QTime::currentTime().second();
    QString t = QString::number(tn);
    if (tn < 10)
    {
        t = "0" + t;
    }
    QStringList dl = spotDate.split('-');
    if (dl.count() == 3)
    {
        dl[0].prepend('0');         // correct if date isn't 0x for < 10
        dl[0] = dl[0].right(2);

        QString time = dl[2] + dl[1] + dl[0] + spotTime + t;
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
