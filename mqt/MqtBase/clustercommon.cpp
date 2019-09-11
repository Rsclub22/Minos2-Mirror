#include "clustercommon.h"
#include "checkmodeagainstfreq.h"
#include "BandList.h"



void getMode(checkModeAgainstFreq* modeBandPlan, QString freq, const QString &dxBand, QString &dxModeStr, QString &dxModeMask)
{
    if (dxBand != "")
    {

        if (modeBandPlan->checkLoadedOk() )
        {
            QString f = freq;
            QString b = dxBand;

            dxModeStr = modeBandPlan->getMode(b, f.remove('.').toDouble());

            int modeMask = clusterModes.indexOf(dxModeStr);
            if (modeMask == -1)
            {
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
            dxModeStr = "None";
            dxModeMask = "0";

        }
    }
}


void getBand(QVector<BandDetail*> &bands, QString freq, QString &band, QString &bandMask)
{
    //double f = freq.append("000").remove('.').toDouble();
    double f = freq.remove('.').toDouble();

    for (int i = 0; i < bands.count(); i++)
    {
        if (f <= bands[i]->fHigh && f >= bands[i]->fLow)
        {
            band = bands[i]->name;
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
