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

#ifndef BandListH
#define BandListH

#include "base_pch.h"
//---------------------------------------------------------------------------
extern const QString allHF;

class BandInfo;

void loadVhfAndUpBands(QVector<QSharedPointer<BandInfo> > &bands);
bool checkValidBand(QString freq);
int getBandOffSet(QStringList supportedBands, QString contestBandStr);

class ModeInfo
{
    QString type;
public:
    double fLow = 0.0;
    double fHigh = 0.0;
    double fcLow1 = 0.0;
    double fcHigh1 = 0.0;
    double fcLow2 = 0.0;
    double fcHigh2 = 0.0;
    void setType ( const QString &t );
    QString getType();
};

class BandInfo
{
        QString type;
    public:
        double fLow = 0.0;
        double fHigh = 0.0;
        QString wlen;
        QString uk;
        QString cabrillo;
        QString adif;
        QString reg1test;

        QString bandColour;

        QVector<QSharedPointer<ModeInfo> > modes;

        void setType ( const QString &t );
        QString getType();

        QString name()
        {
            return uk;
        }
};
class TiXmlElement;
class BandList
{
    public:
        BandList();
        ~BandList();
        QVector<QSharedPointer<BandInfo> > bandList;
        bool parseFile ( const QString &bandFile );
        bool findBand (const QString &freq, QSharedPointer<BandInfo> & );
        bool findBand ( long freq, QSharedPointer<BandInfo>  & );
        bool findBand ( double freq, QSharedPointer<BandInfo>  &bi);

        static BandList &getBandList();

private:
        bool parseBand ( TiXmlElement * e );
        bool parseMode(QSharedPointer<BandInfo> band, QString unit, TiXmlElement *tix);
};

#endif
