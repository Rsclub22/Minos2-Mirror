#ifndef CWWORKER_H
#define CWWORKER_H

#include <QObject>
#include <QQueue>
#include <QPair>
#include <functional>
#include <QMutex>

class CwWorker : public QObject
{
    Q_OBJECT
public:
    explicit CwWorker(QObject *parent = nullptr);
    ~CwWorker();

    void enqueueAction(std::function<void()> func, int delayMs);
    void start();
    void clear();

signals:

    void finished();

private:
    QQueue<QPair<std::function<void()>, int>> actions;
    QMutex mutex;
    bool running = false;
};

#endif // CWWORKER_H
