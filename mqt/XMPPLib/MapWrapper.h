#ifndef MAPWRAPPER_H
#define MAPWRAPPER_H

#include <QSharedPointer>
#include <QPointer>
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
template <class itemtype>
class MapKeyWrapper
{
public:
    QPointer<itemtype> k;
    MapKeyWrapper(){}
    MapKeyWrapper(itemtype *mp):k(mp){}

    MapKeyWrapper(QPointer<itemtype> mp):k(mp.data()){}
    MapKeyWrapper(const MapKeyWrapper &m)
    {
        k = m.k;
    }
    MapKeyWrapper &operator= (const MapKeyWrapper &m)
    {
        k = m.k;
        return *this;
    }
    ~MapKeyWrapper()
    {

    }
    operator bool()
    {
        return !k.isNull();
    }
    bool operator!() const
    {
        return k.isNull();
    }
    bool operator==(const MapKeyWrapper &rhs) const
    {
        return *k == *rhs.k;
    }
    bool operator!=(const MapKeyWrapper &rhs) const
    {
        return *k != *rhs.k;
    }
    bool operator<(const MapKeyWrapper &rhs) const
    {
        return *k < *rhs.k;
    }

};

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#define qHashRet size_t
#else
#define qHashRet uint
#endif

template <class itemtype>
qHashRet qHash(const MapKeyWrapper<itemtype> &m)
{
    return m.k->qHash();
}

#endif // MAPWRAPPER_H
