#ifndef AIRSCOUTLINK_H
#define AIRSCOUTLINK_H

#include "base_pch.h"

#include <QTimer>
#include <QUdpSocket>

#include <QSharedPointer>
#include <QHostAddress>
#include <QHostInfo>

#include <QSettings>
#include <QNetworkInterface>
#include "kstcallgridmodel.h"

enum ASBand {
    asb50M,
    asb70M,
    asb144M,
    asb432M,
    asb1p3G,
    asb2p4G,
    asb3p4G,
    asb5p7G,
    asb10G,
    asb24G,
    asb47G,
    asb74G,
    asbMaxBand
};

class Aircraft
{
    Q_DECLARE_TR_FUNCTIONS(Aircraft)
public:
    Aircraft(){}
    ~Aircraft(){}
    Aircraft(QStringList sl, int acoffset)
    {
        call = sl[acoffset++].trimmed();
        category = sl[acoffset++].trimmed();
        distance = sl[acoffset++].toInt();
        potential = sl[acoffset++].toInt();
        minutes = sl[acoffset++].toInt();
    }
    Aircraft(const Aircraft &ac):
      call(ac.call),
      category(ac.category),
      distance(ac.distance),
      potential(ac.potential),
      minutes(ac.minutes)
    {}

    void traceAircaft() const
    {
        trace(QString("Aircraft %1 category %2 distance %3 potential %4 minutes %5").arg(call).arg(category).arg(distance).arg(potential).arg(minutes) );
    }
    QString getAircraft() const
    {
        return tr("Aircraft %1\tcategory %2\tdistance %3\tpotential %4\tminutes %5").arg(call, 10).arg(category, 10).arg(distance, 4).arg(potential, 2).arg(minutes);
    }
    bool operator< ( const Aircraft& rhs ) const
    {
        if (minutes == rhs.minutes)
        {
            if (distance == rhs.distance)
                return potential < rhs.potential;
            else
                return distance < rhs.distance;
        }
        return minutes < rhs.minutes;
    }

    QString call;
    QString category;
    int distance;
    int potential;
    int minutes;
};

class AirScoutLink: public QObject
{
    Q_OBJECT

    QSharedPointer<QUdpSocket> qus;
    QString  oldWatch;
    QVector<QSharedPointer<KstUser> >  watchList;

    // variablesfor checksums
    char lastbyte;
    int cs;
    char lcs;

    bool assetPathInProgress = false;

    qint64 sendMessage(QString messagetype, QString messageText);
    void sendToAllBroadcast(QByteArray *packet);
    void askNearest(int row);
public:
    AirScoutLink();

    static const char *ASBandStrings[];

    void usersChanged(QSharedPointer<QVector<QSharedPointer<KstUser> > > callVector);
    void asSelected(QSharedPointer<KstUser> user);
    void clearWatchList();
    void asShowPath(QSharedPointer<KstUser> user, QSharedPointer<KstUser> other);
private slots:
    void onReadyRead();
signals:
    void acChanged(QSharedPointer<KstUser>);
};

#endif // AIRSCOUTLINK_H
