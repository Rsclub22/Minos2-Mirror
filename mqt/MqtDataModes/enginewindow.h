#ifndef ENGINEWINDOW_H
#define ENGINEWINDOW_H

#include "RigCache.h"
#include "rxbuffer.h"
#include <QDialog>
#include <QTimer>

#ifdef Q_OS_WIN
#include "MMVARIFrame.h"
#include "MMTTYFrame.h"
#include "grittyframe.h"
#endif
#include "FLDigiFrame.h"
#include "testframe.h"

QT_BEGIN_NAMESPACE
namespace Ui { class EngineWindow; }
QT_END_NAMESPACE

class QPushButton;

class EngineWindow : public QDialog
{
    Q_OBJECT

public:
    explicit EngineWindow(QWidget *parent = nullptr);
    virtual ~EngineWindow() override;

    RxBuffer rxBuff;
    QString engineName;

    static const QString mmvari;
    static const QString mmtty;
    static const QString twotone;
    static const QString gritty;
    static const QString fldigi;
    static const QString test;
    static const QString i1;
    static const QString i2;

    static const QStringList enginesList;

    void selectEngine(QString name);

    void selectMMVARI(QString name);
    void selectMMTTY(QString name);
    void selectTest(QString name);
    void selectFLDigi(QString name);
    void select2Tone(QString name);
    void selectGritty(QString name);
    void clear();

    void doSendCharacters(QString d, int c);
private:
    Ui::EngineWindow *ui;

    QString baseTitle;

    QString router;
    QString appName;
    RigCache rigCache;
    PubSubName mainRig;

    unsigned int inChannels = 0;
    unsigned int outChannels = 0;

    QStringList inputDevices;
    QStringList outputDevices;

    QMap<QString, int> inDeviceIds;
    QMap<QString, int> outDeviceIds;

    QVector<QPushButton *> fButtons;

    QTimer *watchDog = nullptr;

    QTimer LogTimer;
    QString geoStr;         // geometry registry location

#ifdef Q_OS_WIN
    MMVARIFrame *mmvariFrame = nullptr;
    MMTTYFrame *mmttyFrame = nullptr;
    GrittyFrame *grittyFrame = nullptr;
#endif
    FLDigiFrame *fldigiFrame = nullptr;
    TestFrame *testFrame = nullptr;

    void closeAllEngines();

    virtual void closeEvent(QCloseEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void moveEvent(QMoveEvent *event) override;
    virtual void changeEvent( QEvent* e ) override;

    void doCloseEvent();

    virtual bool eventFilter(QObject *obj, QEvent *event) override;

    bool doKeyPressEvent(QKeyEvent *event);
    QStringList populateRig();
    void doSendButton_clicked(QString d, int c);

signals:
    void rigModeFreq(QString, Frequency);
    void sendCharactersDown(QString, int);
    void sendCharactersUp(QString, int);

public slots:
    void onTxChanged(bool);

private slots:
    void on_notify(AnalysePubSubNotify an, const QString from);

    void onNewBackLine(QString);

    void on_sendButton_clicked();

    void onNewCharacter();

    void wordSelected(QString, int carrier);

    void fButtonClicked();

    void fKey(int key);

    void on_stopButton_clicked();
    void on_backDataButton_clicked();

    void onWatchdogTimer();

};
#endif // ENGINEWINDOW_H
