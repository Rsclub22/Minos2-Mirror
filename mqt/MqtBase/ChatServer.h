#ifndef CHATSERVER_H
#define CHATSERVER_H
#include "base_pch.h"

class ChatServerApp
{
public:
    QString routerName;
    QString app;
    Frequency freq;
    PublishState state;

    PubSubName psn() const
    {
        PubSubName p;
        p.setRouter(routerName);
        p.setAppName(app);
        return p;
    }


    bool operator==(const ChatServerApp& rhs) const
    {
        return routerName == rhs.routerName
                && app == rhs.app
                ;
    }
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
    QVector<ChatServerApp> chatServerList;
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
    void on_provider(Provider provider);
signals:
    void ChatServerList(QVector<ChatServerApp>);
    void ChatMessages(QVector<QString>);
};


#endif // CHATSERVER_H
