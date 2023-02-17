#ifndef QMLCPPLINK_H
#define QMLCPPLINK_H

#include "qobjectdefs.h"
#include <QObject>
class BaseContestLog;

class QmlCppLink: public QObject
{
    Q_OBJECT
    BaseContestLog *ct = nullptr;
public:
    QmlCppLink();
    QmlCppLink(BaseContestLog *c);
    QmlCppLink(const QmlCppLink &l);
    Q_INVOKABLE QString locator(double lat, double longi) const;
};

Q_DECLARE_METATYPE(QmlCppLink)

#endif // QMLCPPLINK_H
