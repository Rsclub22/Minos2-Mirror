#include "mqtUtils_pch.h"

void register_frequency_types ()
{
  qRegisterMetaType<Frequency> ("Frequency");
  qRegisterMetaType<ShortFreq> ("ShortFreq");
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
    f = frequency.toLongLong(&ok);
    if (!ok)
    {
        f = -1;
    }
}

QString Frequency::pretty_frequency_MHz_string (QLocale const& locale) const
{
  auto d_string = locale.toString (f / MHz_factor, 'f', frequency_precsion);
  return d_string.insert (d_string.size () - 3, QChar::Nbsp);
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
    sf = frequency.toLong(&ok);
    if (!ok)
    {
        sf = -1;
    }
}
