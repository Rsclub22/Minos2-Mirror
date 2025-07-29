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




#include "cwworker.h"
#include <QtConcurrent/QtConcurrentRun>
#include <QThread>
#include <QElapsedTimer>

#ifdef Q_OS_WIN
#include <windows.h>
#include <mmsystem.h>
#endif
#ifdef _MSC_VER
#pragma comment(lib, "winmm.lib")
#endif

CwWorker::CwWorker(QObject *parent) : QObject(parent)
{
#ifdef Q_OS_WIN
    timeBeginPeriod(1); // Request 1ms timer resolution
#endif
}

CwWorker::~CwWorker()
{
    clear();
#ifdef Q_OS_WIN
    timeEndPeriod(1);
#endif
}

void CwWorker::setSerialPort(QSerialPort *port)
{
    serial = port;
}

void CwWorker::enqueueAction(std::function<void()> func, int delayMs)
{
    QMutexLocker locker(&mutex);
    actions.enqueue({func, delayMs});
}

void CwWorker::enqueueKey(bool on, int delayMs)
{
    enqueueAction([this, on]() {
        if (serial && serial->isOpen())
            serial->setDataTerminalReady(on);
    }, delayMs);
}

void CwWorker::clear()
{
    QMutexLocker locker(&mutex);
    serial->setDataTerminalReady(false);
    actions.clear();
}

void CwWorker::start()
{
    {
        QMutexLocker locker(&mutex);
        if (running || actions.isEmpty())
            return;
        running = true;
    }

    QtConcurrent::run([this]() {
#ifdef Q_OS_WIN
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
#endif
        while (true)
        {
            QPair<std::function<void()>, int> action;

            {
                QMutexLocker locker(&mutex);
                if (actions.isEmpty())
                {
                    running = false;
                    QMetaObject::invokeMethod(this, "finished", Qt::QueuedConnection);
                    return;
                }
                action = actions.dequeue();
            }

            action.first();

            QElapsedTimer timer;
            timer.start();
            while (timer.elapsed() < action.second)
                QThread::msleep(1);
        }
    });
}
