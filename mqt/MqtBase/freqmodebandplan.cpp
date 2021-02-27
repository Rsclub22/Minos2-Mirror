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

// analyses BandList to get mode frequencies for each band or "illegal" operating frequencies for each band

freqModeBandPlan::freqModeBandPlan() :
    loadedOk(false)
{

}

bool freqModeBandPlan::loadBandsFromBandList()
{
    bandModeFreqList.clear();
    BandList &blist = BandList::getBandList();
    for (auto const &b: qAsConst(blist.bandList))
    {
        QMap<QString, ModeFreqDetail<Frequency>> modeFreqList;
        for (auto const &m: qAsConst(b->modes))
        {
            ModeFreqDetail<Frequency> mfl;

            addPair(mfl, m->fcLow1, m->fcHigh1);

            if (qint64(m->fcLow2) > 0)
            {
                addPair(mfl, m->fcLow2, m->fcHigh2);
            }
            modeFreqList.insert(m->getType(), mfl);
        }
        if (modeFreqList.count())
        {
            bandModeFreqList.insert(b->name(), modeFreqList);
        }
    }
    loadedOk = bandModeFreqList.size() > 0;
    return loadedOk;
}

void freqModeBandPlan::addPair(ModeFreqDetail<Frequency> &mfl, Frequency fLow, Frequency fHigh)
{
    QList<Frequency> freqHighLow;
    freqHighLow.append(fLow);
    freqHighLow.append(fHigh);
    mfl.freq.append(freqHighLow);
}
bool freqModeBandPlan::loadExclusionsFromBandList()
{
    bandModeFreqList.clear();
    BandList &blist = BandList::getBandList();
    for (auto const &b: qAsConst(blist.bandList))
    {
        QMap<QString, ModeFreqDetail<Frequency>> modeFreqList;
        for (auto const &m: qAsConst(b->modes))
        {
            ModeFreqDetail<Frequency> mfl;
            if (b->fLow < m->fcLow1)
            {
                addPair(mfl, b->fLow, m->fcLow1);
            }
            for (auto const &e: qAsConst(m->exclusions))
            {
                addPair(mfl, e->fLow, e->fHigh);
            }
            if (qint64(m->fcLow2) > 0)
            {
                // add the bit between contest segments as an exclusion
                addPair(mfl, m->fcHigh1, m->fcLow2);
            }
            Frequency fcHigh = std::max(m->fcHigh1, m->fcHigh2);
            if (fcHigh < b->fHigh)
            {
                addPair(mfl, fcHigh, b->fHigh);
            }
            if (mfl.freq.count())
            {
                modeFreqList.insert(m->getType(), mfl);
            }
        }
        if (modeFreqList.count())
        {
            bandModeFreqList.insert(b->name(), modeFreqList);
        }
    }
    loadedOk = bandModeFreqList.size() > 0;
    return loadedOk;
}
bool freqModeBandPlan::checkLoadedOk()
{
    return loadedOk;
}
