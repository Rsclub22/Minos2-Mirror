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
#include "BandList.h"
#include "cutils.h"
//---------------------------------------------------------------------------

void BandInfo::setType ( const QString &t )
{
    type = t.toUpper();
}
QString BandInfo::getType()
{
    return type;
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
    return true;
}

bool BandList::parseBand ( TiXmlElement * e )
{
    // we know we are on a band; get the attributes we want
    BandInfo band;

    band.setType ( getAttribute ( e, "type" ) );

    QString unit = getAttribute ( e, "unit" );
    QString temp = getAttribute ( e, "flow" );
    band.flow = temp.toInt();
    temp = getAttribute ( e, "fhigh" );
    band.fhigh = temp.toInt();
    if ( unit == "K" )
    {
        band.flow *= 1000.0;
        band.fhigh *= 1000.0;
    }
    else
        if ( unit == "M" )
        {
            band.flow *= 1000000.0;
            band.fhigh *= 1000000.0;
        }
        else
            if ( unit == "G" )
            {
                band.flow *= 1000000000.0;
                band.fhigh *= 1000000000.0;
            }

    band.wlen = getAttribute ( e, "wlen" );
    band.uk = getAttribute ( e, "UK" );
    band.reg1test = getAttribute ( e, "Reg1Test" );
    band.adif = getAttribute ( e, "ADIF" );
    band.cabrillo = getAttribute ( e, "Cabrillo" );

    bandList.push_back ( band );

    return true;
}
bool BandList::findBand ( const QString &psfreq, BandInfo &bi )
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
    int ifreq = sfreq.toInt();
    double dhffreq = ifreq * 1000.0;
    double dvhffreq = dhffreq * 1000.0;
    double dmwvfreq = dvhffreq * 1000.0;

    for ( int i = 0; i < bandList.size(); i++ )
    {
        if (
                sfreq.compare(bandList[ i ].uk ) == 0
                || sfreq.compare(bandList[ i ].wlen ) == 0
                || sfreq.compare(bandList[ i ].adif ) == 0
                || sfreq.compare(bandList[ i ].cabrillo ) == 0
                || sfreq.compare(bandList[ i ].reg1test ) == 0
           )
        {
            bi = bandList[ i ];
            return true;
        }
    }
    for ( int i = 0; i < bandList.size(); i++ )
    {
        QString bandType = bandList[ i ].getType();
        double bfhigh = bandList[ i ].fhigh;
        double bflow = bandList[ i ].flow;

        if ( bandType == "HF" )
        {
            if ( dhffreq <= bfhigh && dhffreq >= bflow )
            {
                bi = bandList[ i ];
                return true;
            }
        }
        else
            if ( bandType == "VHF" )
            {
                if ( dvhffreq <= bfhigh && dvhffreq >= bflow )
                {
                    bi = bandList[ i ];
                    return true;
                }
            }
            else
                if ( bandType == "MWAVE" )
                {
                    if ( dmwvfreq <= bfhigh && dmwvfreq >= bflow )
                    {
                        bi = bandList[ i ];
                        return true;
                    }
                    if ( dvhffreq <= bfhigh && dvhffreq >= bflow )
                    {
                        bi = bandList[ i ];
                        return true;
                    }
                }
    }
    for ( int i = 0; i < bandList.size(); i++ )
    {
        // find in string isn't a massively good idea! But we are doing it after everything else has failed
        if ( bandList[ i ].uk.indexOf ( sfreq ) != -1
             || bandList[ i ].uk.indexOf ( sfreq ) != -1
             || bandList[ i ].adif.indexOf ( sfreq ) != -1
             || bandList[ i ].cabrillo.indexOf ( sfreq ) != -1
             || bandList[ i ].reg1test.indexOf ( sfreq ) != -1
           )
        {
            bi = bandList[ i ];
            return true;
        }
    }
    return false;
}


bool BandList::findBand(int freq, BandInfo &bi)
{
   for (QVector<BandInfo>::iterator i = bandList.begin(); i != bandList.end(); i++)
   {
      if ((*i).flow <= freq && (*i).fhigh >= freq)
      {
         bi = (*i);
         return true;
      }
   }
   return false;
}



bool BandList::findBand(double freq, BandInfo &bi)
{
   for (QVector<BandInfo>::iterator i = bandList.begin(); i != bandList.end(); i++)
   {
      if ((*i).flow <= freq && (*i).fhigh >= freq)
      {
         bi = (*i);
         return true;
      }
   }
   return false;
}

void loadVhfAndUpBands(QVector<BandDetail*> &bands)
{
    BandList &blist = BandList::getBandList();

    for (int i = 0; i < blist.bandList.size(); i++)   // just load VHF/UHF bands
    {
        // don't use bands > 10GHz (can't support Freq display)
        if ( blist.bandList[i].uk != "24 GHz" && blist.bandList[i].uk != "47 GHz"
             && blist.bandList[i].uk != "76 GHz" && blist.bandList[i].uk != "120 GHz"
             && blist.bandList[i].uk != "134 GHz" && blist.bandList[i].uk != "248 GHz")

        {
            if (blist.bandList[i].getType().compare("VHF", Qt::CaseInsensitive) == 0 || blist.bandList[i].getType().compare("MWave", Qt::CaseInsensitive) == 0)
                bands.append(new BandDetail(blist.bandList[i].uk, blist.bandList[i].flow, blist.bandList[i].fhigh));
        }
    }

}


BandDetail::BandDetail(QString _name, double _flow, double _fhigh)
{
    name = _name;
    fLow = _flow;;
    fHigh = _fhigh;
}


