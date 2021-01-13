#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "base_pch.h"
#include "RigCache.h"
#include "n1mmlink.h"
#include "wsjtxlink.h"

namespace Ui {
class MainWindow;
}
class BandInfo;
class ModeInfo;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

public slots:

private slots:
    void on_closeButton_clicked();
    void on_transfer12Button_clicked();
    void on_transfer21Button_clicked();

    void SyncTimerTimer();
    void timer2Timeout();

    void onStdInRead(QString);

    void onSocketConnect();
    void onSocketDisconnect();
    void onError(QAbstractSocket::SocketError);
    void onReadyRead();

    void on_serverCall( bool err, QSharedPointer<MinosRPCObj>mro, const QString &from );

    void on_noTrack_clicked();

    void on_trackRig_clicked();

    void on_trackQS1R_clicked();

    void on_notify(bool err, QSharedPointer<MinosRPCObj> mro, const QString &from);
    void on_trackBandcb_stateChanged(int);

    void on_wsjtxCb_stateChanged(int arg1);

private:
    Ui::MainWindow *ui;
    RigCache rigCache;
    N1MMLink n1mmLink;
    WsjtxLink wsjtxLink;

    virtual void closeEvent(QCloseEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void moveEvent(QMoveEvent *event) override;
    virtual void changeEvent( QEvent* e ) override;

    StdInReader stdinReader;

    QTimer SyncTimer;
    QTimer timer2;

    QTcpSocket ClientSocket1;

    bool qs1rConnected = false;

    bool muted = false;
    QString lastQS1RRx;
    int fCentre = 0.0;
    int ftf = 0;
    int sampleRate = 0;

    QString state;
    QString mainRigMode;
    QString lastMainRigMode;

    Frequency mainRigFreq;
    Frequency lastMainRigFreq;
    Frequency lastTransverterOffset;

    QSharedPointer<BandInfo>  lastBand;
    QSharedPointer<ModeInfo>  lastBandMode;
    int lastModePart = -1;

    PubSubName rigSelected;

    bool transvertState = false;
    Frequency transvertOffset;

    void trackBand();
    void QS1RCentre(const Frequency &fLow, const Frequency &fHigh);
};

#endif // MAINWINDOW_H
