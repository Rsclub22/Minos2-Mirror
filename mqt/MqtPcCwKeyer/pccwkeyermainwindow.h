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
#include <QLabel>
#include "pccwkeyer.h"
#include "CommandReader.h"
#include "pccwkeyerrpc.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class PcCwKeyerMainWindow;
}
QT_END_NAMESPACE

QString PC_CW_KEYER_SETTINGS_FILE();

class PcCwKeyerMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    PcCwKeyerMainWindow(QWidget *parent = nullptr);
    ~PcCwKeyerMainWindow();

signals:

    void requestPtt(bool);

private:
    Ui::PcCwKeyerMainWindow *ui;

    QSharedPointer<CommandReader> commandReader = QSharedPointer<CommandReader>(new CommandReader(this));

    QTimer LogTimer;
    //QTimer *keyerStatusTimer;

    PcCwKeyer *cwKeyer = nullptr;


    QList<QString> cwMsgQueue;


    QString comport;
    int wpm = 15;

    int preTxDelayMs = 0;
    int postTxDelayMs = 0;

    bool pttEnabled = false;

    QTimer* statusTimer;

    PcCwKeyerRpc* pcCwKeyerRpc;

    QMetaObject::Connection cwKeyerFinishedConnection;

    QLabel *comportName;
    QLabel *comportStatus;
    QLabel *errorMsg;





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
    void readSettings();
    void enableTXDelayObjects(bool enable);
    void saveAllSettings();
    void saveWpmSetting();
    void savePreTxDelay();
    void savePostTxDelay();
    void savePttEnabled();
    void saveComport();
    void loadSettingsToMainWindow();





    void sendTxStatusToLogger(bool on);
    void sendPttStateToLogger();
    void setTXStatusVisible(bool visible);
    void setPttStatusIndicatorOnOff(bool on);
    void sendInitialPttStatusToLogger();
private slots:

    void onComportSelected();


    void onWpmValueChanged(int value);
    void onTextInputFinished(const QString &text);
    void handleNextCwString();
    void handleSerialPortOpen(bool state);
    void handleSerialPortError(QString errorMsg);
    void onCommandRead(QString cmd);
    void LogTimerTimer();
    void handleStatusTimer();
    void onPreTxDelayEditingFinished(QString text);
    void onPostTxDelayEditingFinished(QString text);
    void onEnablePTT(bool checked);
    void cwMessageFromLoggerToCwKeyer(QString message);
    void cwStopCommandFromLogger();


};
#endif // PCCWKEYERMAINWINDOW_H

