#include <QFileDialog>

#include "keyerAbout.h"
#include "KeyerMain.h"
#include "ui_KeyerMain.h"
#include "keyctrl.h"
#include "KeyerRPCServer.h"
#include "VKMixer.h"
#include "sbdriver.h"
#include "keyers.h"
#include "portcon.h"
#include "KeyerJson.h"

KeyerMain *keyerMain = nullptr;

static QString alsaStore("store");
static QString alsaRestore("restore");
static bool inhibitCallbacks = false;


void KeyerMain::lcallback( bool pPTT, bool pPTTRef, bool pL1Ref, bool pL2Ref, int lmode )
{
    if (!inhibitCallbacks)
        keyerMain->setLines(pPTT, pPTTRef, pL1Ref, pL2Ref, lmode);
}

void KeyerMain::doSliders(int rec, int rep, int pass)
{
    inVolChangeCount++;
    trace(QString("Set Slider positions %1;%2;%3").arg(rec).arg(rep).arg(pass));
    ui->recordSlider->setValue(rec);
    trace(QString("(doSliders) rec chnaged to %1").arg(rec));
    ui->replaySlider->setValue(rep);
    trace(QString("(doSliders) rep chnaged to %1").arg(rep));
    ui->passThroughSlider->setValue(pass);
    trace(QString("(doSliders) pass chnaged to %1").arg(pass));

    ui->recordValue->setValue(rec/10.0);
    ui->replayValue->setValue(rep/10.0);
    ui->passThroughValue->setValue(pass/10.0);

    inVolChangeCount--;
}
void KeyerMain::doSetVU( unsigned int ppeakvol, unsigned int prmsvol ,unsigned int psamples)
{
    if (!inhibitCallbacks)
    {
        if (VKMixer::GetVKMixer()->GetCurrentMixerSet() == emsPassThroughNoPTT)
        {
            rmsvol = 0;
            peakvol = 0;
        }
        else
        {
            rmsvol = std::max(rmsvol, prmsvol);
            peakvol = std::max(peakvol, ppeakvol);
        }
        samples += psamples;
        ui->levelMeter->levelChanged( rmsvol / 32768.0, peakvol / 32768.0, samples );
    }
}

KeyerJson *getMasterConfig()
{
    return &keyerMain->masterConfig;
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
    inVolChangeCount(0),
    runner(nullptr)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    connect(&stdinReader, &StdInReader::stdinLine, this, &KeyerMain::onStdInRead);
    stdinReader.start();

    QSettings settings;
    QByteArray geometry = settings.value("KeyerMain/geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    createCloseEvent();

    QSettings keyerSettings( GetCurrentDir() + "/Configuration/MixerSettings.ini" , QSettings::IniFormat ) ;
    QString alsaFileName = keyerSettings.value("AlsaCtlFile", "AlsaCtlFile.txt").toString();
    ui->setupScriptEdit->setText(alsaFileName);

    runAlsaScript(alsaFileName, alsaRestore);

    keyerMain = this;

    connect(SoundSystemDriver::getSbDriver(), &SoundSystemDriver::setVU, this, &KeyerMain::doSetVU);

    commonPort * cp = loadKeyers();
    connect(cp, &commonPort::lcallback, this, &KeyerMain::lcallback);

    setVolumeMults();

    connect(&lineTimer, &QTimer::timeout, this, &KeyerMain::lineTimerTimer);
    lineTimer.start(100);

    // NB CaptionTimer only runs after something changes - the line timer triggers it
    connect(&CaptionTimer, &QTimer::timeout, this, &KeyerMain::CaptionTimerTimer);

    connect(SoundSystemDriver::getSbDriver(), &SoundSystemDriver::ptt, this, &KeyerMain::onPTT);

    KeyerServer::checkConnection();
    connect (KS, &KeyerServer::sliders, this, &KeyerMain::doSliders);
    connect (KS, &KeyerServer::keyerConfig, this, &KeyerMain::doConfig);

    inVolChangeCount++;

    // Initially look in settings...
    int recordLevel = settings.value("RecordLevel", 0).toInt();
    int replayLevel = settings.value("ReplayLevel", 0).toInt();
    int passThroughLevel = settings.value("PassThroughLevel", 0).toInt();

    if (masterConfig.read("./Configuration/MinosKeyer.json"))
    {
        recordLevel = masterConfig.recordSliderPosition;
        replayLevel = masterConfig.replaySliderPosition;
        passThroughLevel = masterConfig.passthroughSliderPosition;
    }

    ui->recordSlider->setValue(recordLevel);
    ui->replaySlider->setValue(replayLevel);
    ui->passThroughSlider->setValue(passThroughLevel);

    inVolChangeCount--;

    ui->PipCheckBox->setChecked(getPipEnabled());

    ui->delayEdit->setValue(getAutoRepeatDelay(0));
    ui->AutoRepeatCheckBox->setChecked( getEnableAutoRepeat(0));

    for (int i = 1; i <= KEYERKEYS; i++)
    {
        ui->keyCombo->addItem(QString::number(i));
    }
    ui->keyCombo->setCurrentIndex(0);
//    on_keyCombo_currentIndexChanged(0);
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
    lineTimer.stop();
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
bool KeyerMain::writeConfig(bool force)
{
    bool ret = true;
    static QString old;
    QString conf = masterConfig.makeConfig(QJsonDocument::Compact, force, true);
    if (force || old != conf)
    {
        ret = masterConfig.write("./Configuration/MinosKeyer.json");
        trace(QString("publishConfig force = %1").arg(force));

        KeyerServer::publishConfig(masterConfig.makeConfig(QJsonDocument::Compact, force, false));

        old = conf;
    }
    return ret;
}
void KeyerMain::onPTT(bool s)
{
    if (currentKeyer)
        currentKeyer->ptt(s);
}
void KeyerMain::lineTimerTimer( )
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
        return;     // closed
    }
    bool show = getShowApp();
    if ( !isVisible() && show )
    {
       setVisible(true);
    }
    if ( isVisible() && !show )
    {
       setVisible(false);
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

   const char *ll = VKMixer::GetVKMixer()->getCurrentLevelText();
   ui->levelLabel->setText(tr(ll));

   QString astate;
   getActionState( astate );

   QString tswitch;
   getTransverterSwitch ( tswitch );

   // This isn't quite what we want - needs to be better english and slower changing

   QString kstatus;
   const char *ms = VKMixer::GetVKMixer()->getCurrentMixerText();
   if ( getKeyerStatus( kstatus ) )
      setWindowTitle(VKMixer::tr( ms ) + " : " + astate + " : " + kstatus + " : " + tswitch);
   else
      setWindowTitle(VKMixer::tr( ms ) + " : " + astate + " : " + tswitch);

   static QString old;

   if ( windowTitle() != old )
   {
      old = windowTitle();
      CaptionTimer.start(200);
   }

   if (currentKeyer)
   {
       masterConfig.recordSliderPosition = ui->recordSlider->value();
       masterConfig.replaySliderPosition = ui->replaySlider->value();
       masterConfig.passthroughSliderPosition = ui->passThroughSlider->value();
       writeConfig(false);

       KeyerServer::publishSliders(ui->recordSlider->value(), ui->replaySlider->value(), ui->passThroughSlider->value());
       if (VKMixer::GetVKMixer()->GetCurrentMixerSet() == emsPassThroughNoPTT)
       {
           doSetVU(0, 0, 0);    // make sure the metering goes to zero when nothing is happening
       }
       KeyerServer::publishVUMeter(rmsvol, peakvol, samples);
       rmsvol = 0;
       peakvol = 0;
       samples = 0;
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
    int fno = ui->keyCombo->currentText().toInt() - 1;
    if ( fno >= 0  && fno < KEYERKEYS )
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
    int fno = ui->keyCombo->currentText().toInt( ) - 1;
    if ( fno >= 0 && fno < KEYERKEYS )
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
    int fno = ui->keyCombo->currentText().toInt( ) - 1;
    setEnableAutoRepeat(fno,  ui->AutoRepeatCheckBox->isChecked() );

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
        int fno = ui->keyCombo->currentText().toInt( ) - 1;
        setAutoRepeatDelay(fno, val );
    }

}
void KeyerMain::on_messageName_editingFinished()
{
    int fno = ui->keyCombo->currentText().toInt( ) - 1;
    setKeyName(fno, ui->messageName->text());

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
        connect (runner, &QProcess::started, this, &KeyerMain::on_started);
        connect (runner, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &KeyerMain::on_finished);
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
        connect (runner, &QProcess::errorOccurred, this, &KeyerMain::on_error);
#else
        connect (runner, &QProcess::error, this, &KeyerMain::on_error);
#endif
        connect (runner, &QProcess::readyReadStandardError, this, &KeyerMain::on_readyReadStandardError);
        connect (runner, &QProcess::readyReadStandardOutput, this, &KeyerMain::on_readyReadStandardOutput);

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
    inVolChangeCount++;

    int record = ui->recordSlider->value();
    int replay = ui->replaySlider->value();
    int passThrough = ui->passThroughSlider->value();
    SoundSystemDriver::getSbDriver()->setVolumeMults(record, replay, passThrough);

    ui->recordValue->setValue(record/10.0);
    ui->replayValue->setValue(replay/10.0);
    ui->passThroughValue->setValue(passThrough/10.0);

    inVolChangeCount--;
}

void KeyerMain::on_recordSlider_valueChanged(int position)
{
    masterConfig.recordSliderPosition = position;
    writeConfig(false);

    setVolumeMults();
}

void KeyerMain::on_replaySlider_valueChanged(int position)
{
    masterConfig.replaySliderPosition = position;
    writeConfig(false);

    setVolumeMults();
}

void KeyerMain::on_passThroughSlider_valueChanged(int position)
{
    masterConfig.passthroughSliderPosition = position;
    writeConfig(false);

    setVolumeMults();
}

void KeyerMain::on_recordValue_valueChanged(double arg1)
{
    if (inVolChangeCount <= 0)
    {
        ui->recordSlider->setValue(static_cast<int>(arg1 * 10));
        trace(QString("(v) record chnaged to %1").arg(arg1));
    }
}

void KeyerMain::on_replayValue_valueChanged(double arg1)
{
    if (inVolChangeCount <= 0)
    {
        ui->replaySlider->setValue(static_cast<int>(arg1 * 10));
        trace(QString("(v) replay chnaged to %1").arg(arg1));
    }
}

void KeyerMain::on_passThroughValue_valueChanged(double arg1)
{
    if (inVolChangeCount <= 0)
    {
        ui->passThroughSlider->setValue(static_cast<int>(arg1 * 10));
        trace(QString("(v) pass chnaged to %1").arg(arg1));
    }
}

void KeyerMain::doConfig(QString config)
{
    if (getMasterConfig()->parseConfig(config, false))
    {
        ui->PipCheckBox->setChecked(getMasterConfig()->pipEnable);

        int fno = ui->keyCombo->currentText().toInt() - 1;
        KeyerKeyJson &kjj = getMasterConfig()->kjj[fno];
        ui->messageName->setText(kjj.CQName);
        ui->AutoRepeatCheckBox->setChecked(kjj.autoRepeat);
        ui->delayEdit->setValue(kjj.autoRepeatDelay);

    }
    else
    {
        writeConfig(true);
        // the value change should cause a force publish. We have to set it back again or it stays there!
        KeyerServer::publishSliders(ui->recordSlider->value(), ui->replaySlider->value() - 1, ui->passThroughSlider->value());
        KeyerServer::publishSliders(ui->recordSlider->value(), ui->replaySlider->value(), ui->passThroughSlider->value());
    }
}
void doConfig(QString config)
{
    keyerMain->doConfig(config);
}

void KeyerMain::on_keyCombo_currentIndexChanged(int /*index*/)
{
    // fill the parameters in the box
    int fno = ui->keyCombo->currentText().toInt() - 1;
    KeyerKeyJson &kjj = getMasterConfig()->kjj[fno];
    ui->messageName->setText(kjj.CQName);
    ui->AutoRepeatCheckBox->setChecked(kjj.autoRepeat);
    ui->delayEdit->setValue(kjj.autoRepeatDelay);
}

