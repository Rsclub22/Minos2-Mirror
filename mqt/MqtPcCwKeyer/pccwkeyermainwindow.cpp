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
#include "MTrace.h"

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

    setWpmSpinnerRange(5, 40);
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

    // give time for logger to come up
    QTimer::singleShot(30000, this, [this]() {
        sendPttStateToLogger();
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
    comport = config.value("comport", "").toString();
    preTxDelayMs = config.value("preTxDelayMs", MIN_PRE_TX_DELAY).toInt();
    postTxDelayMs = config.value("postTxDelayMs", MIN_POST_TX_DELAY).toInt();
    pttEnabled = config.value("pttEnabled", false).toBool();

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
        qDebug() << "Comport" << comport << " is no longer available";
    }


    ui->preTxDelayValueDisplayLabel->setText(QString::number(preTxDelayMs));
    ui->postTxDelayValueLabel->setText(QString::number(postTxDelayMs));
    ui->enablePTTCheckbox->setChecked(pttEnabled);

    enableTXDelayObjects(pttEnabled);


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

void PcCwKeyerMainWindow::saveAllSettings()
{
    saveWpmSetting();
    saveComport();
    savePreTxDelay();
    savePostTxDelay();
    savePttEnabled();
}

void PcCwKeyerMainWindow::enableTXDelayObjects(bool enable)
{
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
        pcCwKeyerRpc->publishTxOn("Off");
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
        //trace(QString("handleStatusTimer: PcCwKeyer Client Count Changed old = %1, new = %2 - Send Status to Cluster Clients - %3").arg(oldServerListCount).arg(pcCwKeyerRpc->getServerListCount()).arg(status->text()));
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
    }
}


void PcCwKeyerMainWindow::onEnablePTT(bool checked)
{
    if (checked != pttEnabled)
    {
        pttEnabled = checked;
        savePttEnabled();
        enableTXDelayObjects(checked);
        pcCwKeyerRpc->publishPttEnable(checked);

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
        });

        connect(cwKeyer, &PcCwKeyer::nextStringRequested, this, &PcCwKeyerMainWindow::handleNextCwString);
        connect(cwKeyer, &PcCwKeyer::serialPortOpen, this, &PcCwKeyerMainWindow::handleSerialPortOpen);
        connect(cwKeyer, &PcCwKeyer::serialPortError, this, &PcCwKeyerMainWindow::handleSerialPortError);

        cwKeyer->setPostTxDelayMs(postTxDelayMs);
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

void PcCwKeyerMainWindow::sendPttStateToLogger()
{
    if (pcCwKeyerRpc)
    {

        QString("Send pcCwKeyer PTT state");
        pcCwKeyerRpc->publishPttEnable(pttEnabled);
    }
}


void PcCwKeyerMainWindow::closeCwKeyer()
{
    trace(QString("Closing Cw Keyer"));
    delete cwKeyer;
}

void PcCwKeyerMainWindow::cwStopCommandFromLogger()
{
    trace(QString("stop cw message from logger"));
    if (cwKeyer)
    {
        cwKeyer->abortTransmission();
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
    comportStatus->setText(QString("%1").arg(state ? "Open" : "Closed"));


}

void PcCwKeyerMainWindow::handleSerialPortError(QString msg)
{
    errorMsg->clear();
    errorMsg->setText(QString("%1").arg(msg));
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

    event->accept();  // allow the window to close
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

QString PC_CW_KEYER_SETTINGS_FILE()
{
    return getDirectoryLocation(dlConfiguration) + "/PcCwKeyer/PcCwKeyerSettings.ini";
}

