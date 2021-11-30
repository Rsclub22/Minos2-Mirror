#ifndef N1MMLINK_H
#define N1MMLINK_H

#include "base_pch.h"

#include <QUdpSocket>

class N1MMLink:public QObject
{
    Q_OBJECT
public:
    int port = 12060;
    QString address;

    N1MMLink(QObject *parent);

    void initialise();
    void disconnect();

    void sendFrequencyRequest(Frequency f, QString mode);

    Frequency getFrequency();
    QString getMode();
    QString getRadioName();

    bool isConnected();

private:
    QSharedPointer<QUdpSocket> qus;
    QUdpSocket bc;

    QTimer connectTimer;

    bool connected = false;
    Frequency currFrequency;
    QString radioName;
    QString mode;

    QString genFreqStanza(const Frequency &f);

private slots:

    void onReceiveUDP();
    void connectTimeout();
};

#endif // N1MMLINK_H
