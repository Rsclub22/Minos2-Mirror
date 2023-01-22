#ifndef N1MMBROADCAST_H
#define N1MMBROADCAST_H

#include "clustercommon.h"
#include "spotbasedata.h"
#include <QObject>
#include <QUdpSocket>
class BaseContact;
class BaseContestLog;

class N1MMBroadcast:public QObject
{
    Q_OBJECT

    QUdpSocket bc;

    bool contactsSelect = false;
    bool extCSSelect = false;
    bool spotsSelect = false;
    bool wsjtxRbSelect = false;
    bool ADIFSelect = false;
    QString contactsAddr;
    quint16 contactsPort = 0;
    QString extCSAddr;
    quint16 extCSPort = 0;
    QString spotsAddr;
    quint16 spotsPort = 0;
    QString wsjtxRbAddr;
    quint16 wsjtxRbPort = 0;
    QString ADIFAddr;
    quint16 ADIFPort = 0;

    QHostAddress contactsHost;
    QHostAddress extCSHost;
    QHostAddress spotsHost;
    QHostAddress wsjtxRbHost;
    QHostAddress ADIFHost;

    QString genContactStanza(QString type, BaseContestLog *c, QSharedPointer<BaseContact> tct);
    
    QString genDeleteStanza(QSharedPointer<BaseContact> tct);

    QString genSpotsStanza(QSharedPointer<ClusterSpotData> spotMsg, bool delSpot);
public:
    N1MMBroadcast();

    void configure();
    bool setAddress(QString addr, QHostAddress &host);
    
private slots:
    void afterQSOSaved(BaseContestLog *c, QSharedPointer<BaseContact> tct);
    void wsjtxDatagram(int, QByteArray *);
    void callsignLookup(BaseContestLog *, QString);
    void dxSpots(QSharedPointer<ClusterSpotData> spotMsg, bool delSpot);
};

#endif // N1MMBROADCAST_H
