#include "pccwkeyermainwindow.h"
#include "qevent.h"
#include "ui_pccwkeyermainwindow.h"

#include <QVBoxLayout>
#include <QTimer>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QDebug>



static const char blankString[] = QT_TRANSLATE_NOOP("SettingsDialog", "N/A");


pcCwKeyerMainWindow::pcCwKeyerMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::pcCwKeyerMainWindow)
{
    ui->setupUi(this);

    fillPortsInfo();

    ui->dtrRb->setChecked(true);
    ui->rtsRb->setChecked(false);
    ui->sidetoneChkBox->setChecked(false);


    setWpmSpinnerRange(5, 40);
    setWpmSpinnnerStep(1);
    setWpmValue(wpm);

    setConnections();


    // Timer to feed CW keyer every 100ms
    auto *bufferTimer = new QTimer(this);
    connect(bufferTimer, &QTimer::timeout, this, &pcCwKeyerMainWindow::checkCWBuffer);
    bufferTimer->start(100);
}

pcCwKeyerMainWindow::~pcCwKeyerMainWindow()
{
    delete ui;
}


void pcCwKeyerMainWindow::setConnections()
{

    //connect(ui->cwTextInputLineEdit, &QLineEdit::textEdited, this, &pcCwKeyerMainWindow::onTextEdited);
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
        delete cwKeyer;
    }

    if (comport.isEmpty())
    {
        qDebug() << "Comport is empty";
    }

    cwKeyer = new PcCwKeyer(comport, wpm, farnsworth, sideToneOn, dtrRtsSelected, this);


}

void pcCwKeyerMainWindow::closeCwKeyer()
{
    delete cwKeyer;
}

/*
void pcCwKeyerMainWindow::onTextEdited(const QString &text) {
    static int lastLen = 0;
    if (text.length() > lastLen) {
        QString added = text.mid(lastLen);
        pendingBuffer += added;
    } else if (text.length() < lastLen) {
        // Handle backspace or clear (optional logic)
    }
    lastLen = text.length();
}
*/

void pcCwKeyerMainWindow::onTextInputFinished(const QString &text)
{
    pendingBuffer += text;

}

void pcCwKeyerMainWindow::checkCWBuffer() {

    if (cwKeyer)
    {
        if (!pendingBuffer.isEmpty() && !cwKeyer->isBusy()) {
            QChar next = pendingBuffer[0];
            pendingBuffer.remove(0, 1);
            cwKeyer->sendText(QString(next));
        }
    }

}

// should replace this with the common version in rigcommon, same in rotControl!

void pcCwKeyerMainWindow::fillPortsInfo()
{
    ui->comportSel->clear();


    QString description;
    QString manufacturer;
    QString serialNumber;

    ui->comportSel->addItem("");

    for(auto &info: QSerialPortInfo::availablePorts())
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


       ui->comportSel->addItem(list.first(), list);
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
            pendingBuffer.clear();
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
    if (cwKeyer) {
        cwKeyer->abortTransmission();  // stop any CW sending
        cwKeyer->close();       // close the serial port
    }

    event->accept();  // allow the window to close
}


