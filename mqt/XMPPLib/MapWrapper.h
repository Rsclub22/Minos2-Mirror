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
template <class itemtype>
uint qHash(const MapWrapper<itemtype> &m)
{
    return m.wt->qHash();
}

#endif // MAPWRAPPER_H
