#ifndef CHATSERVER_H
#define CHATSERVER_H
#include "base_pch.h"

class Server
{
public:
    QString serverName;
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
    QVector<Server> serverList;
    QTimer SyncTimer;

    void addChat(const QString &mess);
    void syncChat();
    void syncStations();

private slots:
    void SyncTimerTimer( );

    void onRigFreqChanged(Frequency /*f*/, BaseContestLog *c);
    void on_serverCall(bool err, QSharedPointer<MinosRPCObj> mro, const QString &from );
    void on_notify( bool err, QSharedPointer<MinosRPCObj>, const QString &from );
signals:
    void ChatServerList(QVector<Server>);
    void ChatMessages(QVector<QString>);
};


#endif // CHATSERVER_H
