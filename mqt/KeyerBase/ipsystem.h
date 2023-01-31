#ifndef IPSYSTEM_H
#define IPSYSTEM_H

#include <QThread>
#include <QTcpSocket>
#include <QTcpServer>
#include <QHostAddress>

class IPADataBuffer;

class IPSystem: public QObject
{
    Q_OBJECT
public:
    static IPSystem *createIPSystem();
    IPSystem();
    virtual ~IPSystem() override;

    void initialise(bool ds, IPADataBuffer *db, QHostAddress host, int16_t port);

    void closedown();

    void dostop();

    bool terminated = true;

    bool isListening() const;
    void setListening(bool newListening);

    bool listen();

    void doStart();
    int tryOutput();
    void finish();
private:
    QTcpServer *sv = nullptr;
    QTcpSocket *bc = nullptr;

    IPADataBuffer *dataBuffer = nullptr;
    bool ds;
    QHostAddress host;
    int16_t port;
    qint64 sequence = 0;
    bool listening = false;
    qint16 _pendingPacketSize = 0;

private slots:

    void onReadyRead();
    void onBytesWritten(qint64 /*cnt*/);

    void handleSocketAcceptError();
    void onNewConnection();
    void connected();
    void disconnected();
    void connectionError(QAbstractSocket::SocketError error);
signals:
    void sequenceCount(qint64);
    void finished();
};

#endif // IPSYSTEM_H
