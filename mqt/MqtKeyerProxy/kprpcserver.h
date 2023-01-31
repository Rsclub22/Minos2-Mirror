#ifndef KPRPCSERVER_H
#define KPRPCSERVER_H

#include <QObject>
#include <QString>
#include <QMap>
#include "MinosRPC.h"
#include "AnalysePubSubNotify.h"

class MinosRPCObj;

class KPRPCServer:public QObject
{
    Q_OBJECT
public:
    KPRPCServer();
    ~KPRPCServer() override;
private slots:
      void on_routerCall(bool err, QSharedPointer<MinosRPCObj> mro, const QString from );
      void on_notify(AnalysePubSubNotify an, const QString from );
      void on_provider(Provider provider, QString cat);
};

#endif // KPRPCSERVER_H
