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



#include "freqmodebandplan.h"

#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>


// parses a json file to get mode frequencies for each band or "legal" operating frequencies for each band

freqModeBandPlan::freqModeBandPlan() :
    loadedOk(false)
{

}


bool freqModeBandPlan::loadFile(QString filename)
{
    bool ret = false;
    ret = readFile(filename);

    return ret;
}




bool freqModeBandPlan::readFile(QString f)
{



    QJsonParseError err;
    QFile jf(f);
    QString s;
    ModeFreqDetail mfl;


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
                            freqHighLow.append(freqArray[j].toString().remove('.').toDouble(&ok));

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


bool freqModeBandPlan::checkLoadedOk()
{
    return loadedOk;
}
