#include "mqtUtils_pch.h"
#include <QRegularExpression>

void register_frequency_types ()
{
  qRegisterMetaType<Frequency> ("Frequency");
  qRegisterMetaType<ShortFreq> ("ShortFreq");
}
QString Frequency::str() const
{
    return (f == 0)?"00000000000":QString::number(f);
}
QString Frequency::traceStr() const
{
    return str();
}
QString Frequency::dispStr() const
{
    if (isClear())
    {
        return "";
    }
    return str();
}

static double constexpr MHz_factor {1.e6};
static int constexpr frequency_precsion {6};
bool Frequency::operator<(const Frequency &rhs) const
{
    return f < rhs.f;
}
bool Frequency::operator>(const Frequency &rhs) const
{
    return f > rhs.f;
}
bool Frequency::operator<=(const Frequency &rhs) const
{
    return f <= rhs.f;
}
bool Frequency::operator>=(const Frequency &rhs) const
{
    return f >= rhs.f;
}
bool Frequency::operator==(const Frequency &rhs) const
{
    return f == rhs.f;
}
bool Frequency::operator!=(const Frequency &rhs) const
{
    return f != rhs.f;
}
Frequency::Frequency(QString frequency)
{
    frequency = frequency.remove('.');

    bool ok = false;
    if (frequency.isEmpty())
    {
        f = 0;
    }
    else
    {
        f = frequency.toLongLong(&ok);
        if (!ok)
        {
            f = -1;
        }
    }
}
QString Frequency::extractKhz()
{
    QString khz = "***";
    if (!isClear())
    {
        QString sf = str();
        int i = sf.length();
        if (i >= 6)
        {
            khz = sf.mid(i - 6, 3);
            return khz;
        }
    }


    return khz;


}
QString Frequency::pretty_frequency_MHz_string (QLocale const& locale) const
{
  auto d_string = locale.toString (f / MHz_factor, 'f', frequency_precsion);
  return d_string.insert (d_string.size () - 3, QChar::Nbsp);
}
QString Frequency::convertFreqStrDisp()
{
    QString sfreq = str();
    int len = sfreq.length();

    if (len < 3)
    {
        sfreq = "00.000.000.000";    // error
    }
    else
    {
        for (int i = len; i > 3; i-=3)
        {
            sfreq = sfreq.insert(i - 3, '.');
        }
    }



    return sfreq;
}

/* Single Delimiter Utilities */

// convert freq string for display - single delimter
// 144.000000

QString Frequency::convertFreqStrDispSingle()
{
    QString sfreq = str();
    int len = sfreq.length();

    if (len >= 6)
    {
        sfreq = sfreq.insert(len - 6, '.');
    }
    else
    {
        sfreq = "0.0";    // error
    }

    return sfreq;

}


QString Frequency::convertFreqStrDispSingleNoTrailZero()
{
    QString sfreq = str();

    int len = sfreq.length();

    if (len >= 6)
    {
        sfreq = sfreq.insert(len - 6, '.');
    }
    else
    {
        sfreq = "??.???.???.???";    // error
    }
    sfreq = removeTrailingZeroes(sfreq);
    return sfreq;
}




QString Frequency::removeTrailingZeroes(QString sfreq)
{
    // remove trailing zero, apart from after period.
    QStringList fspl = sfreq.split('.');
    int last = fspl.count() - 1;
    fspl[last].remove(QRegularExpression("0+$"));  //remove trailing zeros
    if (fspl[last].count() == 0)
    {
        fspl[last] = "0";    // add back one zero
    }

    sfreq = fspl.join(".");

    return sfreq;
}



bool ShortFreq::operator<(const ShortFreq &rhs) const
{
    return sf < rhs.sf;
}
bool ShortFreq::operator>(const ShortFreq &rhs) const
{
    return sf > rhs.sf;
}
bool ShortFreq::operator<=(const ShortFreq &rhs) const
{
    return sf <= rhs.sf;
}
bool ShortFreq::operator>=(const ShortFreq &rhs) const
{
    return sf >= rhs.sf;
}
bool ShortFreq::operator==(const ShortFreq &rhs) const
{
    return sf == rhs.sf;
}
bool ShortFreq::operator!=(const ShortFreq &rhs) const
{
    return sf != rhs.sf;
}
ShortFreq::ShortFreq(QString frequency)
{
    frequency = frequency.remove('.');
    bool ok = false;
    sf = frequency.toLong(&ok);
    if (!ok)
    {
        sf = -1;
    }
}
