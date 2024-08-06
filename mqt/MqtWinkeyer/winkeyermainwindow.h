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
    void handleWinKeyerOpenStatus(bool open);
    void handleSetupPushButton();


    void onTextChanged(const QString &text);
    void onCommandRead(QString cmd);
private:
    Ui::WinkeyerMainWindow *ui;
    WinkeyerControl *winkeyerControl;

    QSharedPointer<CommandReader> commandReader = QSharedPointer<CommandReader>(new CommandReader(this));

    QTimer LogTimer;

    void closeEvent(QCloseEvent *event) override;

    void openWinKeyerSetupDialog();



    void handleKeyboardChar(QChar kbdChar);
    void LogTimerTimer();
    void saveWinkeyerSettings();
    void updateStatusBarMessage(QString serialErrorMsg, QString wkStatusMsg);
};

#endif // WINKEYERMAINWINDOW_H
