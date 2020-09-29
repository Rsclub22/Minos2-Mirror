#ifndef FREQUENCY_H
#define FREQUENCY_H

#include <QMetaType>
#include <QDataStream>
#include <QLocale>

class Frequency
{
private:
    qint64 f = 0;
public:
    Frequency(qint64 fr):f(fr){}
    Frequency(QString fr);
    Frequency():f(0){}
    Frequency(const Frequency &fr):f(fr){}
    ~Frequency(){}
    bool operator< ( const Frequency& rhs ) const;
    bool operator> ( const Frequency& rhs ) const;
    bool operator<= ( const Frequency& rhs ) const;
    bool operator>= ( const Frequency& rhs ) const;
    bool operator== ( const Frequency& rhs ) const;
    bool operator!= ( const Frequency& rhs ) const;
    Frequency operator+ (const Frequency& rhs ) const{return Frequency(f + rhs.f);}
    Frequency operator- (const Frequency& rhs ) const{return Frequency(f - rhs.f);}
    operator qint64() const{return f;}
    //operator double() const {return f;}
    QString str() const ;
    QString traceStr() const;

    void clear(){f = 0;}
    bool isOK() const {return f >= 0;}
    bool isClear() const {return f == 0;}

    QString pretty_frequency_MHz_string( const QLocale &locale = QLocale()) const;
    friend QDataStream& operator>> (QDataStream& in, Frequency& ff)
    {
        // required in the WSJT-X code
        qint64 f;
        in  >> f;
        ff = Frequency(f);
        return in;
    }
    QString extractKhz();
    QString convertFreqStrDisp();
    QString convertFreqStrDispSingle();
    QString convertFreqStrDispSingleNoTrailZero();
    QString removeTrailingZeroes(QString sfreq);

};

class ShortFreq
{
private:
    qint32 sf = 0;
public:
    ShortFreq(qint32 fr):sf(fr){}
    ShortFreq(long fr):sf(fr){}
    ShortFreq(double fr):sf(fr){}
    ShortFreq(QString fr);
    ShortFreq():sf(0){}
    ShortFreq(const ShortFreq &fr):sf(fr){}
    ~ShortFreq(){}
    bool operator< ( const ShortFreq& rhs ) const;
    bool operator> ( const ShortFreq& rhs ) const;
    bool operator<= ( const ShortFreq& rhs ) const;
    bool operator>= ( const ShortFreq& rhs ) const;
    bool operator== ( const ShortFreq& rhs ) const;
    bool operator!= ( const ShortFreq& rhs ) const;
    operator qint32() const{return sf;}
    operator double() const {return sf;}
    QString str() const {return QString::number(sf);}
    QString traceStr() const {return str();}
    bool isOK(){return sf >= 0;}

};

//
// Qt type registration
//
void register_frequency_types ();

Q_DECLARE_METATYPE (Frequency);
Q_DECLARE_METATYPE (ShortFreq);

#endif // FREQUENCY_H
