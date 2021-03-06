#ifndef QRZDISPLAYFRAME_H
#define QRZDISPLAYFRAME_H

#include <QFrame>
#include "base_pch.h"

#include "qrzServerCommon.h"


namespace Ui {
class QrzDisplayFrame;
}


class QrzDisplayServerRpc : public QObject
{
    Q_OBJECT
public:
    static const char * stateIndicator[];
    static const char * stateList[];

    explicit QrzDisplayServerRpc();
    virtual ~QrzDisplayServerRpc();

    static QrzDisplayServerRpc *getQrzDisplayServerRpc();


    void sendCallsignFromLoggerToQrzServer(QString callsign, QString frameid);

signals:



private:

    static QrzDisplayServerRpc *qrzDisplayServerRpc;
    //QTimer SyncTimer;

     QVector<QrzServer> serverList;


private slots:

    //void SyncTimerTimer( );
    void on_serverCall(bool err, QSharedPointer<MinosRPCObj> mro, const QString from);

    void on_notify(AnalysePubSubNotify an, const QString );



};






class QrzDisplayFrame : public QFrame
{
    Q_OBJECT

public:
    explicit QrzDisplayFrame(QWidget *parent = nullptr);
    ~QrzDisplayFrame();

    void getQrzDetailsForLogger(QString callign);


private slots:
    void onLoggerQrzMessage(QrzServerMessage qrzRequest);
private:
    Ui::QrzDisplayFrame *ui;

    QrzDisplayServerRpc *qrzDisplayServerRpc;

    QVector<QrzServerMessage> qrzRequestQueue;
    QrzServerMessage requestedStation;

    void clear();
};

#endif // QRZDISPLAYFRAME_H
