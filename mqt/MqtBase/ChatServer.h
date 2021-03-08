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

    void addChat(const QString &mess);
    void syncChat();
    void syncStations();

private slots:
    void SyncTimerTimer( );

    void onRigFreqChanged(Frequency /*f*/, BaseContestLog *c);
    void on_routerCall(bool err, QSharedPointer<MinosRPCObj> mro, const QString from );
    void on_notify(AnalysePubSubNotify an, const QString from );
signals:
    void ChatServerList(QVector<ChatServerApp>);
    void ChatMessages(QVector<QString>);
};


#endif // CHATSERVER_H
