#ifndef DMMAINWINDOW_H
#define DMMAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

#include "StdInReader.h"

#ifdef Q_OS_WIN
#include "MMVARIFrame.h"
#include "MMTTYFrame.h"
#include "grittyframe.h"
#endif
#include "FLDigiFrame.h"
#include "testframe.h"

QT_BEGIN_NAMESPACE
namespace Ui { class DMMainWindow; }
QT_END_NAMESPACE

class RtAudio;
class DMMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    DMMainWindow(QWidget *parent = nullptr);
    virtual ~DMMainWindow() override;


    unsigned int inChannels = 0;
    unsigned int outChannels = 0;

    QStringList inputDevices;
    QStringList outputDevices;

    QMap<QString, int> deviceIds;

private:
    Ui::DMMainWindow *ui;
    StdInReader *stdinReader = new StdInReader(this);

    QMap<QAction *, const char *> actionList;
    QMap<QMenu *, const char *> menuList;
    QAction *clearAction;

#ifdef Q_OS_WIN
    QAction *actionMMVARI;
    QAction *actionMMTTY;
    QAction *action2Tone;
    QAction *actionGritty;
#endif
    QAction *actionFLDigi;
    QAction *actionTest;

    QAction *actionConfigure_Engines;

    QMenu *configMenu;
    QMenu *engineMenu;

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
    QMenu *newMenu(QMenu *m, const char *text);
    QAction *newAction(const char *text, QMenu *m, void (DMMainWindow::*slotparam)());
    QAction *newCheckableAction(const char *text, QMenu *m, void (DMMainWindow::*slotparam)(bool));
    QAction *newCheckableAction(const QString text, QMenu *m, void (DMMainWindow::*slotparam)(bool));
    void checkEnginesAvailable();
protected:
    virtual void showEvent(QShowEvent *) override;

private slots:
    void LogTimerTimer();

    void onStdInRead(QString cmd);

#ifdef Q_OS_WIN

    void onActionMMVARI_triggered(bool checked);

    void onActionMMTTY_triggered(bool checked);

    void onAction2Tone_triggered(bool checked);

    void onActionGritty_triggered(bool checked);
#endif

    void onActionFLDigi_triggered(bool checked);

    void onActionTest_triggered(bool checked);

    void onActionExit_triggered();

    void onActionConfigure_Engines_triggered();

    void on_sendButton_clicked();

    void onNewCharacter();

    void wordSelected(QString);

    void onMenuClear();
};
#endif // DMMAINWINDOW_H
