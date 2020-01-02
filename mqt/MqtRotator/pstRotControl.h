#ifndef PSTROTCONTROL_H
#define PSTROTCONTROL_H

#include <QObject>
#include <QUdpSocket>
#include <QNetworkDatagram>
#include <QDebug>

class PstRotControl : public QObject
{
    Q_OBJECT
public:
    explicit PstRotControl(QObject *parent = nullptr);

    void initPstSockets();
    void sendRequestBearing();
    void sendRotateTo(const QString bearing);
    void sendStop();

    void setPstNetAddress(QString address);
    void setPstPortAddress(QString port);


signals:
    void pstBearing(int);

public slots:

    void processPendingReportDatagrams();

private:

    void sendCommandToPstRotator(const QString msg);

    QString pstNetAddress;
    QHostAddress pstAddress;
    QUdpSocket* pstCommandSocket;
    unsigned short pstCommandPortNumber;


    QUdpSocket* pstReportSocket;
    unsigned short pstReportPortNumber;

    QString bearing;


};

#endif // PSTROTCONTROL_H
