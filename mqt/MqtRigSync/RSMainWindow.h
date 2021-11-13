#ifndef RSMAINWINDOW_H
#define RSMAINWINDOW_H

#include "base_pch.h"
#include <QComboBox>
#include "RigCache.h"
#include "n1mmlink.h"
#include "wsjtxlink.h"

class BandInfo;
class ModeInfo;

class SyncRadio
{
private:
    bool master = false;
public:
    QString which;
    RigCache &rigCache;

    QString server;
    PubSubName selected;

    QString rigMode;
    QString lastRigMode;

    Frequency rigFreq;
    Frequency lastRigFreq;

    QSharedPointer<BandInfo>  lastBand;
    QSharedPointer<ModeInfo>  lastBandMode;
    int lastModePart = -1;

    bool trackThis = false;
    bool trackOther = false;
    bool trackBand = false;
    bool trackWSJTX = false;

    SyncRadio(const QString &w, RigCache &r);

    void setChoices(bool trthis, bool trother, bool tb, bool tw);

    void selectRadio(PubSubName name);
    void subRigSelection(const PubSubName &s, bool state);
    void controlFreq(const Frequency &freq, QString mode);
    void configureServer(const QString s);
    QStringList populateRig();
    bool check(N1MMLink &n1mmLink);
    void rigCentre(const Frequency &fLow, const Frequency &fHigh, const QString &mode);
    void trackOtherBand(SyncRadio &tracked);

    bool isMaster()const;
    void setMaster(bool m);
};

namespace Ui {
class RSMainWindow;
}
class RSMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit RSMainWindow(QWidget *parent = nullptr);
    ~RSMainWindow() override;

public slots:

private slots:
    void on_transfer12Button_clicked();
    void on_transfer21Button_clicked();
    void on_trackRig_clicked();
    void on_trackSub_clicked();
    void on_trackBandcb_stateChanged(int);
    void on_wsjtxCb_stateChanged(int arg1);
    void on_Rig2Combo_activated(const QString &arg1);

    void syncTimerTimer();

    void claimTimerTimer();

    void onStdInRead(QString);

    void on_routerCall(bool err, QSharedPointer<MinosRPCObj>mro, const QString from );
    void on_notify(AnalysePubSubNotify an, const QString from);

    void configure();
private:

    Ui::RSMainWindow *ui;

    QTimer claimTimer;
    RigCache rigCache;
    N1MMLink n1mmLink;
    WsjtxLink wsjtxLink;
    bool firstTime = true;

    StdInReader stdinReader;

    QTimer SyncTimer;

    QAction *configAction;

    SyncRadio mainRig = SyncRadio("Main", rigCache);
    SyncRadio subRig = SyncRadio("Sync", rigCache);

    virtual void closeEvent(QCloseEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void moveEvent(QMoveEvent *event) override;
    virtual void changeEvent( QEvent* e ) override;

    void getRouterAppCatMap();
};

#endif // RSMAINWINDOW_H
