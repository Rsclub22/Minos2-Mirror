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

class ExclusionInfo
{
public:
    Frequency fLow;
    Frequency fHigh;
    QString reason;
};

class ModeInfo
{
    QString type;
public:
    Frequency fLow;
    Frequency fHigh;
    Frequency fcLow1;
    Frequency fcHigh1;
    Frequency fcLow2;
    Frequency fcHigh2;

    QVector<QSharedPointer<ExclusionInfo> > exclusions;

    void setType ( const QString &t );
    QString getType() const;
};

class BandInfo
{
        QString type;
    public:
        Frequency fLow;
        Frequency fHigh;
        QString wlen;
        QString uk;
        QString cabrillo;
        QString adif;
        QString reg1test;

        QString bandColour;

        QVector<QSharedPointer<ModeInfo> > modes;

        QSharedPointer<ModeInfo> findMode(const QString &m) const;
        QSharedPointer<ModeInfo> findMode(const QString &mstr, const Frequency &f, int &mp) const;

        void setType ( const QString &t );
        QString getType() const;

        QString name() const
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
        bool findBand ( const Frequency &freq, QSharedPointer<BandInfo>  &bi);

        bool checkValidBand(Frequency freq);
        void loadVhfAndUpBands(QVector<QSharedPointer<BandInfo> > &bands);

        static BandList &getBandList();

private:
        bool parseBand ( TiXmlElement * e );
        bool parseMode(QSharedPointer<BandInfo> band, QString unit, TiXmlElement *tix);
        bool parseExclusion(QSharedPointer<ModeInfo> mode, QString unit, TiXmlElement *e);
};

#endif
