#include <QSettings>
#include <QFileDialog>
#include <QDateTime>
#include "fileutils.h"
#include "MTrace.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

const QString indevKey("InDevice");
const QString outdevKey("OutDevice");
const QString baseFileKey("BaseFile");
const QString cycleRateKey("CycleRate");

MainWindow *mainWindow = nullptr;

void volcallback( unsigned int rmsvol, unsigned int peakvol, unsigned int samples )
{
        mainWindow->volcallback(rmsvol, peakvol, samples);
}

void MainWindow::volcallback(unsigned int rmsvol , unsigned int peakvol, unsigned int samples)
{
        ui->levelMeter->levelChanged( rmsvol / 32768.0, peakvol / 32768.0, samples );
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    mainWindow = this;

    ui->setupUi(this);

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    connect(&stdinReader, &StdInReader::stdinLine, this, &MainWindow::onStdInRead);
    stdinReader.start();

    QSettings settings;
    QByteArray geometry = settings.value("RigRecorderMain/geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    createCloseEvent();

    connect(&closeTimer, &QTimer::timeout, this, &MainWindow::onCloseTimer);
    closeTimer.start(100);

    ui->inChannelCB->addItems(rass.inputDevices);
    ui->outChannelCB->addItems(rass.outputDevices);

    QString indev = settings.value(indevKey, "").toString();
    QString outdev = settings.value(outdevKey, "").toString();

    ui->inChannelCB->setCurrentText(indev);
    ui->outChannelCB->setCurrentText(outdev);

    QString baseFile = settings.value(baseFileKey, "").toString();
    ui->baseFilename->setText(baseFile);

    int cycleTime = settings.value(cycleRateKey, 10).toInt();
    ui->rotInterval->setValue(cycleTime);

    connect(ui->inChannelCB, SIGNAL(currentTextChanged(const QString &)), this, SLOT(inChannelCB_currentTextChanged(const QString &)));
    connect(ui->outChannelCB, SIGNAL(currentTextChanged(const QString &)), this, SLOT(outChannelCB_currentTextChanged(const QString &)));

    trace("About to initialise audio");
    rass.setRate(11025);
    rass.initialise(ui->inChannelCB->currentText(), ui->outChannelCB->currentText());

    rass.setVUCallBack( &::volcallback );

    ui->startRecButton->setEnabled(true);
    ui->stopRecButton->setEnabled(false);

    inVolChange = true;

    int recordLevel = settings.value("RecordLevel", 0).toInt();

    ui->recordSlider->setValue(recordLevel);

    inVolChange = false;

    bool mono = settings.value("Mono", false).toBool();
    ui->recordMono->setChecked(mono);

    bool autostart = settings.value("AutoStart", false).toBool();
    ui->autostartCb->setChecked(autostart);

    if (autostart)
    {
        on_startRecButton_clicked();
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::onStdInRead(QString cmd)
{
    executeStdIn(cmd);
}

void MainWindow::moveEvent(QMoveEvent *event)
{
    QSettings settings;
    settings.setValue("RigRecorderMain/geometry", saveGeometry());
    QWidget::moveEvent(event);
}
void MainWindow::resizeEvent(QResizeEvent *event)
{
    QSettings settings;
    settings.setValue("RigRecorderMain/geometry", saveGeometry());
    QWidget::resizeEvent(event);
}
void MainWindow::changeEvent( QEvent* e )
{
    if( e->type() == QEvent::WindowStateChange )
    {
        QSettings settings;
        settings.setValue("RigRecorderMain/geometry", saveGeometry());
    }
}

void MainWindow::onCloseTimer()
{
    static bool closed = false;
    if ( !closed )
    {
       if ( checkCloseEvent() )
       {
          closed = true;
          close();
          return;
       }
    }
    else
    {
        bool show = getShowApp();
        if ( !isVisible() && show )
        {
           setVisible(true);
        }
        if ( isVisible() && !show )
        {
           setVisible(false);
        }
    }
}
void MainWindow::closeEvent(QCloseEvent *event)
{
    closing = true;

    rass.closedown();

    QWidget::closeEvent(event);
}
void MainWindow::on_closeButton_clicked()
{
    close();
}

void MainWindow::on_startRecButton_clicked()
{
    QString fname = ui->baseFilename->text() ;

    trace("About to start stream to " + fname);

    rass.startDMA(fname, ui->rotInterval->value() * 60);    // as we have it in minutes

    trace("Start stream complete");

    ui->detailsFrame->setEnabled(false);
    ui->startRecButton->setEnabled(false);
    ui->stopRecButton->setEnabled(true);
}

void MainWindow::on_stopRecButton_clicked()
{
    trace("About to stop audio");
    rass.stopDMA();
    trace("audio stopped");
    ui->detailsFrame->setEnabled(true);
    ui->startRecButton->setEnabled(true);
    ui->stopRecButton->setEnabled(false);
}

void MainWindow::on_baseFileBrowse_clicked()
{
    QString InitialDir = GetCurrentDir();

    QString Filter = "Record Files (*.wav);;"
                     "All Files (*.*)" ;

    QString fileName = QFileDialog::getSaveFileName( this,
                       "Recorded Files",
                       InitialDir,                   // opendir
                       Filter );

    if (!fileName.isEmpty())
    {
        ui->baseFilename->setText(fileName);
        QSettings settings;
        settings.setValue(baseFileKey, fileName);
    }
}

void MainWindow::inChannelCB_currentTextChanged(const QString &arg1)
{
    if (!closing)
    {
        QSettings settings;
        settings.setValue(indevKey, arg1);

        trace("About to re-initialise audio");
        rass.closedown();
        rass.initialise(ui->inChannelCB->currentText(), ui->outChannelCB->currentText());

    }
}

void MainWindow::outChannelCB_currentTextChanged(const QString &arg1)
{
    if (!closing)
    {
        QSettings settings;
        settings.setValue(outdevKey, arg1);
        trace("About to re-initialise audio");
        rass.closedown();
        rass.initialise(ui->inChannelCB->currentText(), ui->outChannelCB->currentText());
    }
}

void MainWindow::on_baseFilename_editingFinished()
{
    if (!closing)
    {
        QSettings settings;
        settings.setValue(baseFileKey, ui->baseFilename->text());
    }
}

void MainWindow::on_rotInterval_editingFinished()
{
    if (!closing)
    {
        QSettings settings;
        settings.setValue(cycleRateKey, ui->rotInterval->value());
    }
}
void MainWindow::setVolumeMults()
{
    int record = ui->recordSlider->value();
    rass.setRecordLevel(record);

    inVolChange = true;

    ui->recordLevel->setValue(record/10.0);

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
        QSettings settings;
        settings.setValue("RecordLevel", position);
    }
    setVolumeMults();
}

void MainWindow::on_recordMono_stateChanged(int /*arg1*/)
{
    bool mono = ui->recordMono->isChecked();
    rass.setMono(mono);
    QSettings settings;
    settings.setValue("Mono", mono);
}

void MainWindow::on_autostartCb_stateChanged(int /*arg1*/)
{
    bool autostart = ui->autostartCb->isChecked();
    QSettings settings;
    settings.setValue("AutoStart", autostart);
}
