/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Winkeyer Control
//
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2024
//
// Interprocess Control Logic
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2017
//
//
//
/////////////////////////////////////////////////////////////////////////////
#ifndef WINKEYERMAINWINDOW_H
#define WINKEYERMAINWINDOW_H

#include <QMainWindow>
#include <QSharedPointer>
#include <QLabel>
#include "winkeyerControl.h"
#include "CommandReader.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class WinkeyerMainWindow;
}
QT_END_NAMESPACE

class WinkeyerMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    WinkeyerMainWindow(QWidget *parent = nullptr);
    ~WinkeyerMainWindow();

    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void moveEvent(QMoveEvent *event) override;
    virtual void changeEvent( QEvent* e ) override;

private slots:
    void on_sendPushButton_clicked(); // Slot for send button click

    void on_openPushButton_clicked();
    void onHandleWinKeyerOpenStatus(bool open);
    void onHandleSetupPushButton();


    void onTextChanged(const QString &text);
    void onCommandRead(QString cmd);
    void onHandleXoffStatus(QString status);
    void onHandleBreakInStatus(QString status);
    void onHandleKBusyStatus(QString status);
    void onHandleKWaitStatus(QString status);
private:
    Ui::WinkeyerMainWindow *ui;
    WinkeyerControl *winkeyerControl;

    QSharedPointer<CommandReader> commandReader = QSharedPointer<CommandReader>(new CommandReader(this));

    QTimer LogTimer;

    QLabel *xoffStatus;
    QLabel *breakInStatus;
    QLabel *busyStatus;
    QLabel *waitStatus;


    void closeEvent(QCloseEvent *event) override;

    void openWinKeyerSetupDialog();



    void handleKeyboardChar(QChar kbdChar);
    void LogTimerTimer();
    void saveWinkeyerSettings(QSharedPointer<WinkeyerStateStorage> winkeySettings);
    void updateStatusBarMessage(QString serialErrorMsg, QString wkStatusMsg);
    void addWinkeyerStatusObjectsToStatusBar();
};

#endif // WINKEYERMAINWINDOW_H
