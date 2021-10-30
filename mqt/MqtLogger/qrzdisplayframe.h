#ifndef QRZDISPLAYFRAME_H
#define QRZDISPLAYFRAME_H

#include <QFrame>
#include "base_pch.h"
#include "cutils.h"
#include "qrzServerCommon.h"


namespace Ui {
class QrzDisplayFrame;
}

class QrzDisplayServerRpc;


class QrzDisplayFrame : public QFrame
{
    Q_OBJECT

public:
    explicit QrzDisplayFrame(QWidget *parent = nullptr);
    ~QrzDisplayFrame();

    void getQrzDetailsForLogger(QString callign);
    void setContest(BaseContestLog *c);



private slots:
    void onQrzServerLoggedState(bool, QString);
    void onLoggerQrzReply(QrzCallsignData cd, QString qrzReplyState, QString uuid);

    void onServerPingTimerTimeout();
    void onCallsignTextMouseDoubleClicked();
    void onQraTextMouseDoubleClicked();

    void onSearchQrzReturnPressed();
    void onBearingMouseDoubleClicked();
    void onNameTextMouseDoubleClicked();
private:
    Ui::QrzDisplayFrame *ui;
    BaseContestLog *ct = nullptr;

    UpperCaseValidator ucValidator;

    QrzDisplayServerRpc *qrzDisplayServerRpc;

    QTimer *serverPingTimer;
    bool receivedServerPing;

    QVector<QrzServerMessage> qrzRequestQueue;
    QrzServerMessage requestedStation;

    double distance = 0.0;
    int bearing = 0;

    void clear();

    void calcSpotDistanceBearing(const QString &_locator, double *distance, int *bearing);
    void setQrzMessageText(QString msg);
    void setLogonPushButtonLabelText(bool loggedOn);
};


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

    void loggerQrzReply(QrzCallsignData, QString, QString);
    void qrzServerLoggedState(bool, QString);

private:

    static QrzDisplayServerRpc *qrzDisplayServerRpc;
    //QTimer SyncTimer;

    QVector<QrzServer> serverList;


private slots:

    //void SyncTimerTimer( );
    void on_routerCall(bool err, QSharedPointer<MinosRPCObj> mro, const QString from);

    void on_notify(AnalysePubSubNotify an, const QString );

    void on_provider(Provider, QString cat);




};


#endif // QRZDISPLAYFRAME_H
