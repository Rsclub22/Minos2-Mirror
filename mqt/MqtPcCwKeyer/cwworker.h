/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Pc Serial Port DTR CW Keyer Worker
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2025
//
//
//
//
/////////////////////////////////////////////////////////////////////////////



#ifndef CWWORKER_H
#define CWWORKER_H

#include <QObject>
#include <QQueue>
#include <QPair>
#include <QMutex>
#include <QSerialPort>
#include <functional>

class CwWorker : public QObject
{
    Q_OBJECT

public:
    explicit CwWorker(QObject *parent = nullptr);
    ~CwWorker();

    void enqueueAction(std::function<void()> func, int delayMs);
    void enqueueKey(bool on, int delayMs);
    void clear();
    void start();

    void setSerialPort(QSerialPort *port);

    void setInverKeyDownFlag(const bool invertKeyDownFlag);

signals:
    void finished();

private:
    QQueue<QPair<std::function<void()>, int>> actions;
    QMutex mutex;
    bool running = false;
    QSerialPort *serial = nullptr;
    bool invertDtrKeyDown = false;
    void setKeyOutput(bool keyDown);
};

#endif // CWWORKER_H
