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
    void traceAircaft() const
    {
        trace(QString("Aircraft %1 category %2 distance %3 potential %4 minutes %5").arg(call).arg(category).arg(distance).arg(potential).arg(minutes) );
    }
    bool operator< ( const Aircraft& rhs ) const
    {
        if (minutes == rhs.minutes)
            return potential < rhs.potential;
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

    void usersChanged(QSharedPointer<QVector<QSharedPointer<KstUser> > > callVector, int chatId, QString filterString);
    void asSelected(QSharedPointer<KstUser> user);
    void clearWatchList();
private slots:
    void onReadyRead();
signals:
    void acChanged(QSharedPointer<KstUser>);
};

#endif // AIRSCOUTLINK_H
