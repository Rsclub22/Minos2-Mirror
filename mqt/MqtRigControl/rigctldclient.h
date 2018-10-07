#ifndef RIGCTLDCLIENT_H
#define RIGCTLDCLIENT_H

#include <QObject>
#include <QtNetwork>

class RigCtldClient : public QObject
{
    Q_OBJECT
public:
    explicit RigCtldClient(QObject *parent = nullptr);


    bool connectToHost(QString host, quint16 port);
    void disconnectFromHost();
    bool checkMsgRecieved();
    bool checkConnected();

    bool writeData(const QByteArray &data);

    QString findItemMessage(QString item);
    QString getRadioModel();
    QString getRadioModelName();
    QString getRadioManufacturerName();

    int retCode;
    int getRetCode();

    void startRecvTimer(int time);

signals:
    void finished();

public slots:



private slots:
    void connected();
    void disconnected();
    void readyRead();

    void recvTimeout();
private:
    QTcpSocket *socket;
    QStringList msg;
    QTimer *recvTimer;

    bool msgComplete;
    bool clientConnected;

    int bytes = 0;

    int getErrorCode(QString msg);
};

#endif // RIGCTLDCLIENT_H
