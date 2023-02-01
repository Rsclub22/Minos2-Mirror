#include <QFileDialog>
#include "AppStartup.h"
#include "LogEvents.h"
#include "MTrace.h"
#include "MinosRPC.h"
#include "fileutils.h"
#include "kprpcserver.h"
#include "sbdriver.h"

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

    KPRPCServer *kpc = new KPRPCServer();
    Q_UNUSED(kpc)

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

    QStringList outputList = SoundSystemDriver::getSbDriver()->getOutputDevices();

    ui->outputCombo->addItem(tr("Remote IP Client"));
    ui->outputCombo->addItems(outputList);
    QString currentOutput = settings.value("outputDevice", SoundSystemDriver::getSbDriver()->getDefaultOutputDevice()).toString();
    ui->outputCombo->setCurrentText(currentOutput);

    QString port = settings.value("SenderPort", DEFAULT_PORT).toString();
    ui->portEdit->setText(port);
    ui->portEdit->setValidator(new QIntValidator(0, 0xffff, this));

    SoundSystemDriver::getSbDriver()->setSampleRate( 22050);
    SoundSystemDriver::getSbDriver()->initialise("IP"
                                                 , ui->outputCombo->currentText()
                                                 , ui->portEdit->text());
}

KPMainWindow::~KPMainWindow()
{
    delete ui;
}
void KPMainWindow::CloseTimerTimer()
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
                                                 , ui->portEdit->text());
}
