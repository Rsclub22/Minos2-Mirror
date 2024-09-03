#include <QSettings>
#include <QFileDialog>
#include <QDateTime>
#include "MShowMessageDlg.h"
#include "regsettings.h"
#include "AppStartup.h"
#include "RPCCommandConstants.h"
#include "dtg.h"
#include "fileutils.h"
#include "MTrace.h"
#include "MinosRPC.h"
#include "LogEvents.h"
#include "rcmainwindow.h"
#include "ui_rcmainwindow.h"

const char * indevKey("InDevice");
const char * indevKey2("InDevice2");
const char * outdevKey("OutDevice");
const char * baseFileKey("BaseFile");
const char * cycleRateKey("CycleRate");

MainWindow *mainWindow = nullptr;

void volcallback( int instance, unsigned int peakvol, unsigned int rmsvol, unsigned int samples )
{
        mainWindow->volcallback(instance, peakvol, rmsvol, samples);
}

void MainWindow::volcallback(int instance, unsigned int peakvol, unsigned int rmsvol , unsigned int samples)
{
    if (instance == 1)
    {
        ui->levelMeter->levelChanged( peakvol / 32768.0, rmsvol / 32768.0, samples );
    }
    else if (instance == 2)
    {
        ui->levelMeter_2->levelChanged( peakvol / 32768.0, rmsvol / 32768.0, samples );
    }
}
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    mainWindow = this;

    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    connect(commandReader.data(), &CommandReader::commandLine, this, &MainWindow::onCommandRead);

    if (rass.inputDevices.count() == 0)
    {
        mShowMessage(tr("No Sound Input devices available"), this);
        exit(2);
    }

    RegSettings gsettings;
    QByteArray geometry = gsettings.getSettings().value("RigRecorderMain/geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    createCloseEvent();

    connect(&closeTimer, &QTimer::timeout, this, &MainWindow::onCloseTimer);
    closeTimer.start(100);

    trace("About to initialise audio");
    rass.setRate(11025);
    rass.setVUCallBack( &::volcallback );

    ui->inChannelCB->addItems(rass.inputDevices);
    ui->inChannelCB_2->addItem(QString());
    ui->inChannelCB_2->addItems(rass.inputDevices);


    connect(ui->inChannelCB, &QComboBox::currentTextChanged, this, &MainWindow::inChannelCB_currentTextChanged);
    connect(ui->inChannelCB_2, &QComboBox::currentTextChanged, this, &MainWindow::inChannelCB_2_currentTextChanged);

    MinosRPC *rpc = MinosRPC::getMinosRPC(getAppStartupName(), true);
    connect(rpc, &MinosRPC::notify, this, &MainWindow::on_notify);

    QStringList sv = {rpcConstants::monitorLogCategory};
    rpc->findProviders(rpcConstants::LoggerCategory, sv);

    doConfig();
}

void MainWindow::doConfig()
{
    QSettings csettings(getDirectoryLocation(dlConfiguration) + "/RigRecorderSelect.ini", QSettings::IniFormat);
    configFile = csettings.value("ConfigurationFile").toString();
    if (configFile.isEmpty())
    {
        configFile = getDirectoryLocation(dlConfiguration) + "/RigRecorder/RigRecorder.ini";
        csettings.setValue("ConfigurationFile", configFile);
    }

    setConfigDisplay(configFile);

    QSettings settings(configFile, QSettings::IniFormat);

    QString indev = settings.value(indevKey).toString();

    ui->inChannelCB->setCurrentText(indev);

    QString indev_2 = settings.value(indevKey2).toString();

    ui->inChannelCB_2->setCurrentText(indev_2);

    QString baseFile = settings.value(baseFileKey, "./RigRecording/rigrec.wav").toString();
    ui->baseFilename->setText(baseFile);

    int cycleTime = settings.value(cycleRateKey, 10).toInt();
    ui->rotInterval->setValue(cycleTime);

    rass.initialise(ui->inChannelCB->currentText(), ui->inChannelCB_2->currentText());

    ui->startRecButton->setEnabled(true);
    ui->stopRecButton->setEnabled(false);

    inVolChange = true;

    int recordLevel = settings.value("RecordLevel", 0).toInt();

    ui->recordSlider->setValue(recordLevel);

    inVolChange = false;

    bool mono = settings.value("Mono", false).toBool();
    ui->monoCb->setChecked(mono);
    rass.setMono(mono);

    bool link = settings.value("ContestLink", false).toBool();
    ui->contestLinkCB->setChecked(link);

    settings.setValue("configured", true); // make sure the config file is created;
}
void MainWindow::setConfigDisplay(QString s)
{
    QString f = "./" + GetCleanPath(s);
    ui->rrConfigEdit->setText(f);
    QFontMetrics fm(ui->rrConfigEdit->font());
    int pixelsWide = fm.boundingRect(f).width() + fm.maxWidth();

    ui->rrConfigEdit->setFixedWidth(pixelsWide);
    adjustSize();
}
MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::moveEvent(QMoveEvent *event)
{
    RegSettings settings;
    settings.getSettings().setValue("RigRecorderMain/geometry", saveGeometry());
    QWidget::moveEvent(event);
}
void MainWindow::resizeEvent(QResizeEvent *event)
{
    RegSettings settings;
    settings.getSettings().setValue("RigRecorderMain/geometry", saveGeometry());
    QWidget::resizeEvent(event);
}
void MainWindow::changeEvent( QEvent* e )
{
    if( e->type() == QEvent::WindowStateChange )
    {
        RegSettings settings;
        settings.getSettings().setValue("RigRecorderMain/geometry", saveGeometry());
    }
}
void MainWindow::onCommandRead(QString cmd)
{
    bool doClose = false;
    if (cmd.indexOf("Shutdown", 0, Qt::CaseInsensitive) >= 0)
    {
        trace("MainWindow::onCommandRead - Start shutdown");
        doClose = true;

        closing = true;

        rass.closedown();
    }
    if (doClose)
        close();
}
void MainWindow::onCloseTimer()
{
    static bool closed = false;
    if ( closed )
    {
        return;
    }
    if ( checkCloseEvent() )
    {
        closed = true;
        close();
        return;
    }

    if (closing)
    {
        trace("closing set in close timer");
        return;
    }
    bool link = ui->contestLinkCB->isChecked();

    if (!stopped && link && tstart.isValid() && tend.isValid())
    {
        QDateTime tnow = QDateTime::currentDateTimeUtc();

        if (started && tnow > tend.addSecs(60))
        {
            trace("Stop record by time");
            on_stopRecButton_clicked();
        }
        else
            if (!started && tstart.addSecs(-60) < tnow && tnow < tend.addSecs(60))
            {
                trace("Start record by time");
                on_startRecButton_clicked();
            }
    }
}
void MainWindow::closeEvent(QCloseEvent *event)
{
    trace("MainWindow::closeEvent");

    closing = true;

    rass.closedown();

    QWidget::closeEvent(event);
}

void MainWindow::on_startRecButton_clicked()
{
    QString fname = ui->baseFilename->text() ;

    trace("About to start stream to " + fname);

    rass.startDMA(fname, ui->rotInterval->value() * 60);    // as we have it in minutes

    trace("Start stream complete");
    started = true;
    stopped = false;

    ui->detailsFrame->setEnabled(false);
    ui->startRecButton->setEnabled(false);
    ui->stopRecButton->setEnabled(true);
}

void MainWindow::on_stopRecButton_clicked()
{
    trace("About to stop audio");
    rass.stopDMA();
    trace("audio stopped");
    started = false;
    stopped = true;
    ui->detailsFrame->setEnabled(true);
    ui->startRecButton->setEnabled(true);
    ui->stopRecButton->setEnabled(false);
}

void MainWindow::on_baseFileBrowse_clicked()
{
    QString InitialDir = GetCurrentDir();

    QString Filter = tr("Record Files (*.wav);;All Files (*.*)") ;

    QString fileName = QFileDialog::getSaveFileName( this,
                       tr("Recorded Files"),
                       InitialDir,                   // opendir
                       Filter );

    if (!fileName.isEmpty())
    {
        ui->baseFilename->setText(fileName);

        QSettings settings(configFile, QSettings::IniFormat);
        settings.setValue(baseFileKey, fileName);
    }
}

void MainWindow::inChannelCB_currentTextChanged(const QString &arg1)
{
    if (!closing)
    {
        QSettings settings(configFile, QSettings::IniFormat);
        settings.setValue(indevKey, arg1);

        trace("About to re-initialise audio");
        rass.closedown();
        rass.initialise(ui->inChannelCB->currentText(), ui->inChannelCB_2->currentText());

    }
}
void MainWindow::inChannelCB_2_currentTextChanged(const QString &arg1)
{
    if (!closing)
    {
        QSettings settings(configFile, QSettings::IniFormat);
        settings.setValue(indevKey2, arg1);

        trace("About to re-initialise audio");
        rass.closedown();
        rass.initialise(ui->inChannelCB->currentText(), ui->inChannelCB_2->currentText());

    }
}

void MainWindow::on_baseFilename_editingFinished()
{
    if (!closing)
    {
        QSettings settings(configFile, QSettings::IniFormat);
        settings.setValue(baseFileKey, ui->baseFilename->text());
    }
}

void MainWindow::on_rotInterval_editingFinished()
{
    if (!closing)
    {
        QSettings settings(configFile, QSettings::IniFormat);
        settings.setValue(cycleRateKey, ui->rotInterval->value());
    }
}
void MainWindow::setVolumeMults()
{
    int record = ui->recordSlider->value();
    rass.setRecordLevel(record);
    int record2 = ui->recordSlider_2->value();
    rass.setRecordLevel2(record2);

    inVolChange = true;

    ui->recordLevel->setValue(record/10.0);
    ui->recordLevel_2->setValue(record2/10.0);

    inVolChange = false;
}

void MainWindow::on_recordLevel_valueChanged(double arg1)
{
    if (!inVolChange)
    {
        ui->recordSlider->setValue(static_cast<int>(arg1 * 10));
    }
}

void MainWindow::on_recordSlider_valueChanged(int position)
{
    if (!inVolChange)
    {
        QSettings settings(configFile, QSettings::IniFormat);
        settings.setValue("RecordLevel", position);
    }
    setVolumeMults();
}

void MainWindow::on_monoCb_stateChanged(int /*arg1*/)
{
    bool mono = ui->monoCb->isChecked();
    rass.setMono(mono);
    QSettings settings(configFile, QSettings::IniFormat);
    settings.setValue("Mono", mono);
}
void MainWindow::on_recordLevel_2_valueChanged(double arg1)
{
    if (!inVolChange)
    {
        ui->recordSlider_2->setValue(static_cast<int>(arg1 * 10));
    }
}

void MainWindow::on_recordSlider_2_valueChanged(int position)
{
    if (!inVolChange)
    {
        QSettings settings(configFile, QSettings::IniFormat);
        settings.setValue("RecordLevel2", position);
    }
    setVolumeMults();
}

void MainWindow::on_contestLinkCB_stateChanged(int /*arg1*/)
{
    bool link = ui->contestLinkCB->isChecked();
    QSettings settings(configFile, QSettings::IniFormat);
    settings.setValue("ContestLink", link);
}

void MainWindow::on_notify(AnalysePubSubNotify an, const QString /*from*/ )
{
    // pubsub notify
    //trace( "Notify callback from " + from + ( !an.getOK() ? ":Error" : ":Normal" ) );

    if ( an.getOK() )
    {
        PublishState state = an.getState();
        if (state != psPublished)
        {
            return;
        }
        //QString key = an.getKey();          // key is minos file name
        QString value = an.getValue();      // value is stanzacount;[band] name;start time;end time

        if ( an.getCategory() == rpcConstants::monitorLogCategory )
        {
            QStringList args = value.split(";");
            if (args.size() < 4)
            {
                return;
            }
            QDateTime  contestStart = CanonicalToTDT(args[2]);
            if (!tstart.isValid() || contestStart < tstart)
            {
                tstart = contestStart;
            }

            QDateTime  contestEnd = CanonicalToTDT(args[3]);
            if (!tend.isValid() || contestEnd > tend)
            {
                tend = contestEnd;
            }
        }
    }
}


void MainWindow::on_rrConfigBrowse_clicked()
{
    on_stopRecButton_clicked();

    stopped = true;
    started = false;
    QString InitialDir = ExtractFileDir(configFile);

    QFileInfo qf(InitialDir);

    InitialDir = qf.canonicalFilePath();

    QString Filter = tr("INI (*.ini);;"
                        "All Files (*.*)") ;

    QString fname = QFileDialog::getOpenFileName( this,
                                                 tr("Rig Recorder Configuration file"),
                                                 configFile,  // dir
                                                 Filter
                                                 );

    if (!fname.isEmpty())
    {
        configFile = fname;
        QSettings csettings(getDirectoryLocation(dlConfiguration) + "/DataModeSelect.ini", QSettings::IniFormat);
        csettings.setValue("ConfigurationFile", configFile);
        setConfigDisplay(configFile);
    }

    started = false;
    stopped = false;

    doConfig();
}

