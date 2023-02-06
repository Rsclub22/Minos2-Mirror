#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QFileDialog>
#include "LogEvents.h"
#include "MShowMessageDlg.h"
#include "MTrace.h"
#include "cutils.h"
#include "fileutils.h"
#include "kprpcserver.h"
#include "sbdriver.h"
#include "serialPTT.h"

#ifdef Q_OS_UNIX
#include <unistd.h>
#endif

#include "kpmainwindow.h"
#include "ui_kpmainwindow.h"

static QString alsaStore("store");
static QString alsaRestore("restore");

KPMainWindow::KPMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::KPMainWindow)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    kpc = new KPRPCServer();
    connect(kpc, &KPRPCServer::newHost, this, &KPMainWindow::onNewHost);

    QSettings settings;
    QByteArray geometry = settings.value("KeyerProxyMain/geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    createCloseEvent();
    connect(&CloseTimer, &QTimer::timeout, this, &KPMainWindow::CloseTimerTimer);
    CloseTimer.start(100);

    QSettings keyerSettings( GetCurrentDir() + "/Configuration/MixerSettings.ini" , QSettings::IniFormat ) ;
    QString alsaFileName = keyerSettings.value("AlsaCtlFile", "AlsaCtlFile.txt").toString();
    ui->setupScriptEdit->setText(alsaFileName);

    runAlsaScript(alsaFileName, alsaRestore);

    fillPortsInfo(ui->spCombo);

    bool senabled = settings.value("PTTEnabled").toBool();
    QString serialPort = settings.value("PTTPort").toString();
    bool rts = settings.value("PTTRTS").toBool();
    bool dtr = settings.value("PTTDTR").toBool();

    inPTTConfig = true;
    ui->serialPTTEnable->setChecked(senabled);
    ui->spCombo->setCurrentText(serialPort);
    ui->rts->setChecked(rts);
    ui->dtr->setChecked(dtr);
    inPTTConfig = false;



    QStringList outputList = SoundSystemDriver::getSbDriver()->getOutputDevices();

    ui->outputCombo->addItem(tr("Remote IP Client"));
    ui->outputCombo->addItems(outputList);
    QString currentOutput = settings.value("outputDevice", SoundSystemDriver::getSbDriver()->getDefaultOutputDevice()).toString();
    ui->outputCombo->setCurrentText(currentOutput);

    QString port = settings.value("SenderPort", DEFAULT_PORT).toString();
    ui->portEdit->setText(port);
    ui->portEdit->setValidator(new QIntValidator(0, 0xffff, this));

    SoundSystemDriver::getSbDriver()->setSampleRate( 48000);
    connect(SoundSystemDriver::getSbDriver(), &SoundSystemDriver::sequenceCount, this, &KPMainWindow::onSequenceCount);

    SoundSystemDriver::getSbDriver()->initialise("IP"
                                                 , ui->outputCombo->currentText()
                                                 , host
                                                 , ui->portEdit->text());
    connect(SoundSystemDriver::getSbDriver(), &SoundSystemDriver::setVU, this, &KPMainWindow::doSetVU);
    connect(SoundSystemDriver::getSbDriver(), &SoundSystemDriver::ptt, this, &KPMainWindow::doPTT);

    //We need a PTT indicator
    //We need to implement PTT from Keyer and into here, and to radio
}

KPMainWindow::~KPMainWindow()
{
    delete ui;
    delete kpc;
}
void KPMainWindow::fillPortsInfo(QComboBox* comportSel)
{
    comportSel->clear();

    QString description;
    QString manufacturer;
    QString serialNumber;

    comportSel->addItem("");

    QList<QSerialPortInfo> availablePorts = QSerialPortInfo::availablePorts();
    for (auto const &info: qAsConst(availablePorts))
    {
        QStringList list;
        description = info.description();
        manufacturer = info.manufacturer();
#if QT_VERSION > QT_VERSION_CHECK(5, 3, 0)
        serialNumber = info.serialNumber();
#endif
        list << info.portName()
             << (!description.isEmpty() ? description : QString())
             << (!manufacturer.isEmpty() ? manufacturer : QString())
             << (!serialNumber.isEmpty() ? serialNumber : QString())
             << info.systemLocation()
             << (info.vendorIdentifier() ? QString::number(info.vendorIdentifier(), 16) : QString())
             << (info.productIdentifier() ? QString::number(info.productIdentifier(), 16) : QString());


        comportSel->addItem(list.first(), list);

    }

}void KPMainWindow::CloseTimerTimer()
{
    static bool closed = false;
    if ( !closed )
    {
       if ( checkCloseEvent() )
       {
          closed = true;
          close();
       }
    }
}
void KPMainWindow::closeEvent(QCloseEvent *event)
{
    QWidget::closeEvent(event);
}
void KPMainWindow::moveEvent(QMoveEvent *event)
{
    QSettings settings;
    settings.setValue("KeyerProxyMain/geometry", saveGeometry());
    QWidget::moveEvent(event);
}
void KPMainWindow::resizeEvent(QResizeEvent *event)
{
    QSettings settings;
    settings.setValue("KeyerProxyMain/geometry", saveGeometry());
    QWidget::resizeEvent(event);
}
void KPMainWindow::changeEvent( QEvent* e )
{
    if( e->type() == QEvent::WindowStateChange )
    {
        QSettings settings;
        settings.setValue("geometry", saveGeometry());
    }
}
void KPMainWindow::runAlsaScript(const QString &alsaFileName, const QString &command)
{
    if (alsaFileName.isEmpty())
        return;

    if ( !runner)
    {
        runner = new QProcess(parent());
        connect (runner, &QProcess::started, this, &KPMainWindow::on_started);
        connect (runner, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &KPMainWindow::on_finished);
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
        connect (runner, &QProcess::errorOccurred, this, &KPMainWindow::on_error);
#else
        connect (runner, &QProcess::error, this, &KPMainWindow::on_error);
#endif
        connect (runner, &QProcess::readyReadStandardError, this, &KPMainWindow::on_readyReadStandardError);
        connect (runner, &QProcess::readyReadStandardOutput, this, &KPMainWindow::on_readyReadStandardOutput);

        QString wdir = GetCurrentDir();
        runner->setWorkingDirectory(wdir);
    }


    QString commandLine = "alsactl -f " + alsaFileName + " " + command;// <card>

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        QStringList progArgs = runner->splitCommand(commandLine);
        const QString prog = progArgs.takeFirst();

        runner->start(prog, progArgs);
#else
        runner->start(commandLine);
#endif
}
void KPMainWindow::on_started()
{
    trace("AlsaCtl :started");
}

void KPMainWindow::on_finished(int err, QProcess::ExitStatus exitStatus)
{
    trace("AlsaCtl :finished:" + QString::number(err) + ":" + QString::number(exitStatus));
}

void KPMainWindow::on_error(QProcess::ProcessError error)
{
    trace("AlsaCtl :error:" + QString::number(error));
}

void KPMainWindow::on_readyReadStandardError()
{
    QString r = runner->readAllStandardError();
    trace("AlsaCtl :stdErr:" + r);
}

void KPMainWindow::on_readyReadStandardOutput()
{
    QString r = runner->readAllStandardOutput();
    trace("AlsaCtl :stdOut:" + r);
}


void KPMainWindow::on_setupBrowseButton_clicked()
{
    QString InitialDir = GetCurrentDir();

    QString Filter = tr("Alsa Control Files") + " (*.txt);;" +
                     tr("All Files") + " (*.*)" ;

    QString alsaFileName = QFileDialog::getOpenFileName( this,
                       tr("File for mixer setup"),
                       InitialDir,                   // opendir
                       Filter );

    if ( !alsaFileName.isEmpty() )
    {
        // run alsactl on the file
        // so we need the alsa card names :(
        // AND I think we need the QtAudio card names
        // unless we can always use the default card...

        // alsactl -f <filename> restore <card>

        // And save the file name for the next startup

        ui->setupScriptEdit->setText(alsaFileName);

        {
            QSettings keyerSettings( GetCurrentDir() + "/Configuration/MixerSettings.ini" , QSettings::IniFormat ) ;
            keyerSettings.setValue("AlsaCtlFile", alsaFileName);
            keyerSettings.sync();
        }
    #ifdef Q_OS_UNIX
        sync();         // as just turning machine off can clear the ini file
    #endif


        runAlsaScript(alsaFileName, alsaRestore);
    }
}

void KPMainWindow::on_saveAlsaButton_clicked()
{
    QString alsaFileName = ui->setupScriptEdit->text();
    runAlsaScript(alsaFileName, alsaStore);
}

void KPMainWindow::on_restoreAlsaButton_clicked()
{
    QString alsaFileName = ui->setupScriptEdit->text();
    runAlsaScript(alsaFileName, alsaRestore);
}

void KPMainWindow::on_outputCombo_activated(int /*index*/)
{
    QSettings settings;
    settings.setValue("outputDevice", ui->outputCombo->currentText());
    trace("About to re-initialise audio");
    SoundSystemDriver::getSbDriver()->closedown();
    SoundSystemDriver::getSbDriver()->initialise("IP"
                                                 , ui->outputCombo->currentText()
                                                 , host
                                                 , ui->portEdit->text());
}
void KPMainWindow::onNewHost(QString h, QString p, QString s)
{
    SoundSystemDriver::getSbDriver()->closedown();
    SoundSystemDriver::getSbDriver()->setSampleRate(s.toInt());
    SoundSystemDriver::getSbDriver()->initialise("IP"
                                                 , ui->outputCombo->currentText()
                                                 , h
                                                 , p);
}
void KPMainWindow::onSequenceCount(qint64 s)
{
    ui->seqLabel->setText(QString::number(s));
}
void KPMainWindow::doSetVU( vudata v)
{
    ui->levelMeter->levelChanged( v.peak / 32768.0, v.rms / 32768.0, v.blocks );

    // delay is in millisecs

    ui->delayLabel->setText(QString::number(v.delay));
    ui->bufferedLabel->setText(QString("C%1 M%2 D%3").arg(v.callbacks).arg(v.missed).arg(v.dropped));
}
bool KPMainWindow::openSerialPort()
{
    QSettings settings;

    bool senabled = settings.value("PTTEnabled").toBool();

    if (senabled && !sp)
    {
        sp = new SerialPTT(this);
    }
    if (senabled)
    {
        if (!sp->isOpen())
        {
            QString serialPort = settings.value("PTTPort").toString();
            bool rts = settings.value("PTTRTS").toBool();
            bool dtr = settings.value("PTTDTR").toBool();

            if (!sp->openComport(serialPort, rts, dtr))
            {
                mShowMessage(sp->error(), this);
                return false;
            }
        }
    }
    return sp && sp->isOpen();
}
void KPMainWindow::doPTT(bool s)
{
    if (openSerialPort())
    {
        if (pttState != s)
        {
            if (s)
            {
                ui->pttLabel->setText(HtmlFontColour(Qt::red) + QString("PTT"));
            }
            else
            {
                ui->pttLabel->setText(HtmlFontColour(Qt::blue) + QString("PTT"));
            }
            pttState = s;

            if (sp->isOpen())
            {
                sp->setPTT(s);
            }
        }
    }
}
void KPMainWindow::saveSerialSettings()
{
    if (!inPTTConfig)
    {
        QString serialPort = ui->spCombo->currentText();
        bool rts = ui->rts->isChecked();
        bool dtr = ui->dtr->isChecked();
        bool senabled = ui->serialPTTEnable->isChecked();

        QSettings settings;
        settings.setValue("PTTEnabled", senabled);
        settings.setValue("PTTPort", serialPort);
        settings.setValue("PTTRTS", rts);
        settings.setValue("PTTDTR", dtr);

        if (sp)
        {
            sp->setRtsDtr(rts, dtr);
        }
    }
}
void KPMainWindow::on_spCombo_activated(int /*index*/)
{
    saveSerialSettings();
}


void KPMainWindow::on_dtr_clicked()
{
    saveSerialSettings();
}


void KPMainWindow::on_rts_clicked()
{
    saveSerialSettings();
}


void KPMainWindow::on_serialPTTEnable_stateChanged(int /*arg1*/)
{
    saveSerialSettings();

}


void KPMainWindow::on_pushButton_clicked()
{
    doPTT(!pttState);
}

