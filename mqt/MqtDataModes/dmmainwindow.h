#ifndef DMMAINWINDOW_H
#define DMMAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

#include "StdInReader.h"

#include "MMVARIFrame.h"
#include "MMTTYFrame.h"
#include "FLDigiFrame.h"
#include "grittyframe.h"


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

    QAction *actionMMVARI;
    QAction *actionMMTTY;
    QAction *action2Tone;
    QAction *actionFLDigi;
    QAction *actionGritty;
    QAction *actionConfigure_Engines;

    QMenu *configMenu;
    QMenu *engineMenu;

    QTimer LogTimer;
    QString geoStr;         // geometry registry location

    MMVARIFrame *mmvariFrame = nullptr;
    MMTTYFrame *mmttyFrame = nullptr;
    FLDigiFrame *fldigiFrame = nullptr;
    GrittyFrame *grittyFrame = nullptr;

    void closeAllEngines();

    virtual void closeEvent(QCloseEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void moveEvent(QMoveEvent *event) override;
    virtual void changeEvent( QEvent* e ) override;

    void doCloseEvent();
    QMenu *newMenu(QMenu *m, const char *text);
    QAction *newAction(const char *text, QMenu *m, void (DMMainWindow::*slotparam)());
    void checkEnginesAvailable();
protected:
    virtual void showEvent(QShowEvent *) override;

private slots:
    void LogTimerTimer();

    void onStdInRead(QString cmd);

    void onActionMMVARI_triggered();

    void onActionMMTTY_triggered();

    void onAction2Tone_triggered();

    void onActionFLDigi_triggered();

    void onActionExit_triggered();

    void onActionConfigure_Engines_triggered();

    void onActionGritty_triggered();

    void on_sendButton_clicked();
};
#endif // DMMAINWINDOW_H
