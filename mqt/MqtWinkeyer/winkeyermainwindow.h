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
#include "winKeyerCommon.h"

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

private slots:
    void on_sendPushButton_clicked(); // Slot for send button click

    void on_openPushButton_clicked();
    void handleWinKeyerOpenStatus(bool open);
    void handleSetupPushButton();


    void onTextChanged(const QString &text);
private:
    Ui::WinkeyerMainWindow *ui;
    WinkeyerControl *winkeyerControl;
    //QSharedPointer<WinkeyerState> currentWinkeyerStatePtr;
    //QSharedPointer<WinkeyerStateStorage> currentWinkeyStateStoragePtr;

    //QSharedPointer<WinkeyerState> newWinkeyerStatePtr;
    //QSharedPointer<WinkeyerStateStorage> newWinkeyStateStoragePtr;



    void openWinKeyerSetupDialog();



    void handleKeyboardChar(QChar kbdChar);
};

#endif // WINKEYERMAINWINDOW_H
