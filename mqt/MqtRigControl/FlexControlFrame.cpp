#include <QSettings>
#include "regsettings.h"
#include "MTrace.h"
#include "cutils.h"

#include "rigcontrolmainwindow.h"
#include "FlexControlFrame.h"
#include "ui_FlexControlFrame.h"
/*
Sending from FlexControl to host:
All commands terminate with ';' (no returns or line feeds)

Sends F0304;F0304; on reset

U; - knob CW (single tick) -- U02; U03; U04; etc - multiticks
D; - knob CCW (single tick) -- D02; D03; D04; etc - multiticks
S; - short press, main knob
L; - long press, main knob
C; - fast double click, main knob

The fast knob codes reflect multiple encoder ticks between USB polling times, so the knob should be able to keep track of fast spinning.  (Alas, SmartSDR will not always keep up so well.)

XnS; - normal press, key n=1,2,3
XnL; - long press
XnC; - fast double click
e.g.
   U; (frequency up one tick)
   X2S; (normal press, button 2)

Sending from host to FlexControl:

Ixyz;  where x,y,z = 1 or 0 for LED 0, 1, 2 on or off
e.g.
   I001; set right hand LED on
   I000; set all LEDs off
   I111; set all LEDs on
*/
ControlFlex *ControlFlex::cf = nullptr;

FlexControlFrame::FlexControlFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::FlexControlFrame)
{
    ui->setupUi(this);
    fillPortsInfo(ui->comPort);

    RegSettings settings;
    QString comportname = settings.getSettings().value("FlexControl/COMPort").toString();

    if (ui->comPort->findText(comportname) < 0)
    {
        ui->comPort->insertItem(0, comportname);
    }

    ui->comPort->setCurrentText(comportname);

    bool cenabled = settings.getSettings().value("FlexControl/Enabled", false).toBool();
    ui->enabledCb->setChecked(cenabled);
}

FlexControlFrame::~FlexControlFrame()
{
    delete ui;
}
void FlexControlFrame::on_enabledCb_clicked()
{
    RegSettings settings;
    settings.getSettings().setValue("FlexControl/Enabled", ui->enabledCb->isChecked());
}

void FlexControlFrame::fillPortsInfo(QComboBox *cb)
{
    QString blankString;
    cb->clear();


    QString description;
    QString manufacturer;
    QString serialNumber;

    cb->addItem("");

    for(auto &info: QSerialPortInfo::availablePorts())
    {
        QStringList list;
        description = info.description();
        manufacturer = info.manufacturer();
        serialNumber = info.serialNumber();

        list << info.portName()
             << (!description.isEmpty() ? description : blankString)
             << (!manufacturer.isEmpty() ? manufacturer : blankString)
             << (!serialNumber.isEmpty() ? serialNumber : blankString)
             << info.systemLocation()
             << (info.vendorIdentifier() ? QString::number(info.vendorIdentifier(), 16) : blankString)
             << (info.productIdentifier() ? QString::number(info.productIdentifier(), 16) : blankString);


        cb->addItem(list.first(), list);
    }
}
void FlexControlFrame::on_comPort_activated(const QString &arg1)
{
    RegSettings settings;
    settings.getSettings().setValue("FlexControl/COMPort", arg1);

}

//============================================================================
ControlFlex::ControlFlex()
{
    connect(this, &ControlFlex::dataReceived, mainWindow, &RigControlMainWindow::tuneData);
}

ControlFlex::~ControlFlex()
{

}

void ControlFlex::stop()
{
    if (sp)
    {
        sp->close();
        delete sp;
        sp = nullptr;
    }
    openFlag = false;

}
void ControlFlex::start()
{
    stop();
    RegSettings settings;
    bool enabled = settings.getSettings().value("FlexControl/Enabled", false).toBool();
    QString comPort = settings.getSettings().value("FlexControl/COMPort").toString();

    if (enabled && !comPort.isEmpty())
    {
        sp = new QSerialPort;
        sp->setPortName(comPort);

        // http://ryeng.name/blog/3
        //1200 bps (Rot1Prog) or 600 bps (Rot2Prog), 8 bits, no parity and 1 stop bit.

        sp->setBaudRate(QSerialPort::Baud9600);  // for SPID
        sp->setDataBits(QSerialPort::Data8);
        sp->setParity(QSerialPort::NoParity);
        sp->setStopBits(QSerialPort::OneStop);
        sp->setFlowControl(QSerialPort::NoFlowControl);

        connect(sp, &QSerialPort::readyRead, this, &ControlFlex::on_readyRead);

        openFlag = sp->open(QIODevice::ReadWrite);
        if (!openFlag)
        {
            QString msg = sp->errorString();
            trace(QString("Serial open error %1").arg(msg));
            emit errString(msg);
        }
        else
        {
            emit errString(QString());
        }
    }
}

void ControlFlex::on_readyRead()
{
    QByteArray data = sp->readAll();
    hex_dump(data, 8, "FlexControl");

    emit dataReceived(data);
}
