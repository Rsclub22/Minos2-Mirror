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

//---------------------------------------------------------------------------
#include <QVector>
#include <QSharedPointer>

#include "frequency.h"
extern const QString allHF;

extern const char * HF_BANDTYPE;
extern const char * VHF_BANDTYPE;
extern const char * MW_BANDTYPE;
extern const char * NO_BANDTYPE;


class ExclusionInfo
{
public:
    Frequency fExcLow;
    Frequency fExcHigh;
    QString reason;
};

class ModeInfo
{
    QString type;
public:
    Frequency fModeLow;
    Frequency fModeHigh;
    Frequency fcLow1;
    Frequency fcHigh1;
    Frequency fcLow2;
    Frequency fcHigh2;

    QVector<QSharedPointer<ExclusionInfo> > exclusions;

    void setType ( const QString &t );
    QString getType() const;

    bool isFreqOK(const Frequency &f, bool &excludedFreq);
};

class BandInfo
{
        QString type;
    public:
        bool enabled = true;

        bool contestAllowed = true;

        Frequency fLow;
        Frequency fHigh;

        Frequency fcLow;
        Frequency fcHigh;

        Frequency bandmapLow;
        Frequency bandmapHigh;

        Frequency bandfreq;

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
        static QString normalise(const QString s)
        {
            QString n = s;
            n.remove('\x20').replace('H', 'h').replace('.', '_');
            return n;
        }
        QString normalisedName() const
        {
            QString n = normalise(uk);
            return n;
        }
        bool operator<(const BandInfo &rhs);
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

        static QString getBand(const Frequency &freq);

        QString findType(const QString &band) const;
        QString findType(const Frequency &freq) const;


        bool checkValidBand(Frequency freq);

        static BandList &getBandList();


        bool loadAllBands(QVector<QSharedPointer<BandInfo> > &bands, bool filtered = true);

        static QString findBandNameFromIndex(int i, QVector<QSharedPointer<BandInfo> > &bands);
        bool isFreqOK(const Frequency &f, const QString &band, const QString &mode, bool &excludedFreq);
        void updateEnabled();
private:
        bool parseBand ( TiXmlElement * e );
        bool parseMode(QSharedPointer<BandInfo> band, QString unit, TiXmlElement *tix);
        bool parseExclusion(QSharedPointer<ModeInfo> mode, QString unit, TiXmlElement *e);
        void readEnabled();
};

#endif
