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

signals:
    void finished();

public slots:



private slots:
    void connected();
    void disconnected();
    void readyRead();

private:
    QTcpSocket *socket;
    QStringList msg;
    QTimer *recvTimer;

    bool msgComplete;
    bool clientConnected;

    int getErrorCode(QString msg);
};

#endif // RIGCTLDCLIENT_H
