#include "CwWorker.h"
#include <QThread>
#include <QMetaObject>
#include <QtConcurrentRun>

CwWorker::CwWorker(QObject *parent) : QObject(parent)
{
}

CwWorker::~CwWorker()
{
    clear();
}

void CwWorker::enqueueAction(std::function<void()> func, int delayMs)
{
    QMutexLocker locker(&mutex);
    actions.enqueue({func, delayMs});
}

void CwWorker::clear()
{
    QMutexLocker locker(&mutex);
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
        while (true)
        {
            QPair<std::function<void()>, int> action;

            {
                QMutexLocker locker(&mutex);
                if (actions.isEmpty())
                {
                    running = false;
                    // Emit finished signal safely in object's thread:
                    QMetaObject::invokeMethod(this, "finished", Qt::QueuedConnection);
                    return;
                }
                action = actions.dequeue();
            }

            // Execute the function in the object's thread:
            QMetaObject::invokeMethod(this, [func = action.first]() { func(); }, Qt::QueuedConnection);

            // Sleep for the delay in the worker thread (this thread):
            QThread::msleep(action.second);
        }
    });
}
