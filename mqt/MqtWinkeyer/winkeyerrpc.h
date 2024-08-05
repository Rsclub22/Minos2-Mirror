#ifndef WINKEYERRPC_H
#define WINKEYERRPC_H

#include <QObject>

class WinkeyerRpc : public QObject
{
    Q_OBJECT
public:
    explicit WinkeyerRpc(QObject *parent = nullptr);

signals:
};

#endif // WINKEYERRPC_H
