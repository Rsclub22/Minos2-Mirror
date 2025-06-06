/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Pc Serial Port DTR CW Keyer
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2025
//
// Interprocess Control Logic
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2017
//
//
//
/////////////////////////////////////////////////////////////////////////////




#ifndef PCCWKEYERMAINWINDOW_H
#define PCCWKEYERMAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QXmlStreamReader>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "pccwkeyer.h"
#include "CommandReader.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class pcCwKeyerMainWindow;
}
QT_END_NAMESPACE

class pcCwKeyerMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    pcCwKeyerMainWindow(QWidget *parent = nullptr);
    ~pcCwKeyerMainWindow();

private:
    Ui::pcCwKeyerMainWindow *ui;

    QSharedPointer<CommandReader> commandReader = QSharedPointer<CommandReader>(new CommandReader(this));

    QTimer LogTimer;

    PcCwKeyer *cwKeyer = nullptr;
    //QString pendingBuffer;

    QList<QString> cwMsgQueue;


    QString comport;
    int wpm = 15;

    int farnsworth = -1;        // disable farnsworth

    bool sideToneOn = false;

    bool dtrRtsSelected = true; // true = dtr, false = rts




    void fillPortsInfo();

    void setWpmSpinnerRange(int minValue, int maxValue);
    void setWpmSpinnnerStep(int step);

    void setWpmValue(int value);
    int getWpmValue() const;
    void setConnections();
    void openCwKeyer();

    void closeCwKeyer();
    void keyPressEvent(QKeyEvent *event);
    void closeEvent(QCloseEvent *event);
private slots:
    //void onTextEdited(const QString &text);
    //void checkCWBuffer();
    void onComportSelected();
    void onDtrSelected();
    void onSidetoneChkBoxSelected();
    void onRtsSelected();
    void onWpmValueChanged(int value);
    void onTextInputFinished(const QString &text);
    void handleNextCwString();
    void handleSerialPortOpen(bool state);
    void handleSerialPortError(QString errorMsg);
    void onCommandRead(QString cmd);
    void LogTimerTimer();
};
#endif // PCCWKEYERMAINWINDOW_H

