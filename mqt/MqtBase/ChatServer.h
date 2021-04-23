#ifndef CHATSERVER_H
#define CHATSERVER_H
#include "base_pch.h"

//class Provider
//{
//public:
//    Provider(){}

//    QString routerName;
//    QString app;
//    PublishState state = psNotConnected;
//}
class ChatServerApp
{
public:
    Frequency freq;
};

class ChatServer : public QObject
{
    Q_OBJECT

public:
    static const char * stateIndicator[];
    static const char * stateList[];


    explicit ChatServer();
    virtual ~ChatServer();
    static ChatServer *getChatServer();

    void sendMessage(QString mess);
private:
    static ChatServer *chatServer;
    QMap<Provider, ChatServerApp> chatServerList;
    QTimer SyncTimer;

    bool syncstat = false;
    QVector<QString> chatQueue;


    void addChat(const QString &mess);
    void syncChat();
    void syncStations();

private slots:
    void SyncTimerTimer( );

    void onRigFreqChanged(Frequency /*f*/, BaseContestLog *c);
    void on_routerCall(bool err, QSharedPointer<MinosRPCObj> mro, const QString from );
    void on_notify(AnalysePubSubNotify an, const QString from );
    void on_provider(Provider provider, QString cat);
signals:
    void ChatServerList(QMap<Provider, ChatServerApp>);
    void ChatMessages(QVector<QString>);
};


#endif // CHATSERVER_H
