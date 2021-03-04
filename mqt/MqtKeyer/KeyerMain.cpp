#include <QFileDialog>

#include "keyerAbout.h"
#include "KeyerMain.h"
#include "ui_KeyerMain.h"
#include "keyctrl.h"
#include "KeyerRPCServer.h"
#include "VKMixer.h"
#include "sbdriver.h"

KeyerMain *keyerMain = nullptr;

static QString alsaStore("store");
static QString alsaRestore("restore");
static bool inhibitCallbacks = false;

// texts for displaying the current mixer set

static const char *msets[emsMaxMixerSet] = {QT_TRANSLATE_NOOP("VoiceKeyer", "Unloaded"), QT_TRANSLATE_NOOP("VoiceKeyer", "No PTT"), QT_TRANSLATE_NOOP("VoiceKeyer", "PassThrough"),
                 QT_TRANSLATE_NOOP("VoiceKeyer", "Replay"), QT_TRANSLATE_NOOP("VoiceKeyer", "Pip"), QT_TRANSLATE_NOOP("VoiceKeyer", "Replay Tone1"), QT_TRANSLATE_NOOP("VoiceKeyer", "Replay Tone2"),
                 QT_TRANSLATE_NOOP("VoiceKeyer", "Voice Record"),
                 QT_TRANSLATE_NOOP("VoiceKeyer", "CW Transmit"), QT_TRANSLATE_NOOP("VoiceKeyer", "CW PassThrough")
                };
static const char *levelLabels[emsMaxMixerSet] = {QT_TRANSLATE_NOOP("VoiceKeyer", "none"), QT_TRANSLATE_NOOP("VoiceKeyer", "none"), QT_TRANSLATE_NOOP("VoiceKeyer", "output"),
                 QT_TRANSLATE_NOOP("VoiceKeyer", "output"), QT_TRANSLATE_NOOP("VoiceKeyer", "output"), QT_TRANSLATE_NOOP("VoiceKeyer", "output"), QT_TRANSLATE_NOOP("VoiceKeyer", "output"),
                 QT_TRANSLATE_NOOP("VoiceKeyer", "input"),
                 QT_TRANSLATE_NOOP("VoiceKeyer", "output"), QT_TRANSLATE_NOOP("VoiceKeyer", "output")
                };

void lcallback( bool pPTT, bool pPTTRef, bool pL1Ref, bool pL2Ref, int lmode )
{
    if (!inhibitCallbacks)
        keyerMain->setLines(pPTT, pPTTRef, pL1Ref, pL2Ref, lmode);
}

//---------------------------------------------------------------------------
// Actually, we only want ONE level meter -
// input for recording
// output for replay/passthrough

// And we want gain control
// on input level for recording
// on output level from recording
// on transfer level for pasthrough
//---------------------------------------------------------------------------
void volcallback( unsigned int rmsvol, unsigned int peakvol, unsigned int samples )
{
    if (!inhibitCallbacks)
        keyerMain->volcallback(rmsvol, peakvol, samples);
}
void KeyerMain::volcallback(unsigned int rmsvol , unsigned int peakvol, unsigned int samples)
{
    if (!inhibitCallbacks)
        ui->levelMeter->levelChanged( rmsvol / 32768.0, peakvol / 32768.0, samples );
}

//---------------------------------------------------------------------------
void KeyerMain::setLines(bool PTTOut, bool PTTIn, bool L1, bool L2, int lmode )
{
    PTT = PTTOut;
    PTTRef = PTTIn;
    L1Ref = L1;
    L2Ref = L2;
    linesMode = lmode;
}
void KeyerMain::syncSetLines()
{
   ui->PTTOutCheckBox->setChecked(PTT);
   ui->PTTReflectCheckBox->setChecked(PTTRef);
   ui->L1ReflectCheckBox->setChecked(L1Ref);
   ui->L2ReflectCheckBox->setChecked(L2Ref);

   ui->modeLabel->setText(QString::number(linesMode));
}

KeyerMain::KeyerMain(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::KeyerMain),
    PTT(false), PTTRef(false), L1Ref(false), L2Ref(false),
    recordWait(false),
    recording(false),
    inVolChange(false),
    runner(nullptr)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    connect(&stdinReader, SIGNAL(stdinLine(QString)), this, SLOT(onStdInRead(QString)));
    stdinReader.start();

    QSettings settings;
    QByteArray geometry = settings.value("KeyerMain/geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    createCloseEvent();

    inVolChange = true;

    int recordLevel = settings.value("RecordLevel", 0).toInt();
    int replayLevel = settings.value("ReplayLevel", 0).toInt();
    int passThroughLevel = settings.value("PassThroughLevel", 0).toInt();

    ui->recordSlider->setValue(recordLevel);
    ui->replaySlider->setValue(replayLevel);
    ui->passThroughSlider->setValue(passThroughLevel);

    inVolChange = false;


    QSettings keyerSettings( GetCurrentDir() + "/Configuration/MixerSettings.ini" , QSettings::IniFormat ) ;
    QString alsaFileName = keyerSettings.value("AlsaCtlFile", "AlsaCtlFile.txt").toString();
    ui->setupScriptEdit->setText(alsaFileName);

    runAlsaScript(alsaFileName, alsaRestore);

    keyerMain = this;
    setLineCallBack( lcallback );
    setVUCallBack( &::volcallback );

    loadKeyers();

    setVolumeMults();

    connect(&LineTimer, SIGNAL(timeout()), this, SLOT(LineTimerTimer()));
    LineTimer.start(100);

    // NB CaptionTimer only runs after something changes - the line timer triggers it
    connect(&CaptionTimer, SIGNAL(timeout()), this, SLOT(CaptionTimerTimer()));

    ui->PipCheckBox->setChecked(getPipEnabled());

    ui->delayEdit->setValue(getAutoRepeatDelay());
    ui->AutoRepeatCheckBox->setChecked( getEnableAutoRepeat());

    for (int i = 1; i < 10; i++)
    {
        ui->keyCombo->addItem(QString::number(i));
    }
    ui->keyCombo->setCurrentIndex(0);
}

KeyerMain::~KeyerMain()
{
    inhibitCallbacks = true;
    delete ui;
}
void KeyerMain::onStdInRead(QString cmd)
{
    executeStdIn(cmd);
}

void KeyerMain::closeEvent(QCloseEvent *event)
{
    inhibitCallbacks = true;
    unloadKeyers();

    QWidget::closeEvent(event);
}
void KeyerMain::moveEvent(QMoveEvent *event)
{
    QSettings settings;
    settings.setValue("KeyerMain/geometry", saveGeometry());
    QWidget::moveEvent(event);
}
void KeyerMain::resizeEvent(QResizeEvent *event)
{
    QSettings settings;
    settings.setValue("KeyerMain/geometry", saveGeometry());
    QWidget::resizeEvent(event);
}
void KeyerMain::changeEvent( QEvent* e )
{
    if( e->type() == QEvent::WindowStateChange )
    {
        QSettings settings;
        settings.setValue("geometry", saveGeometry());
    }
}

void KeyerMain::LineTimerTimer( )
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

   syncSetLines();
   bool PTT = getPTT();
   if ( recordWait && PTT )
   {
      ui->recind->setText("Release PTT to stop Recording");
      recordWait = false;
      recording = true;
   }
   else
      if ( recording && !PTT )
      {
         ui->recind->setText("");
         recordWait = false;
         recording = false;
      }
   KeyerServer::publishCommand( ui->recind->text() );
   eMixerSets m = VKMixer::GetVKMixer()->GetCurrentMixerSet();

   ui->levelLabel->setText(tr(levelLabels[m]));

   QString astate;
   getActionState( astate );

   QString tswitch;
   getTransverterSwitch ( tswitch );

   // This isn't quite what we want - needs to be better english and slower changing

   QString kstatus;
   if ( getKeyerStatus( kstatus ) )
      setWindowTitle(tr( msets[ m ] ) + " : " + astate + " : " + kstatus + " : " + tswitch);
   else
      setWindowTitle(tr( msets[ m ] ) + " : " + astate + " : " + tswitch);

   static QString old;

   if ( windowTitle() != old )
   {
      old = windowTitle();
      CaptionTimer.start(200);
   }
}
void KeyerMain::CaptionTimerTimer( )
{
   CaptionTimer.stop();

   KeyerServer::publishState( windowTitle() );
}
void KeyerMain::on_recordButton_clicked()
{
    trace("Record Button");
    int fno = ui->keyCombo->currentText().toInt();
    if ( fno >= 1 && fno <= 12 )
    {
       startRecordDVPFile( fno );
       ui->recind->setText(tr("Push PTT to Commence Recording"));
       KeyerServer::publishCommand( ui->recind->text() );
       recordWait = true;
    }
}

void KeyerMain::on_playButton_clicked()
{
    trace("Play Button");
    int fno = ui->keyCombo->currentText().toInt( );
    if ( fno >= 1 && fno <= 12 )
    {
       ui->recind->setText("");
       playKeyerFile( fno, false );
    }
    KeyerServer::publishCommand( ui->recind->text() );
}

void KeyerMain::on_stopButton_clicked()
{
    trace("Stop Button");
    if ( recordWait )
    {
       ui->recind->setText("");
       recordWait = false;
    }
    if ( recording )
    {
       finishRecordDVPFile();
       ui->recind->setText("");
       recording = false;
    }
    else
       stopKeyer();
    KeyerServer::publishCommand( ui->recind->text() );
}

void KeyerMain::on_AutoRepeatCheckBox_clicked()
{
    setEnableAutoRepeat( ui->AutoRepeatCheckBox->isChecked() );

}

void KeyerMain::on_PipCheckBox_clicked()
{
    setPipEnabled( ui->PipCheckBox->isChecked() );
}

void KeyerMain::on_delayEdit_valueChanged(const QString &/*arg1*/)
{
    QString temp = ui->delayEdit->text();
    int val = temp.toInt();
    if (val > 0)
    {
       setAutoRepeatDelay( val );
    }

}

void KeyerMain::on_tuneButton_clicked()
{
    trace("Tone 1 Button");
    sendTone1();
}

void KeyerMain::on_twoToneButton_clicked()
{
    trace("Tone 2 Button");
    sendTone2();
}

void KeyerMain::on_aboutButton_clicked()
{
    keyerAbout ka;
    ka.exec();
}

void KeyerMain::runAlsaScript(const QString &alsaFileName, const QString &command)
{
    if (alsaFileName.isEmpty())
        return;

    if ( !runner)
    {
        runner = new QProcess(parent());
        connect (runner, SIGNAL(started()), this, SLOT(on_started()));
        connect (runner, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(on_finished(int, QProcess::ExitStatus)));
        connect (runner, SIGNAL(error(QProcess::ProcessError)), this, SLOT(on_error(QProcess::ProcessError)));

        connect (runner, SIGNAL(readyReadStandardError()), this, SLOT(on_readyReadStandardError()));
        connect (runner, SIGNAL(readyReadStandardOutput()), this, SLOT(on_readyReadStandardOutput()));

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
void KeyerMain::on_started()
{
    trace("AlsaCtl :started");
}

void KeyerMain::on_finished(int err, QProcess::ExitStatus exitStatus)
{
    trace("AlsaCtl :finished:" + QString::number(err) + ":" + QString::number(exitStatus));
}

void KeyerMain::on_error(QProcess::ProcessError error)
{
    trace("AlsaCtl :error:" + QString::number(error));
}

void KeyerMain::on_readyReadStandardError()
{
    QString r = runner->readAllStandardError();
    trace("AlsaCtl :stdErr:" + r);
}

void KeyerMain::on_readyReadStandardOutput()
{
    QString r = runner->readAllStandardOutput();
    trace("AlsaCtl :stdOut:" + r);
}


void KeyerMain::on_setupBrowseButton_clicked()
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

void KeyerMain::on_saveAlsaButton_clicked()
{
    QString alsaFileName = ui->setupScriptEdit->text();
    runAlsaScript(alsaFileName, alsaStore);
}

void KeyerMain::on_restoreAlsaButton_clicked()
{
    QString alsaFileName = ui->setupScriptEdit->text();
    runAlsaScript(alsaFileName, alsaRestore);
}

void KeyerMain::setVolumeMults()
{
    int record = ui->recordSlider->value();
    int replay = ui->replaySlider->value();
    int passThrough = ui->passThroughSlider->value();
    SoundSystemDriver::getSbDriver()->setVolumeMults(record, replay, passThrough);

    inVolChange = true;

    ui->recordValue->setValue(record/10.0);
    ui->replayValue->setValue(replay/10.0);
    ui->passThroughValue->setValue(passThrough/10.0);

    inVolChange = false;
}

void KeyerMain::on_recordSlider_valueChanged(int position)
{
    if (!inVolChange)
    {
        QSettings settings;
        settings.setValue("RecordLevel", position);
    }
    setVolumeMults();
}

void KeyerMain::on_replaySlider_valueChanged(int position)
{
    if (!inVolChange)
    {
        QSettings settings;
        settings.setValue("ReplayLevel", position);
    }
    setVolumeMults();
}

void KeyerMain::on_passThroughSlider_valueChanged(int position)
{
    if (!inVolChange)
    {
        QSettings settings;
        settings.setValue("PassThroughLevel", position);
    }
    setVolumeMults();
}

void KeyerMain::on_recordValue_valueChanged(double arg1)
{
    if (!inVolChange)
    {
        ui->recordSlider->setValue(static_cast<int>(arg1 * 10));
    }
}

void KeyerMain::on_replayValue_valueChanged(double arg1)
{
    if (!inVolChange)
    {
        ui->replaySlider->setValue(static_cast<int>(arg1 * 10));
    }
}

void KeyerMain::on_passThroughValue_valueChanged(double arg1)
{
    if (!inVolChange)
    {
        ui->passThroughSlider->setValue(static_cast<int>(arg1 * 10));
    }
}
