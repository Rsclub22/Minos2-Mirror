#ifndef RSMAINWINDOW_H
#define RSMAINWINDOW_H

#include "base_pch.h"
#include <QComboBox>
#include "RigCache.h"
#include "n1mmlink.h"
#include "wsjtxlink.h"

namespace Ui {
class RSMainWindow;
}
class BandInfo;
class ModeInfo;
class RSMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit RSMainWindow(QWidget *parent = nullptr);
    ~RSMainWindow() override;

    void selectRadio(PubSubName name);

public slots:

private slots:
    void on_closeButton_clicked();
    void on_transfer12Button_clicked();
    void on_transfer21Button_clicked();

    void SyncTimerTimer();

    void onStdInRead(QString);

    void on_routerCall(bool err, QSharedPointer<MinosRPCObj>mro, const QString from );
    void on_notify(AnalysePubSubNotify an, const QString from);

    void on_noTrack_clicked();
    void on_trackRig_clicked();
    void on_trackSub_clicked();
    void on_trackBandcb_stateChanged(int);
    void on_wsjtxCb_stateChanged(int arg1);

    void on_Rig2Combo_activated(const QString &arg1);

    void configure();
private:
    Ui::RSMainWindow *ui;
    RigCache rigCache;
    N1MMLink n1mmLink;
    WsjtxLink wsjtxLink;

    virtual void closeEvent(QCloseEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void moveEvent(QMoveEvent *event) override;
    virtual void changeEvent( QEvent* e ) override;

    StdInReader stdinReader;

    QTimer SyncTimer;

    const QString rigSyncUuid = "RigSync";

    QAction *configAction;

    QString mainRigMode;
    QString lastMainRigMode;

    Frequency mainRigFreq;
    Frequency lastMainRigFreq;

    QSharedPointer<BandInfo>  lastBand;
    QSharedPointer<ModeInfo>  lastBandMode;
    int lastModePart = -1;

    PubSubName mainRigSelected;

    QString subServer;
    PubSubName subRigSelected;

    QString subRigMode;
    Frequency subRigFreq;

    void trackBand();
    void rig2Centre(const Frequency &fLow, const Frequency &fHigh);
    void getRouterAppCatMap();
    QStringList rigs();
    QStringList populateRig2();
    void subRigSelection(const PubSubName &s, bool state);
    void subRigControlFreq(const Frequency &freq, QString mode);
    void mainRigControlFreq(const Frequency &lFreq, QString mode);
};

#endif // RSMAINWINDOW_H
