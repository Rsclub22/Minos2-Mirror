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
    asb144M,
    asb432M,
    asb13G,
    asb24G,
    asb34G,
    asb57G,
    asb10G,
    asbMaxBand
};


class AirScoutLink: public QObject
{
    Q_OBJECT

    QSharedPointer<QUdpSocket> qus;
    QString oldWatch;
    QStringList watchList;

    // variablesfor checksums
    char lastbyte;
    int cs;
    char lcs;

    qint64 sendMessage(QString messagetype, QString messageText);
    void sendToAllBroadcast(QByteArray *packet);
    void askNearest(QString lastcall);
public:
    AirScoutLink();
    void usersChanged(QSharedPointer<QVector<QSharedPointer<KstUser> > > callVector, int chatId, QString filterString);
private slots:
    void onReadyRead();
};

#endif // AIRSCOUTLINK_H
