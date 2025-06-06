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
#include "RPCCommandConstants.h"
#include "LogEvents.h"
#include "MTrace.h"

#include "pccwkeyermainwindow.h"
#include "qevent.h"
#include "ui_pccwkeyermainwindow.h"




static const char blankString[] = QT_TRANSLATE_NOOP("SettingsDialog", "N/A");


pcCwKeyerMainWindow::pcCwKeyerMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::pcCwKeyerMainWindow)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    trace("Connect to commandRead");  // This connect doesn't appear to work for some time!
    connect(commandReader.data(), &CommandReader::commandLine, this, &pcCwKeyerMainWindow::onCommandRead);

    RegSettings settings;
    QByteArray geometry = settings.getSettings().value("geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    QString fileName = getDirectoryLocation(dlConfiguration) + "/PcCwKeyer.ini";
    QSettings config(fileName, QSettings::IniFormat);


    createCloseEvent();

    connect(&LogTimer, &QTimer::timeout, this, &pcCwKeyerMainWindow::LogTimerTimer);
    LogTimer.start(100);

    QString appName = getAppStartupName();
    trace(QString("AppName = %1").arg(appName));
    MinosRPC *rpc = MinosRPC::getMinosRPC(appName);
    Q_UNUSED(rpc)

    fillPortsInfo();

    ui->dtrRb->setChecked(true);
    ui->rtsRb->setChecked(false);
    ui->sidetoneChkBox->setChecked(false);


    setWpmSpinnerRange(5, 40);
    setWpmSpinnnerStep(1);
    setWpmValue(wpm);

    setConnections();



}

pcCwKeyerMainWindow::~pcCwKeyerMainWindow()
{
    delete ui;
}


void pcCwKeyerMainWindow::setConnections()
{

    connect(ui->comportSel, QOverload<int>::of(&QComboBox::activated), this, &pcCwKeyerMainWindow::onComportSelected);
    connect(ui->dtrRb, &QRadioButton::clicked,this, &pcCwKeyerMainWindow::onDtrSelected);
    connect(ui->rtsRb, &QRadioButton::clicked,this, &pcCwKeyerMainWindow::onRtsSelected);
    connect(ui->sidetoneChkBox, &QCheckBox::clicked,this, &pcCwKeyerMainWindow::onSidetoneChkBoxSelected);
    connect(ui->wpmSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &pcCwKeyerMainWindow::onWpmValueChanged);

    connect(ui->cwTextInputLineEdit, &QLineEdit::returnPressed, this, [this]() {
        QString text = ui->cwTextInputLineEdit->text();
        onTextInputFinished(text);
        ui->cwTextInputLineEdit->clear();  // clear after sending
    });




}

void pcCwKeyerMainWindow::handleNextCwString()
{
    if (!cwMsgQueue.isEmpty())
    {
        QString next = cwMsgQueue.takeFirst().append(" "); // add space between messages
        cwKeyer->sendText(next);
    }
}


void pcCwKeyerMainWindow::onComportSelected()
{
    if (ui->comportSel->currentText() != comport)
    {
        comport = ui->comportSel->currentText();
        openCwKeyer();
    }
}

void pcCwKeyerMainWindow::onDtrSelected()
{
    if (!dtrRtsSelected)
    {
        dtrRtsSelected = !dtrRtsSelected;
        ui->dtrRb->setChecked(true);
        ui->rtsRb->setChecked(false);
    }
}

void pcCwKeyerMainWindow::onRtsSelected()
{
    if (dtrRtsSelected)
    {
        dtrRtsSelected = !dtrRtsSelected;
        ui->dtrRb->setChecked(false);
        ui->rtsRb->setChecked(true);
    }
}



void pcCwKeyerMainWindow::onSidetoneChkBoxSelected()
{
    if (ui->sidetoneChkBox->isChecked() != sideToneOn)
    {
        sideToneOn = !sideToneOn;
        if (cwKeyer)
        {
            cwKeyer->setUseSideTone(sideToneOn);
        }
    }

}

void pcCwKeyerMainWindow::onWpmValueChanged(int value)
{
    if (value != wpm)
    {
        wpm = getWpmValue();
        openCwKeyer();
    }
}


void pcCwKeyerMainWindow::openCwKeyer()
{
    if (cwKeyer)
    {
        disconnect(cwKeyer, &PcCwKeyer::nextStringRequested, this, &pcCwKeyerMainWindow::handleNextCwString);
        delete cwKeyer;
    }

    if (comport.isEmpty())
    {
        qDebug() << "Comport is empty";
    }

    cwKeyer = new PcCwKeyer(wpm, farnsworth, sideToneOn, dtrRtsSelected, this);

    if (cwKeyer)
    {
        connect(cwKeyer, &PcCwKeyer::nextStringRequested, this, &pcCwKeyerMainWindow::handleNextCwString);
        connect(cwKeyer, &PcCwKeyer::serialPortOpen, this, &pcCwKeyerMainWindow::handleSerialPortOpen);
        connect(cwKeyer, &PcCwKeyer::serialPortError, this, &pcCwKeyerMainWindow::handleSerialPortError);

        cwKeyer->openComPort(comport);
    }




}

void pcCwKeyerMainWindow::closeCwKeyer()
{
    delete cwKeyer;
}



void pcCwKeyerMainWindow::onTextInputFinished(const QString &text)
{
    if (!text.trimmed().isEmpty())
        cwMsgQueue.append(text.trimmed());

    // If keyer is not currently busy, kick it off
    if (!cwKeyer->isBusy())
    {
        handleNextCwString();
    }

}

void pcCwKeyerMainWindow::handleSerialPortOpen(bool state)
{
    ui->statusbar->clearMessage();
    ui->statusbar->showMessage(QString("%1 %2").arg(ui->comportSel->currentText(), state ? "Open" : "Closed"));

}

void pcCwKeyerMainWindow::handleSerialPortError(QString errorMsg)
{
    ui->statusbar->clearMessage();
    ui->statusbar->showMessage(QString("%1 %2").arg(ui->comportSel->currentText(), errorMsg));
}




// should replace this with the common version in rigcommon, same in rotControl!

void pcCwKeyerMainWindow::fillPortsInfo()
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




void pcCwKeyerMainWindow::setWpmSpinnerRange(int minValue, int maxValue)
{
    ui->wpmSpinBox->setRange(minValue, maxValue);
}



void pcCwKeyerMainWindow::setWpmSpinnnerStep(int step)
{
    ui->wpmSpinBox->setSingleStep(step);
}

void pcCwKeyerMainWindow::setWpmValue(int value)
{
    ui->wpmSpinBox->setValue(value);
}

int pcCwKeyerMainWindow::getWpmValue() const
{
    return ui->wpmSpinBox->value();
}


void pcCwKeyerMainWindow::keyPressEvent(QKeyEvent *event)
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

void pcCwKeyerMainWindow::closeEvent(QCloseEvent *event)
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

void pcCwKeyerMainWindow::onCommandRead(QString cmd)
{
    trace(QString("onCommandRead %1").arg(cmd));
    if (cmd.indexOf("Shutdown", 0, Qt::CaseInsensitive) >= 0)
    {
        close();
    }
}

void pcCwKeyerMainWindow::LogTimerTimer()
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



