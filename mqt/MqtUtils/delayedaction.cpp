#include <QTimer>
#include "delayedaction.h"

void delayedAction(QObject *p, std::function<void()> pred , int t)
{
        QTimer *timer = new QTimer(p);
        timer->setSingleShot(true);

        p->connect(timer, &QTimer::timeout, [=]()
        {
            // NB a lambda function
            pred();
            timer->deleteLater();
        }
        );

        timer->start(t);
}
