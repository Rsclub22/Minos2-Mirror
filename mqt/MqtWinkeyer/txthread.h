/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Winkeyer Control
//
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2024
//
// Interprocess Control Logic
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2017
//
//
//
/////////////////////////////////////////////////////////////////////////////


#ifndef TXTHREAD_H
#define TXTHREAD_H

#include <QThread>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>
#include <QQueue>
#include <QByteArray>
#include <QTimer>
#include <QEventLoop>


class WinkeyerControl;

class TxThread : public QThread
{
    Q_OBJECT

public:
    explicit TxThread(WinkeyerControl *winkeyercontrol, QObject *parent = nullptr);

    void run() override;
    void stop();

signals:
    void writeData(const QByteArray &data);

private:
    WinkeyerControl *winkeyerControl;
    bool shouldTerminate = false;
    QTimer *timer;
    QEventLoop eventLoop;

    void writeABuffer(const QByteArray &data);
    void processImmediateCommands();

    void handleBackspace();
};

#endif // TXTHREAD_H
