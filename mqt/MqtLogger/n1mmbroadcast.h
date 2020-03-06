#ifndef N1MMBROADCAST_H
#define N1MMBROADCAST_H

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
    bool wsjtxRbSelect = false;
    QString contactsAddr;
    quint16 contactsPort = 0;
    QString extCSAddr;
    quint16 extCSPort = 0;
    QString wsjtxRbAddr;
    quint16 wsjtxRbPort = 0;

    QHostAddress contactsHost;
    QHostAddress extCSHost;
    QHostAddress wsjtxRbHost;

    QString genContactStanza(QString type, BaseContestLog *c, QSharedPointer<BaseContact> tct);
public:
    N1MMBroadcast();

    void configure();
private slots:
    void afterQSOSaved(BaseContestLog *c, QSharedPointer<BaseContact> tct);
    void wsjtxDatagram(QByteArray *);
    void callsignLookup(BaseContestLog *, QString);
};

#endif // N1MMBROADCAST_H
