#ifndef MAPWRAPPER_H
#define MAPWRAPPER_H

#include <QSharedPointer>
template <class itemtype>
class MapWrapper
{
public:
    QSharedPointer<itemtype> wt;

    MapWrapper(){}
    MapWrapper(itemtype *mp):wt(mp){}
    MapWrapper(QSharedPointer<itemtype> mp):wt(mp){}
    MapWrapper(const MapWrapper &m)
    {
        wt = m.wt;
    }
    ~MapWrapper(){}
    MapWrapper &operator= (const MapWrapper &m)
    {
        wt = m.wt;
        return *this;
    }
    operator bool()
    {
        return !wt.isNull();
    }
    bool operator!() const
    {
        return wt.isNull();
    }

    bool operator==(const MapWrapper &rhs) const
    {
        return *wt.data() == *rhs.wt.data();
    }
    bool operator!=(const MapWrapper &rhs) const
    {
        return *wt.data() != *rhs.wt.data();
    }
    bool operator<(const MapWrapper &rhs) const
    {
        return *wt.data() < *rhs.wt.data();
    }

};

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#define qHashRet size_t
#else
#define qHashRet uint
#endif

template <class itemtype>
qHashRet qHash(const MapWrapper<itemtype> &m)
{
    return m.wt->qHash();
}

#endif // MAPWRAPPER_H
