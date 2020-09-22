#ifndef FREQUENCY_H
#define FREQUENCY_H

#include <QMetaType>
#include <QDataStream>
#include <QLocale>

class Frequency
{
private:
    qint64 f = 0;
    bool ok = true;
public:
    Frequency(qint64 fr):f(fr){}
    Frequency(long fr):f(fr){}
    Frequency(int fr):f(fr){}
    Frequency(double fr):f(fr){}
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
    operator double() const {return f;}
    QString str() const {return (f == 0)?"00000000000":QString::number(f);}
    void clear(){f = 0;}
    bool isOK() const {return ok;}
    bool isClear() const {return f == 0;}

    QString pretty_frequency_MHz_string( const QLocale &locale = QLocale()) const;
    friend QDataStream& operator>> (QDataStream& in, const Frequency& ff)
    {
        in >> ff;
        return in;
    }
};

class ShortFreq
{
private:
    qint32 sf = 0;
    bool ok = true;
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
    bool isOK(){return ok;}

};

//
// Qt type registration
//
void register_frequency_types ();

Q_DECLARE_METATYPE (Frequency);
Q_DECLARE_METATYPE (ShortFreq);

#endif // FREQUENCY_H
