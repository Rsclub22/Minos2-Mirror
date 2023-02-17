#include "contest.h"
#include "latlong.h"
#include "qmlcpplink.h"

QmlCppLink::QmlCppLink()
{
}

QmlCppLink::QmlCppLink(BaseContestLog *c):ct(c)
{}

QmlCppLink::QmlCppLink(const QmlCppLink &l):QObject(this)
{
    ct = l.ct;
}

QString QmlCppLink::locator(double lat, double longi) const
{
    QString loc;
    /*int ret =*/ geotoloc( lat, longi, loc );

    return loc.left(6);
}
