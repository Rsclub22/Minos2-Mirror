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



#include <QTimer>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QSettings>
#include <QProcessEnvironment>
#include <QDebug>

#include "regsettings.h"
#include "AppStartup.h"
#include "MinosRPC.h"
#include "LogEvents.h"
#include "clustercommon.h"
#include "MTrace.h"
#include "txkeyerCommonConstants.h"

#include "pccwkeyermainwindow.h"
#include "qevent.h"
#include "ui_pccwkeyermainwindow.h"


const int STATUS_TIMER_DUR = 1000;

const int MIN_PRE_TX_DELAY = 10;
const int MAX_PRE_TX_DELAY = 100;
const int MIN_POST_TX_DELAY = 50;
const int MAX_POST_TX_DELAY = 750;

static const char blankString[] = QT_TRANSLATE_NOOP("SettingsDialog", "N/A");


PcCwKeyerMainWindow::PcCwKeyerMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::PcCwKeyerMainWindow)
{
    ui->setupUi(this);

    this->setWindowTitle("Serial DTR CW Keyer");

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    trace("Connect to commandRead");  // This connect doesn't appear to work for some time!
    connect(commandReader.data(), &CommandReader::commandLine, this, &PcCwKeyerMainWindow::onCommandRead);

    QString appName = getAppStartupName();
    trace(QString("AppName = %1").arg(appName));

    MinosRPC *rpc = MinosRPC::getMinosRPC(getAppStartupName());
    Q_UNUSED(rpc)

    createCloseEvent();

    RegSettings settings;
    QByteArray geometry = settings.getSettings().value("geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    //QString fileName = getDirectoryLocation(dlConfiguration) + "/PcCwKeyer.ini";
    //QSettings config(fileName, QSettings::IniFormat);

    connect(&LogTimer, &QTimer::timeout, this, &PcCwKeyerMainWindow::LogTimerTimer);
    LogTimer.start(100);


    fillPortsInfo();

    setWpmSpinnerRange(TxKeyerCommon::PC_CW_KEYER_MIN_WPM, TxKeyerCommon::PC_CW_KEYER_MAX_WPM);
    setWpmSpinnnerStep(1);
    setWpmValue(wpm);

    ui->preTxDelayLineEdit->setValidator(new QIntValidator(MIN_PRE_TX_DELAY, MAX_PRE_TX_DELAY, this));
    ui->postTxDelayLineEdit->setValidator(new QIntValidator(MIN_POST_TX_DELAY, MAX_POST_TX_DELAY, this));
    ui->preTxDelayLineEdit->setToolTip(QString("Min Delay %1, Max Delay %2").arg(QString::number(MIN_PRE_TX_DELAY), QString::number(MAX_PRE_TX_DELAY)));
    ui->postTxDelayLineEdit->setToolTip(QString("Min Delay %1, Max Delay %2").arg(QString::number(MIN_POST_TX_DELAY), QString::number(MAX_POST_TX_DELAY)));

    setConnections();

    comportName = new QLabel();
    comportStatus = new QLabel();
    errorMsg = new QLabel();


    ui->statusbar->addWidget(comportName);
    ui->statusbar->addWidget(comportStatus);
    ui->statusbar->addPermanentWidget(errorMsg);



    statusTimer = new QTimer(this);
    connect(statusTimer, &QTimer::timeout, this, &PcCwKeyerMainWindow::handleStatusTimer);
    statusTimer->start(STATUS_TIMER_DUR);

    readSettings();
    loadSettingsToMainWindow();



    pcCwKeyerRpc = new PcCwKeyerRpc();
    connect(pcCwKeyerRpc, &PcCwKeyerRpc::cwMessageFromLoggerToKeyer, this, &PcCwKeyerMainWindow::cwMessageFromLoggerToCwKeyer);
    connect(pcCwKeyerRpc, &PcCwKeyerRpc::cwStopCommandFromLogger, this, &PcCwKeyerMainWindow::cwStopCommandFromLogger);
    connect(pcCwKeyerRpc, &PcCwKeyerRpc::wpmFromLog, this, &PcCwKeyerMainWindow::cwWpmFromLogger);


    openCwKeyer();

    if (comport.isEmpty())
    {
        trace(QString("Open CWKeyer - Comport is empty"));
        errorMsg->setText("Comport is empty");
        comportName->clear();
        comportStatus->clear();

    }
    else
    {

       if (cwKeyer)
       {
         cwKeyer->openComPort(comport);
       }
    }


    QTimer::singleShot(30000, this, [this]() {
        sendInitialStatusToLogger();
    });






}

PcCwKeyerMainWindow::~PcCwKeyerMainWindow()
{
    delete ui;
}


void PcCwKeyerMainWindow::readSettings()
{
    QSettings config(PC_CW_KEYER_SETTINGS_FILE(), QSettings::IniFormat);

    wpm = config.value("currentWpm", 15).toInt();
    trace(QString("readSettings wpm = %1").arg(QString::number(wpm)));
    comport = config.value("comport", "").toString();
    trace(QString("readSettings comport = %1").arg(comport));
    preTxDelayMs = config.value("preTxDelayMs", MIN_PRE_TX_DELAY).toInt();
    trace(QString("readSettings preTxDelayMs = %1").arg(QString::number(preTxDelayMs)));
    postTxDelayMs = config.value("postTxDelayMs", MIN_POST_TX_DELAY).toInt();
    trace(QString("readSettings postTxDelayMs = %1").arg(QString::number(postTxDelayMs)));
    pttEnabled = config.value("pttEnabled", false).toBool();
    trace(QString("readSettings pttEnabled = %1").arg(pttEnabled ? "True" : "False"));
    invertDtrState = config.value("invertDtrState", false).toBool();
    trace(QString("readSettings invertDtrState = %1").arg(invertDtrState ? "True" : "False"));
    invertPttState = config.value("invertPttState", false).toBool();
    trace(QString("readSettings invertPttState = %1").arg(invertPttState ? "True" : "False"));


}

void PcCwKeyerMainWindow::loadSettingsToMainWindow()
{
    ui->wpmSpinBox->setValue(wpm);

    int index = ui->comportSel->findText(comport);
    if (index != -1)
    {
       ui->comportSel->setCurrentIndex(index);
    }
    else
    {
        trace(QString("Comport %1 is no longer available").arg(comport));
    }


    ui->preTxDelayValueDisplayLabel->setText(QString::number(preTxDelayMs));
    ui->postTxDelayValueLabel->setText(QString::number(postTxDelayMs));
    ui->enablePTTCheckbox->setChecked(pttEnabled);
    ui->invertPttLineCheckBox->setEnabled(pttEnabled);

    ui->invertDtrKeyerStateCheckBox->setChecked(invertDtrState);
    ui->invertPttLineCheckBox->setChecked(invertPttState);

    enablePTTObjects(pttEnabled);


}

void PcCwKeyerMainWindow::saveWpmSetting()
{
    QSettings config(PC_CW_KEYER_SETTINGS_FILE(), QSettings::IniFormat);

    config.setValue("currentWpm", wpm);
}

void PcCwKeyerMainWindow::saveComport()
{
    QSettings config(PC_CW_KEYER_SETTINGS_FILE(), QSettings::IniFormat);

    config.setValue("comport", comport);

}

void PcCwKeyerMainWindow::savePreTxDelay()
{
    QSettings config(PC_CW_KEYER_SETTINGS_FILE(), QSettings::IniFormat);

    config.setValue("preTxDelayMs", preTxDelayMs);
}

void PcCwKeyerMainWindow::savePostTxDelay()
{
    QSettings config(PC_CW_KEYER_SETTINGS_FILE(), QSettings::IniFormat);

    config.setValue("postTxDelayMs", postTxDelayMs);
}

void PcCwKeyerMainWindow::savePttEnabled()
{
    QSettings config(PC_CW_KEYER_SETTINGS_FILE(), QSettings::IniFormat);

    config.setValue("pttEnabled", pttEnabled);
}

void PcCwKeyerMainWindow::saveInvertedDtrStateFlag()
{
    QSettings config(PC_CW_KEYER_SETTINGS_FILE(), QSettings::IniFormat);
    config.setValue("invertDtrState", invertDtrState);
}

void PcCwKeyerMainWindow::saveInvertedPttStateFlag()
{
    QSettings config(PC_CW_KEYER_SETTINGS_FILE(), QSettings::IniFormat);
    config.setValue("invertPttState", invertPttState);
}

void PcCwKeyerMainWindow::saveAllSettings()
{
    saveWpmSetting();
    saveComport();
    savePreTxDelay();
    savePostTxDelay();
    savePttEnabled();
    saveInvertedDtrStateFlag();
    saveInvertedPttStateFlag();
}

void PcCwKeyerMainWindow::enablePTTObjects(bool enable)
{
    ui->invertPttLineCheckBox->setEnabled(enable);

    ui->preTxDelayLineEdit->setEnabled(enable);
    ui->preTxDelayTitleLabel->setEnabled(enable);
    ui->preTxDelayValueDisplayLabel->setEnabled(enable);

    ui->postTxDelayLineEdit->setEnabled(enable);
    ui->postTxDelayTitleLabel->setEnabled(enable);
    ui->postTxDelayValueLabel->setEnabled(enable);
}


void PcCwKeyerMainWindow::setConnections()
{

    connect(ui->comportSel, QOverload<int>::of(&QComboBox::activated), this, &PcCwKeyerMainWindow::onComportSelected);
    connect(ui->wpmSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &PcCwKeyerMainWindow::onWpmValueChanged);

    connect(ui->enablePTTCheckbox, &QCheckBox::clicked, this, &PcCwKeyerMainWindow::onEnablePTT);


    connect(ui->preTxDelayLineEdit, &QLineEdit::returnPressed, this, [this]() {
        QString text = ui->preTxDelayLineEdit->text();
        onPreTxDelayEditingFinished(text);
    });

    connect(ui->postTxDelayLineEdit, &QLineEdit::returnPressed, this, [this]() {
        QString text = ui->postTxDelayLineEdit->text();
        onPostTxDelayEditingFinished(text);
    });

    connect(ui->cwTextInputLineEdit, &QLineEdit::returnPressed, this, [this]() {
        QString text = ui->cwTextInputLineEdit->text();
        onTextInputFinished(text);
    });

    connect(ui->invertDtrKeyerStateCheckBox, &QCheckBox::clicked, this, &PcCwKeyerMainWindow::onInvertDtrStateClicked);
    connect(ui->invertPttLineCheckBox, &QCheckBox::clicked, this, &PcCwKeyerMainWindow::onInvertPttStateClicked);

}



void PcCwKeyerMainWindow::handleNextCwString()
{



    if (!cwMsgQueue.isEmpty())
    {
        QString next = cwMsgQueue.takeFirst().append(" "); // add space between messages
        trace(QString("handle next Cw string: %1").arg(next));
        if (pttEnabled)
        {
            cwKeyer->setPttPendingFlag(pttEnabled);
            cwKeyer->pttOn(true);  // Turn on PTT
            trace("Ptt Turned On");

            QTimer::singleShot(preTxDelayMs, this, [this, next]() {

                cwKeyer->sendText(next);
            });
        }
        else
        {
           cwKeyer->sendText(next);
        }

    }
    else
    {
        // no more messages
        sendTxStatusToLogger(false);
        setPttStatusIndicatorOnOff(false);


    }
}



void PcCwKeyerMainWindow::handleStatusTimer()
{
    static QString oldStatusMsg;
    static int oldServerListCount = 0;


    if (oldServerListCount != pcCwKeyerRpc->getServerListCount())
    {
        oldServerListCount = pcCwKeyerRpc->getServerListCount();
        // send status to clients
        pcCwKeyerRpc->publishState(comportName->text(), comportStatus->text(), errorMsg->text());

    }

   // send status message if it has changed
    else if (!comportStatus->text().isEmpty())
    {
        if (oldStatusMsg != comportStatus->text())
        {
            oldStatusMsg = comportStatus->text();

            // send status to clients
            trace(QString("handleStatusTimer: Send Status to PcCwKeyer Clients - %1").arg(ui->statusbar->currentMessage()));

            pcCwKeyerRpc->publishState(comportName->text(), comportStatus->text(), errorMsg->text());
        }
    }



}


void PcCwKeyerMainWindow::onComportSelected()
{
    if (ui->comportSel->currentText() != comport)
    {
        comport = ui->comportSel->currentText();
        trace(QString("comport changed = %1").arg(comport));
        saveComport();
        if (cwKeyer)
        {
            if (comport.isEmpty())
            {
                if (cwKeyer->isSerialOpen())
                {
                   cwKeyer->closeComport(comport);
                }

                trace(QString("OnComport Sel  - Comport is empty"));
                errorMsg->setText("Comport is empty");
                comportName->clear();
                comportStatus->clear();

            }
            else
            {
               cwKeyer->openComPort(comport);
            }

        }
    }
}




void PcCwKeyerMainWindow::onWpmValueChanged(int value)
{
    if (value != wpm)
    {
        wpm = getWpmValue();
        if (cwKeyer)
        {
            cwKeyer->setWPM(wpm);
        }
        trace(QString("wpm changed = %1").arg(QString::number(wpm)));
        saveWpmSetting();
        pcCwKeyerRpc->publishWpm(wpm);

    }
}




void PcCwKeyerMainWindow::onEnablePTT(bool checked)
{
    if (checked != pttEnabled)
    {
        pttEnabled = checked;
        ui->invertPttLineCheckBox->setEnabled(checked);
        savePttEnabled();
        enablePTTObjects(checked);
        pcCwKeyerRpc->publishPttEnable(checked);

    }
}

void PcCwKeyerMainWindow::onInvertDtrStateClicked(bool checked)
{
    if (checked != invertDtrState)
    {
        invertDtrState = checked;
        saveInvertedDtrStateFlag();
        cwKeyer->setInverKeyDownFlag(invertDtrState);
    }
}
void PcCwKeyerMainWindow::onInvertPttStateClicked(bool checked)
{
    if (checked != invertPttState)
    {
        invertPttState = checked;
        saveInvertedPttStateFlag();
        cwKeyer->setInverPttDownFlag(invertPttState);
    }
}


void PcCwKeyerMainWindow::openCwKeyer()
{
    trace(QString("Opening Cw Keyer"));
    cwKeyer = new PcCwKeyer(this);

    if (cwKeyer)
    {
        connect(cwKeyer, &PcCwKeyer::startTxMessage, this, [this]() {
            sendTxStatusToLogger(true);
            setPttStatusIndicatorOnOff(true);
        });

        connect(cwKeyer, &PcCwKeyer::nextStringRequested, this, &PcCwKeyerMainWindow::handleNextCwString);
        connect(cwKeyer, &PcCwKeyer::serialPortOpen, this, &PcCwKeyerMainWindow::handleSerialPortOpen);
        connect(cwKeyer, &PcCwKeyer::serialPortError, this, &PcCwKeyerMainWindow::handleSerialPortError);

        cwKeyer->setPostTxDelayMs(postTxDelayMs);
        cwKeyer->setWPM(wpm);
        cwKeyer->setInverKeyDownFlag(invertDtrState);
        cwKeyer->setInverPttDownFlag(invertPttState);
    }




}



void PcCwKeyerMainWindow::sendTxStatusToLogger(bool on)
{
    if (pcCwKeyerRpc)
    {
        QString txState;
        if (on)
        {
            txState = "On";
        }
        else
        {
            txState = "Off";
        }

        pcCwKeyerRpc->publishTxOn(txState);
    }
}


void PcCwKeyerMainWindow::sendInitialStatusToLogger()
{
    if (pcCwKeyerRpc)
    {
       pcCwKeyerRpc->publishPttEnable(ui->enablePTTCheckbox->isChecked());
       pcCwKeyerRpc->publishWpm(wpm);

    }

}






void PcCwKeyerMainWindow::sendPttStateToLogger()
{
    if (pcCwKeyerRpc)
    {

        trace(QString("Send pcCwKeyer PTT state - %1").arg(ui->enablePTTCheckbox->isChecked() ? "On" : "Off"));
        pcCwKeyerRpc->publishPttEnable(ui->enablePTTCheckbox->isChecked());
    }
}


void PcCwKeyerMainWindow::closeCwKeyer()
{
    trace(QString("Closing Cw Keyer"));
    delete cwKeyer;
}

void PcCwKeyerMainWindow::cwStopCommandFromLogger()
{
    trace(QString("Stop cw message from logger"));
    if (cwKeyer)
    {
        cwKeyer->abortTransmission();
    }

}

void PcCwKeyerMainWindow::cwWpmFromLogger(int newWpm)
{
    if (newWpm != wpm)
    {
        trace(QString("Cw WPM from logger = %1").arg(QString::number(newWpm)));
        ui->wpmSpinBox->blockSignals(true); // prevent update being sent back to logger
        ui->wpmSpinBox->setValue(newWpm);
        ui->wpmSpinBox->blockSignals(false);

        if (cwKeyer)
        {
            cwKeyer->setWPM(newWpm);
        }

        saveWpmSetting();
    }




}

void PcCwKeyerMainWindow::cwMessageFromLoggerToCwKeyer(QString message)
{
    trace(QString("cw message from logger = %1").arg(message));
    onTextInputFinished(message);
}

void PcCwKeyerMainWindow::onTextInputFinished(const QString &text)
{
    QString trimmed = text.trimmed();
    if (!trimmed.isEmpty())
    {
        bool wasEmpty = cwMsgQueue.isEmpty();
        if (!ui->cwTextInputLineEdit->text().isEmpty())
        {
          ui->cwTextInputLineEdit->selectAll();     // if text came from this app
        }
        trace(QString("Cw message from local CW Entry = %1").arg(trimmed));
        cwMsgQueue.append(trimmed);

        if (wasEmpty && cwKeyer)
        {
            handleNextCwString();  // start sending right away
        }
    }


}

void PcCwKeyerMainWindow::onPreTxDelayEditingFinished(QString text)
{
    bool ok = false;
    int delay = text.toInt(&ok);
    if (ok)
    {
        preTxDelayMs = delay;
        savePreTxDelay();
        ui->preTxDelayValueDisplayLabel->setText(text);
        ui->preTxDelayLineEdit->clear();
    }
}

void PcCwKeyerMainWindow::onPostTxDelayEditingFinished(QString text)
{
    bool ok = false;
    int delay = text.toInt(&ok);
    if (ok)
    {
        postTxDelayMs = delay;
        savePostTxDelay();
        ui->postTxDelayValueLabel->setText(text);
        ui->postTxDelayLineEdit->clear();
        if (cwKeyer)
        {
            cwKeyer->setPostTxDelayMs(postTxDelayMs);
        }
    }
}

void PcCwKeyerMainWindow::handleSerialPortOpen(bool state)
{
    comportName->clear();
    comportStatus->clear();
    errorMsg->clear();
    comportName->setText(QString("%1: ").arg(ui->comportSel->currentText()));
    QString stateStr = state ? "Open" : "Closed";
    comportStatus->setText(QString("%1").arg(stateStr));
    trace(QString("Comport State = %1").arg(stateStr));

}

void PcCwKeyerMainWindow::handleSerialPortError(QString msg)
{
    errorMsg->clear();
    errorMsg->setText(QString("%1").arg(msg));
    trace(QString("Serial Port Error = %1").arg(msg));
}




// should replace this with the common version in rigcommon, same in rotControl!

void PcCwKeyerMainWindow::fillPortsInfo()
{
    ui->comportSel->clear();
    ui->comportSel->addItem("");  // Add blank entry

    QString description;
    QString manufacturer;
    QString serialNumber;


    QList<QPair<QString, QStringList>> portEntries;

    // Collect all port info into a list
    const auto portInfo = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : portInfo)
    {
        QStringList list;
        description = info.description();
        manufacturer = info.manufacturer();
#if QT_VERSION > QT_VERSION_CHECK(5, 3, 0)
        serialNumber = info.serialNumber();
#endif
        list << info.portName()
             << (!description.isEmpty() ? description : blankString)
             << (!manufacturer.isEmpty() ? manufacturer : blankString)
             << (!serialNumber.isEmpty() ? serialNumber : blankString)
             << info.systemLocation()
             << (info.vendorIdentifier() ? QString::number(info.vendorIdentifier(), 16) : blankString)
             << (info.productIdentifier() ? QString::number(info.productIdentifier(), 16) : blankString);

        portEntries.append({info.portName(), list});
    }

    // Sort using numeric part of COM port name
    std::sort(portEntries.begin(), portEntries.end(), [](const QPair<QString, QStringList> &a, const QPair<QString, QStringList> &b) {
        static QRegularExpression re("COM(\\d+)");
        QRegularExpressionMatch ma = re.match(a.first);
        QRegularExpressionMatch mb = re.match(b.first);

        int na = ma.hasMatch() ? ma.captured(1).toInt() : 0;
        int nb = mb.hasMatch() ? mb.captured(1).toInt() : 0;

        return na < nb;
    });

    // Add sorted items to the combobox
    for (const auto &pair : portEntries)
    {
        ui->comportSel->addItem(pair.first, pair.second);
    }
}




void PcCwKeyerMainWindow::setWpmSpinnerRange(int minValue, int maxValue)
{
    ui->wpmSpinBox->setRange(minValue, maxValue);
}



void PcCwKeyerMainWindow::setWpmSpinnnerStep(int step)
{
    ui->wpmSpinBox->setSingleStep(step);
}

void PcCwKeyerMainWindow::setWpmValue(int value)
{
    ui->wpmSpinBox->setValue(value);
}

int PcCwKeyerMainWindow::getWpmValue() const
{
    return ui->wpmSpinBox->value();
}


void PcCwKeyerMainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
    {
        qDebug() << "Esc key pressed";
        event->accept(); // mark it as handled
        if (cwKeyer)
        {
            cwMsgQueue.clear();
            cwKeyer->abortTransmission();
        }

    }
    else
    {
        QWidget::keyPressEvent(event); // Pass to base class
    }
}

void PcCwKeyerMainWindow::closeEvent(QCloseEvent *event)
{
    trace("pcCwKeyerMainWindow::closeEvent");

    RegSettings settings;
    settings.getSettings().setValue("geometry", saveGeometry());

    if (cwKeyer)
    {
        cwMsgQueue.clear();
        cwKeyer->abortTransmission();  // stop any CW sending
        cwKeyer->close();       // close the serial port
    }

    QWidget::closeEvent(event);  // allow the window to close
}

void PcCwKeyerMainWindow::onCommandRead(QString cmd)
{
    trace(QString("onCommandRead %1").arg(cmd));
    if (cmd.indexOf("Shutdown", 0, Qt::CaseInsensitive) >= 0)
    {
        close();
    }
}

void PcCwKeyerMainWindow::LogTimerTimer()
{
    static bool closed = false;
    if ( !closed )
    {
        if ( checkCloseEvent() )
        {
            trace("close event seen");
            closed = true;
            close();
        }
    }
}

void PcCwKeyerMainWindow::setTXStatusVisible(bool visible)
{
    ui->txStatusIndicator->setVisible(visible);
    ui->txStatusIndicatorLabel->setVisible(visible);
}

void PcCwKeyerMainWindow::setPttStatusIndicatorOnOff(bool on)
{
    if (on)
    {
        ui->txStatusIndicator->setStyleSheet(STATUS_INDICATOR_CONNECT_STYLE);
        ui->txStatusIndicator->setToolTip(tr("TX On"));

    }
    else
    {
        ui->txStatusIndicator->setStyleSheet(STATUS_INDICATOR_DISCONNECT_STYLE);
        ui->txStatusIndicator->setToolTip(tr("TX Off"));
    }

}

QString PC_CW_KEYER_SETTINGS_FILE()
{
    return getDirectoryLocation(dlConfiguration) + "/PcCwKeyer/PcCwKeyerSettings.ini";
}

