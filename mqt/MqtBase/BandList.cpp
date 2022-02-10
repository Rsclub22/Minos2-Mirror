/*====================================================================================
    This file is part of AdjQt, the QT based version of the RSGB
    contest adjudication software.
    
    AdjQt and its predecessor AdjSQL are Copyright 1992 - 2016 Mike Goodey G0GJV 
 
    AdjQt is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    AdjQt is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with AdjQt in file gpl.txt.  If not, see <http://www.gnu.org/licenses/>.
    
======================================================================================*/

#include "base_pch.h"
#include "INIFile.h"
#include "bandmapcommon.h"
#include "BandList.h"
#include "cutils.h"
//---------------------------------------------------------------------------
const QString allHF{"ALLHF"}; // not to be translated

void BandInfo::setType ( const QString &t )
{
    type = t.toUpper();
}
QString BandInfo::getType() const
{
    return type;
}
QSharedPointer<ModeInfo> BandInfo::findMode(const QString &mstr) const
{
    for(auto m: modes)
    {
        if (mstr == "SSB" || mstr == "PH" || mstr == "PHONE")
        {
            if (m->getType() == "LSB")
                return m;
            if (m->getType() == "USB")
                return m;
        }
        if (m->getType() == mstr)
            return m;
    }
    return QSharedPointer<ModeInfo>();
}
QSharedPointer<ModeInfo> BandInfo::findMode(const QString &mstr, const Frequency &f, int &mp) const
{
    QSharedPointer<ModeInfo> mi = findMode(mstr);
    if (mi)
    {
        if (f >= mi->fcLow1 && f <= mi->fcHigh1)
        {
            mp = 1;
        }
        else if (f >= mi->fcLow2 && f <= mi->fcHigh2)
        {
            mp = 2;
        }
        else
        {
            mp = -1;
        }
    }
    return mi;
}
void ModeInfo::setType ( const QString &t )
{
    type = t.toUpper();
}
QString ModeInfo::getType() const
{
    return type;
}

bool ModeInfo::isFreqOK(const Frequency &f, bool &excludedFreq)
{
    for(const auto &exc:qAsConst(exclusions))
    {
        if (f >= exc->fLow && f <= exc->fHigh )
        {

            excludedFreq = true;
            return false;
        }
    }
    if (fcLow2)
    {
        if (f > fcHigh1 && f < fcLow2)
        {
            return false;
        }
        if (f >= fcLow2 && f <= fcHigh2)
        {
            return true;
        }
    }
    if (f >= fcLow1 && f <= fcHigh1)
    {
        return true;
    }

    return false;
}


BandList::BandList()
{
}
BandList::~BandList()
{
}
/*static*/
BandList &BandList::getBandList()
{
    static BandList blist;
    static bool loaded = false;
    if ( !loaded )
    {
        blist.parseFile ( "./Configuration/bandlist.xml" ) ;
        loaded = true;
    }
    return blist;
}
//---------------------------------------------------------------------------
bool BandList::parseFile (const QString &fname )
{
    QFile file( fname );

    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
//        mShowMessage ( "Cannot open " + fname, 0 );
        return false;
    }

    QByteArray total = file.readAll();

    QString buffer = QString( total );
    QString buffer2;

    int dtdPos = buffer.indexOf ( "<!DOCTYPE" );
    if ( dtdPos >= 0 )
    {
        buffer2 = buffer.left(dtdPos );

        int dtdEndPos = buffer.indexOf ( "]>" );
        if ( dtdEndPos < 0 )
        {
            return false;
        }
        buffer2 += buffer.mid ( dtdEndPos + 2, buffer.size() - dtdEndPos - 2 );
    }
    else
    {
        buffer2 = buffer;
    }

    TiXmlBase::SetCondenseWhiteSpace ( false );
    TiXmlDocument xdoc;
    xdoc.Parse ( buffer2.toStdString().c_str() );
    if ( xdoc.Error())
    {
        trace(QString(fname + " parse failed; ") + xdoc.ErrorDesc());
        return false;
    }
    TiXmlElement *tix = xdoc.RootElement();
    if ( !tix || !checkElementName ( tix, "Bandlist" ) )
    {
        return false;
    }
    for ( TiXmlElement * e = tix->FirstChildElement(); e; e = e->NextSiblingElement() )
    {
        if ( checkElementName ( e, "Band" ) )
        {
            if ( !parseBand ( e ) )     // at the moment it always returns true
            {
                return false;
            }
        }
    }

    std::sort(bandList.begin(), bandList.end(),
    [=](const QSharedPointer<BandInfo> a, const QSharedPointer<BandInfo> b)->bool
      {
          return a->fLow < b->fLow;
      }
    );

    readEnabled();
    return true;
}

bool BandList::parseBand ( TiXmlElement * e )
{
    // we know we are on a band; get the attributes we want
    QSharedPointer<BandInfo>  band(new BandInfo());

    band->setType ( getAttribute ( e, "type" ) );

    QString unit = getAttribute ( e, "unit" );
    QString temp = getAttribute ( e, "flow" );
    band->fLow = Frequency(temp);
    temp = getAttribute ( e, "fhigh" );
    band->fHigh = Frequency(temp);
    int fMult = 1;
    if ( unit == "K" )
    {
        fMult = 1000;
    }
    else
        if ( unit == "M" )
        {
            fMult = 1000000;
        }
        else
            if ( unit == "G" )
            {
                fMult = 1000000000;
            }
    band->fLow  = qint64(band->fLow) * fMult;
    band->fHigh = qint64(band->fHigh) * fMult;

    band->wlen = getAttribute ( e, "wlen" );
    band->uk = getAttribute ( e, "UK" );
    band->reg1test = getAttribute ( e, "Reg1Test" );
    band->adif = getAttribute ( e, "ADIF" );
    band->cabrillo = getAttribute ( e, "Cabrillo" );

    band->bandmapHigh = band->fHigh;
    temp = getAttribute(e, "bandmapHigh");
    if (!temp.isEmpty())
    {
        Frequency fTemp = Frequency(temp);
        fTemp = fTemp * fMult;
        band->bandmapHigh = fTemp;
    }
    band->bandmapLow = band->fLow;
    temp = getAttribute(e, "bandmapLow");
    if (!temp.isEmpty())
    {
        Frequency fTemp = Frequency(temp);
        fTemp = fTemp * fMult;
        band->bandmapLow = fTemp;
    }
    if (band->bandmapHigh < band->fLow || band->bandmapHigh > band->fHigh)
    {
        band->bandmapHigh = band->fHigh;
    }
    if (band->bandmapLow < band->fLow || band->bandmapLow > band->fHigh)
    {
        band->bandmapLow = band->fLow;
    }

    if (band->bandmapLow > band->bandmapHigh)
    {
        Frequency temp = band->bandmapLow;
        band->bandmapLow = band->bandmapHigh;
        band->bandmapHigh = temp;
    }

    band->bandColour = getAttribute(e, "Colour");

    if (band->bandColour.isEmpty())
    {
        if (band->cabrillo=="1800")  band->bandColour="red";
        if (band->cabrillo=="3500")  band->bandColour="green";
        if (band->cabrillo=="7000")  band->bandColour="blue";
        if (band->cabrillo=="14000") band->bandColour="teal";
        if (band->cabrillo=="21000") band->bandColour="purple";
        if (band->cabrillo=="28000") band->bandColour="navy";
        if (band->cabrillo=="50")    band->bandColour="red";
        if (band->cabrillo=="70")    band->bandColour="green";
        if (band->cabrillo=="144")   band->bandColour="blue";
        if (band->cabrillo=="432")   band->bandColour="teal";
        if (band->cabrillo=="1.2G")  band->bandColour="purple";
        if (band->cabrillo=="2.3G")  band->bandColour="navy";
        if (band->cabrillo=="3.4G")  band->bandColour="black";
        if (band->cabrillo=="5.7G")  band->bandColour="red";
        if (band->cabrillo=="10G")   band->bandColour="green";
        if (band->cabrillo=="24G")   band->bandColour="blue";
        if (band->cabrillo=="47G")   band->bandColour="teal";
        if (band->cabrillo=="76G")   band->bandColour="purple";
        if (band->cabrillo=="120G")  band->bandColour="navy";
        if (band->cabrillo=="134G")  band->bandColour="black";
        if (band->cabrillo=="241G")  band->bandColour="red";
    }

    for ( TiXmlElement * m = e->FirstChildElement(); m; m = m->NextSiblingElement() )
    {
        if ( checkElementName ( m, "Mode" ) )
        {
            if ( !parseMode ( band, unit, m ) )     // at the moment it always returns true
            {
                return false;
            }
        }
    }
    if (band->modes.size() > 0)
    {
        band->fcLow = band->fHigh;
        band->fcHigh = band->fLow;
        for (const auto &mi:qAsConst(band->modes))
        {
            if (mi->fcLow1 < band->fcLow)
            {
                band->fcLow = mi->fcLow1;
            }
            Frequency fh = mi->fcHigh2?mi->fcHigh2:mi->fcHigh1;
            if (fh > band->fcHigh)
            {
                band->fcHigh = fh;
            }
        }
    }
    else
    {
        band->fcLow = band->fLow;
        band->fcHigh = band->fHigh;
    }
    bandList.push_back ( band );

    return true;
}
bool BandList::parseMode (QSharedPointer<BandInfo> band, QString unit, TiXmlElement *e)
{
    QSharedPointer<ModeInfo> mode(new ModeInfo());
    mode->setType ( getAttribute ( e, "type" ) );

    QString temp;
    temp = getAttribute ( e, "flow" );
    mode->fLow = Frequency(temp);
    temp = getAttribute ( e, "fhigh" );
    mode->fHigh = Frequency(temp);

    temp = getAttribute ( e, "fclow1" );
    mode->fcLow1 = Frequency(temp);
    temp = getAttribute ( e, "fchigh1" );
    mode->fcHigh1 = Frequency(temp);

    temp = getAttribute ( e, "fclow2" );
    mode->fcLow2 = Frequency(temp);
    temp = getAttribute ( e, "fchigh2" );
    mode->fcHigh2 = Frequency(temp);

    if ( unit == "K" )
    {
        mode->fLow = qint64(mode->fLow) * 1000;
        mode->fHigh = qint64(mode->fHigh) * 1000;
        mode->fcLow1 = qint64(mode->fcLow1) * 1000;
        mode->fcHigh1 = qint64(mode->fcHigh1) * 1000;
        mode->fcLow2 = qint64(mode->fcLow2) * 1000;
        mode->fcHigh2 = qint64(mode->fcHigh2) * 1000;
    }
    else
        if ( unit == "M" )
        {
            mode->fLow = qint64(mode->fLow) * 1000000;
            mode->fHigh = qint64(mode->fHigh) * 1000000;
            mode->fcLow1 = qint64(mode->fcLow1) * 1000000;
            mode->fcHigh1 = qint64(mode->fcHigh1) * 1000000;
            mode->fcLow2 = qint64(mode->fcLow2) * 1000000;
            mode->fcHigh2 = qint64(mode->fcHigh2) * 1000000;
        }
        else
            if ( unit == "G" )
            {
                mode->fLow = qint64(mode->fLow) * 1000000000;
                mode->fHigh = qint64(mode->fHigh) * 1000000000;
                mode->fcLow1 = qint64(mode->fcLow1) * 1000000000;
                mode->fcHigh1 = qint64(mode->fcHigh1) * 1000000000;
                mode->fcLow2 = qint64(mode->fcLow2) * 1000000000;
                mode->fcHigh2 = qint64(mode->fcHigh2) * 1000000000;
            }

    for ( TiXmlElement * m = e->FirstChildElement(); m; m = m->NextSiblingElement() )
    {
        if ( checkElementName ( m, "Exclude" ) )
        {
            if ( !parseExclusion( mode, unit, m ) )    // at the moment it always returns true
            {
                return false;
            }
        }
    }

    band->modes.push_back(mode);
    return true;
}
bool BandList::parseExclusion (QSharedPointer<ModeInfo> mode, QString unit, TiXmlElement *e)
{
    QSharedPointer<ExclusionInfo> excl(new ExclusionInfo());

    QString temp = getAttribute ( e, "flow" );
    excl->fLow = Frequency(temp);
    temp = getAttribute ( e, "fhigh" );
    excl->fHigh = Frequency(temp);
    if ( unit == "K" )
    {
        excl->fLow = qint64(excl->fLow) * 1000;
        excl->fHigh = qint64(excl->fHigh) * 1000;
    }
    else
        if ( unit == "M" )
        {
            excl->fLow = qint64(excl->fLow) * 1000000;
            excl->fHigh = qint64(excl->fHigh) * 1000000;
        }
        else
            if ( unit == "G" )
            {
                excl->fLow = qint64(excl->fLow) * 1000000000;
                excl->fHigh = qint64(excl->fHigh) * 1000000000;
            }
    excl->reason = getAttribute( e, "reason");
    mode->exclusions.push_back(excl);
    return true;
}
bool BandList::findBand ( const QString &psfreq, QSharedPointer<BandInfo> &bi )
{
    QString sfreq = psfreq.trimmed();
    if ( sfreq.size() == 0 )
    {
        return false;
    }
    if ( sfreq == "1,2 GHz" )
    {
        sfreq = "1,3 GHz";
    }
    if ( sfreq == "3.6 MHz" )
    {
        sfreq = "3.5 MHz";
    }
    qint64 ifreq = sfreq.toInt();
    Frequency dhffreq(ifreq * 1000);
    Frequency dvhffreq(ifreq * 1000000);
    Frequency dmwvfreq(ifreq * 1000000000);

    for ( auto const &b: qAsConst(bandList ))
    {
        if (
                sfreq.compare(b->uk ) == 0
                || sfreq.compare(b->wlen ) == 0
                || sfreq.compare(b->adif ) == 0
                || sfreq.compare(b->cabrillo ) == 0
                || sfreq.compare(b->reg1test ) == 0
           )
        {
            bi = b;
            return true;
        }
    }
    for ( auto const &b: qAsConst(bandList ))
    {
        QString bandType = b->getType();
        Frequency bfhigh = b->fHigh;
        Frequency bflow = b->fLow;

        if ( bandType == "HF" )
        {
            if ( dhffreq <= bfhigh && dhffreq >= bflow )
            {
                bi = b;
                return true;
            }
        }
        else
            if ( bandType == VHF_BANDTYPE )
            {
                if ( dvhffreq <= bfhigh && dvhffreq >= bflow )
                {
                    bi = b;
                    return true;
                }
            }
            else
                if ( bandType == MW_BANDTYPE )
                {
                    if ( dmwvfreq <= bfhigh && dmwvfreq >= bflow )
                    {
                        bi = b;
                        return true;
                    }
                    if ( dvhffreq <= bfhigh && dvhffreq >= bflow )
                    {
                        bi = b;
                        return true;
                    }
                }
    }
    for ( auto const &b: qAsConst(bandList ))
    {
        // find in string isn't a massively good idea! But we are doing it after everything else has failed
        if ( b->uk.indexOf ( sfreq ) != -1
             || b->uk.indexOf ( sfreq ) != -1
             || b->adif.indexOf ( sfreq ) != -1
             || b->cabrillo.indexOf ( sfreq ) != -1
             || b->reg1test.indexOf ( sfreq ) != -1
           )
        {
            bi = b;
            return true;
        }
    }
    return false;
}


bool BandList::findBand(const Frequency &freq, QSharedPointer<BandInfo> &bi)
{
   for ( auto const &b: qAsConst(bandList ))
   {
      if (b->fLow <= freq && b->fHigh >= freq)
      {
         bi = b;
         return true;
      }
   }
   return false;
}

QString BandList::getBand(const Frequency &freq)
{
    QString band;
    BandList &bl = getBandList();
    QSharedPointer<BandInfo>  b1;
    bool b1Ok = bl.findBand(freq, b1);
    if (b1Ok)
    {
        band = b1->uk;
    }
    return band;
}

bool BandList::loadAllBands(QVector<QSharedPointer<BandInfo> > &bands, bool filtered)
{
    bool filtret = false;
    bands.clear();
    for ( auto const &b: qAsConst(bandList ))
    {
        // don't use bands > 1THz (can't support Freq display)
        // (fortunate that there aren't any!)

        if ((!filtered || b->enabled) && b->fHigh < Frequency(1000000000000))
        {
            if (b->getType().compare(VHF_BANDTYPE, Qt::CaseInsensitive) == 0
                    || b->getType().compare(MW_BANDTYPE, Qt::CaseInsensitive) == 0
                    || b->getType().compare(HF_BANDTYPE, Qt::CaseInsensitive) == 0)

                bands.append(b);
        }
        else
        {
            filtret = true;
        }
    }
    return filtret;
}


bool BandList::checkValidBand(Frequency freq)
{
    QSharedPointer<BandInfo>  bi;
    bool bandOK = false;
    bandOK = findBand(freq, bi);

    return bandOK;
}


QString BandList::findType(const QString &band) const
{
    for (const auto &b: qAsConst(bandList))
    {
        if (b->uk == band)
        {
            return b->getType();
        }

    }

    return "";
}
QString BandList::findType(const Frequency &freq) const
{
    for (const auto &b: qAsConst(bandList))
    {
        if (freq >= b->fLow && freq <= b->fHigh)
        {
            return b->getType();
        }
    }
    return "";
}

QString BandList::findBandNameFromIndex(int idx, QVector<QSharedPointer<BandInfo> > &bands)
{
    if (idx < bands.count() || idx >= 0)
    {
        for (int i = 0; i < bands.count(); i++)
        {
            if (i == idx)
            {
                return bands[i].data()->name();
            }
        }
    }

    return "";
}

bool BandList::isFreqOK(const Frequency &f, const QString &band, const QString &mode, bool &excludedFreq)
{
    QSharedPointer<BandInfo>  bi;
    bool bandOK = false;
    bool excludedF = false;
    bandOK = findBand(band, bi);
    if (bandOK)
    {
        QSharedPointer<ModeInfo> mi = bi->findMode(mode);
        if (mi)
        {

            bool fOk = mi->isFreqOK(f, excludedF);
            excludedFreq = excludedF;
            return fOk;
        }
        if (bi->modes.size() == 0)
        {
            if (f >= bi->fcLow && f <= bi->fcHigh)
            {
                return true;
            }
        }
    }
    return false;
}

void BandList::readEnabled()
{
    QString filename = "./Configuration/bandsEnabled.ini";
    INIFile settings(filename);
    for ( auto const &b: qAsConst(bandList ))
    {
        b->enabled = settings.getPrivateProfileBool("General", "Enabled_" + b->normalisedName(), true);
    }
}
void BandList::updateEnabled()
{
    QString filename = "./Configuration/bandsEnabled.ini";
    INIFile settings(filename);
    for ( auto const &b: qAsConst(bandList ))
    {
        settings.writePrivateProfileBool("General", "Enabled_" + b->normalisedName(), b->enabled);
    }
    settings.writePrivateProfileString( "", "", "" );
}
