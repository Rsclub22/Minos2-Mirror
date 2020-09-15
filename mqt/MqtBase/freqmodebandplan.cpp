////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Cluster Mode/Bandplan
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2019
//
///
//
//
/////////////////////////////////////////////////////////////////////////////


#include "BandList.h"
#include "freqmodebandplan.h"

//#include <QJsonDocument>
//#include <QJsonParseError>
//#include <QJsonObject>
//#include <QJsonArray>


// parses a json file to get mode frequencies for each band or "legal" operating frequencies for each band

freqModeBandPlan::freqModeBandPlan() :
    loadedOk(false)
{

}


bool freqModeBandPlan::loadBandsFromBandList()
{
    bandModeFreqList.clear();
    BandList &blist = BandList::getBandList();
    for (QVector<QSharedPointer<BandInfo> >::iterator b = blist.bandList.begin(); b != blist.bandList.end(); b++)
    {
        QMap<QString, ModeFreqDetail<double>> modeFreqList;
        for (QVector<QSharedPointer<ModeInfo> >::iterator m = (*b)->modes.begin(); m != (*b)->modes.end(); m++)
        {
            ModeFreqDetail<double> mfl;
            QList<double> freqHighLow;

            freqHighLow.append((*m)->fLow/1000.0);
            freqHighLow.append((*m)->fHigh/1000.0);
            mfl.freq.append(freqHighLow);

            modeFreqList.insert((*m)->getType(), mfl);
        }
        if (modeFreqList.count())
        {
            bandModeFreqList.insert((*b)->name(), modeFreqList);
        }
    }
    return bandModeFreqList.size() > 0;
}

bool freqModeBandPlan::loadExclusionsFromBandList()
{
    bandModeFreqList.clear();
    BandList &blist = BandList::getBandList();
    for (QVector<QSharedPointer<BandInfo> >::iterator b = blist.bandList.begin(); b != blist.bandList.end(); b++)
    {
        QMap<QString, ModeFreqDetail<double>> modeFreqList;
        for (QVector<QSharedPointer<ModeInfo> >::iterator m = (*b)->modes.begin(); m != (*b)->modes.end(); m++)
        {
            ModeFreqDetail<double> mfl;
            for (QVector<QSharedPointer<ExclusionInfo> >::iterator e = (*m)->exclusions.begin(); e != (*m)->exclusions.end(); e++)
            {
                QList<double> freqHighLow;
                freqHighLow.append((*e)->fLow/1000.0);
                freqHighLow.append((*e)->fHigh/1000.0);
                mfl.freq.append(freqHighLow);
            }
            if (mfl.freq.count())
            {
                modeFreqList.insert((*m)->getType(), mfl);
            }
        }
        if (modeFreqList.count())
        {
            bandModeFreqList.insert((*b)->name(), modeFreqList);
        }
    }
    return bandModeFreqList.size() > 0;
}


#ifdef RUBBISH
bool freqModeBandPlan::readFile(QString f)
{
/*
 * array of band object - bandModeArray of bandObj
 *      bandObj - array of modeObj
 *          modeObj - array of frequency pairs
 *
 * each modeobj creates ModeFreqDetail<double> mfl, with a list of frequencies, up to 9 pairs in QList<QList<T>> freq
 *
 * modeFreqList is a map of mfl agains mode
 *
 * bandModeFreqList is a map of modeFreqList against band
 */


    QJsonParseError err;
    QFile jf(f);
    QString s;
    ModeFreqDetail<double> mfl;


    if (jf.open(QIODevice::ReadOnly))
    {
        s = jf.readAll();
    }
    else
    {
        trace("Failed to open " + f );

    }


    QJsonDocument json = QJsonDocument::fromJson(s.toUtf8(), &err);
    if (!err.error)
    {
        if( json.isArray())
        {

            QJsonArray bandModeArray = json.array();

            QJsonObject bandObj;
            QJsonValue bandVal;
            QStringList bandlist;        // list of bands, normally 1
            QJsonArray modeArray;
            QJsonObject modeObj;
            QStringList modeList;
            QJsonValue modeVal;
            QJsonArray modeFreqArray;

            QJsonObject modeFreqObj;


            QJsonArray freqArray;



            QList<double> freqHighLow;

            for (int i = 0; i < bandModeArray.count(); i++)
            {
                QMap<QString, ModeFreqDetail<double>> modeFreqList;

                bandObj = bandModeArray[i].toObject();
                bandlist = bandObj.keys(); // got the band key name
                bandVal = bandObj[bandlist[0]];
                modeArray = bandVal.toArray();
                for (int k = 0; k < modeArray.count(); k++)
                {
                    modeObj = modeArray[k].toObject();
                    modeList = modeObj.keys();  // got the mode
                    modeVal = modeObj[modeList[0]];
                    modeFreqArray = modeVal.toArray();

                    mfl.freq.clear();

                    for (int f = 0; f < modeFreqArray.count(); f++)
                    {
                        modeFreqObj = modeFreqArray[f].toObject();
                        freqArray = modeFreqObj.value(QString::number(f)).toArray();
                        freqHighLow.clear();

                        if (freqArray.count() < 0 && freqArray.count() > 9)     // max 9 freqs.
                        {
                            trace(QString("Cluster Mode Plan - Too many freqs - %1 - %2").arg(bandlist[0].arg(modeList[0])));
                            return false;
                        }

                        for (int j = 0; j < freqArray.count(); j++)
                        {

                            bool ok = false;
                            QString faj = freqArray[j].toString();
                            faj = faj.remove('.');
                            freqHighLow.append(faj.toDouble(&ok));

                        }


                        mfl.freq.append(freqHighLow);

                    }
                    modeFreqList.insert(modeList[0], mfl);
                }

                bandModeFreqList.insert(bandlist[0], modeFreqList);
            }


            loadedOk = true;
            return true;
        }
        else
        {
            trace("Cluster Mode Plan Not a JSON object");
            return false;
        }
    }
    else
    {
        trace("Err " + err.errorString() + " Bad Json document " + s);
        return false;
    }

}

#endif
bool freqModeBandPlan::checkLoadedOk()
{
    return loadedOk;
}
