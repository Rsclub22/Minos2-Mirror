#include  <QtGlobal>
#ifdef Q_OS_UNIX
#include <unistd.h>
#endif

#include <QFileDialog>

#include "LogEvents.h"
#include "MTrace.h"
#include "fileutils.h"
#include "keyerAbout.h"
#include "keyctrl.h"
#include "KeyerRPCServer.h"
#include "VKMixer.h"
#include "sbdriver.h"
#include "keyers.h"
#include "portcon.h"
#include "KeyerJson.h"
#include "WaveShowDialog.h"

#include "KeyerMain.h"
#include "ui_KeyerMain.h"

KeyerMain *keyerMain = nullptr;

static QString alsaStore("store");
static QString alsaRestore("restore");
static bool kmInhibitCallbacks = false;


void KeyerMain::lcallback( bool pPTT, bool pPTTRef, bool pL1Ref, bool pL2Ref, int lmode )
{
    if (!kmInhibitCallbacks)
        keyerMain->setLines(pPTT, pPTTRef, pL1Ref, pL2Ref, lmode);
}

void KeyerMain::doSliders(int rec, int rep, int pass, CompressorParams comp)
{
    inVolChangeCount++;
    recordFrame->setIntValue(rec);
    replayFrame->setIntValue(rep);
    passthroughFrame->setIntValue(pass);

    setCompSliders(comp);

    inVolChangeCount--;
}
void KeyerMain::doSetVU( unsigned int ppeakvol, unsigned int prmsvol ,unsigned int psamples)
{
    if (!kmInhibitCallbacks)
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
        ui->levelMeter->levelChanged( peakvol / 32768.0, rmsvol / 32768.0, samples );
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

    // record
    recordFrame = new SliderSpinner(this, tr("\nRecord"), Qt::Vertical, -10, +10, 0);
    ui->volFrame->layout()->addWidget(recordFrame);
    connect(recordFrame, &SliderSpinner::valueChanged, this, &KeyerMain::recordChanged);

    // replay
    replayFrame = new SliderSpinner(this, tr("\nReplay"), Qt::Vertical, -10, +10, 0);
    ui->volFrame->layout()->addWidget(replayFrame);
    connect(replayFrame, &SliderSpinner::valueChanged, this, &KeyerMain::replayChanged);

    // passthrough
    passthroughFrame = new SliderSpinner(this, tr("Pass\nThrough"), Qt::Vertical, -10, +10, 0);
    ui->volFrame->layout()->addWidget(passthroughFrame);
    connect(passthroughFrame, &SliderSpinner::valueChanged, this, &KeyerMain::passthroughChanged);

    ui->compFrame->setLayout(new QVBoxLayout());

    windowFrame = new SliderSpinner(this, tr("Window (ms)"), Qt::Horizontal, 1, +100, 1);
    ui->compFrame->layout()->addWidget(windowFrame);
    connect(windowFrame, &SliderSpinner::valueChanged, this, &KeyerMain::window_valueChanged);

    thresholdFrame = new SliderSpinner(this, tr("Threshold (db below max)"), Qt::Horizontal, -40, 0, 0);
    ui->compFrame->layout()->addWidget(thresholdFrame);
    connect(thresholdFrame, &SliderSpinner::valueChanged, this, &KeyerMain::threshold_valueChanged);

    ratioFrame = new SliderSpinner(this, tr("Compression Ratio"), Qt::Horizontal, 0, +50, 0);
    ui->compFrame->layout()->addWidget(ratioFrame);
    connect(ratioFrame, &SliderSpinner::valueChanged, this, &KeyerMain::ratio_valueChanged);

    attackFrame = new SliderSpinner(this, tr("Attack (ms)"), Qt::Horizontal, 1, 100, 0);
    ui->compFrame->layout()->addWidget(attackFrame);
    connect(attackFrame, &SliderSpinner::valueChanged, this, &KeyerMain::attack_valueChanged);

    releaseFrame = new SliderSpinner(this, tr("Release (ms)"), Qt::Horizontal, 1, 100, 0);
    ui->compFrame->layout()->addWidget(releaseFrame);
    connect(releaseFrame, &SliderSpinner::valueChanged, this, &KeyerMain::release_valueChanged);

    makeUpGainFrame = new SliderSpinner(this, tr("Makeup Gain (db)"), Qt::Horizontal, 0, +20, 0);
    ui->compFrame->layout()->addWidget(makeUpGainFrame);
    connect(makeUpGainFrame, &SliderSpinner::valueChanged, this, &KeyerMain::makeUpGain_valueChanged);


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

    QStringList inputList = SoundSystemDriver::getSbDriver()->getInputDevices();
    QStringList outputList = SoundSystemDriver::getSbDriver()->getOutputDevices();

    ui->inputCombo->addItems(inputList);
    QString currentInput = settings.value("inputDevice", SoundSystemDriver::getSbDriver()->getDefaultInputDevice()).toString();
    ui->inputCombo->setCurrentText(currentInput);

    ui->outputCombo->addItem(tr("Remote IP Client"));
    ui->outputCombo->addItems(outputList);
    QString currentOutput = settings.value("outputDevice", SoundSystemDriver::getSbDriver()->getDefaultOutputDevice()).toString();
    ui->outputCombo->setCurrentText(currentOutput);

    QString port = settings.value("SenderPort", DEFAULT_PORT).toString();
    ui->portEdit->setText(port);
    ui->portEdit->setValidator(new QIntValidator(0, 0xffff, this));

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

    CompressorParams cpar;
    if (masterConfig.read("./Configuration/MinosKeyer.json"))
    {
        recordLevel = masterConfig.recordSliderPosition;
        replayLevel = masterConfig.replaySliderPosition;
        passThroughLevel = masterConfig.passthroughSliderPosition;
    }

    cpar = masterConfig.compression;
    setCompSliders(cpar);
    recordFrame->setIntValue(recordLevel);
    replayFrame->setIntValue(replayLevel);
    passthroughFrame->setIntValue(passThroughLevel);

    inVolChangeCount--;

    ui->PipCheckBox->setChecked(getPipEnabled());

    ui->delayEdit->setValue(getAutoRepeatDelay(0));
    ui->AutoRepeatCheckBox->setChecked( getEnableAutoRepeat(0));

    for (int i = 1; i <= KEYERKEYS; i++)
    {
        ui->keyCombo->addItem(QString::number(i));
    }
    ui->keyCombo->setCurrentIndex(0);
    ui->KeyerTabs->setCurrentIndex(0);

    KeyerServer::publishIPDetail(port);
}
KeyerMain::~KeyerMain()
{
    kmInhibitCallbacks = true;
    delete ui;
}
void KeyerMain::closeEvent(QCloseEvent *event)
{
    kmInhibitCallbacks = true;
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
    //masterConfig.traceConfig();
    bool ret = true;
    static QString old;
    QString conf = masterConfig.makeConfig(QJsonDocument::Compact, force, true);
    if (force || old != conf)
    {
        ret = masterConfig.write("./Configuration/MinosKeyer.json");
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
       masterConfig.recordSliderPosition = recordFrame->getIntValue();
       masterConfig.replaySliderPosition = replayFrame->getIntValue();
       masterConfig.passthroughSliderPosition = passthroughFrame->getIntValue();

       masterConfig.compression = getCompSliders();

       writeConfig(false);

       KeyerServer::publishSliders(recordFrame->getIntValue(), replayFrame->getIntValue(), passthroughFrame->getIntValue(), masterConfig.compression);
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

    int record = recordFrame->getIntValue();
    int replay = replayFrame->getIntValue();
    int passThrough = passthroughFrame->getIntValue();
    CompressorParams cpar = getCompSliders();

    bool doFilter = ui->doFilter->isChecked();
    bool doCompression = ui->doCompression->isChecked();

    SoundSystemDriver::getSbDriver()->setVolumeMults(record, replay, passThrough, cpar, doFilter, doCompression);

    inVolChangeCount--;
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
        KeyerServer::publishSliders(recordFrame->getIntValue(), replayFrame->getIntValue() - 1, passthroughFrame->getIntValue(), masterConfig.compression);
        KeyerServer::publishSliders(recordFrame->getIntValue(), replayFrame->getIntValue(), passthroughFrame->getIntValue(), masterConfig.compression);
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

void KeyerMain::on_showButton_clicked()
{
    int fno = ui->keyCombo->currentText().toInt() - 1;
    WaveShowDialog wsd(this, fno);
    wsd.exec();
}

CompressorParams KeyerMain::getCompSliders()
{
    CompressorParams cp;
    cp.window = windowFrame->getValue();       // milliseconds
    cp.threshold = thresholdFrame->getValue();

    double rrange = ratioFrame->maximum() - ratioFrame->minimum() + 1;
    cp.ratio = 1 - ratioFrame->getValue()/rrange;

    cp.attack = attackFrame->getValue();     // ms
    cp.release = releaseFrame->getValue(); // ms
    cp.makeUpGain = makeUpGainFrame->getValue();

    cp.doCompression = ui->doCompression->isChecked();
    cp.doFilter = ui->doFilter->isChecked();

    return cp;

}

void KeyerMain::setCompSliders(CompressorParams &cp)
{
    windowFrame->setValue(cp.window);       // milliseconds
    thresholdFrame->setValue(cp.threshold);

    double rrange = ratioFrame->maximum() - ratioFrame->minimum() + 1;
    ratioFrame->setValue(rrange * (1 - cp.ratio));

    attackFrame->setValue(cp.attack);     // ms
    releaseFrame->setValue(cp.release); // ms
    makeUpGainFrame->setValue(cp.makeUpGain);

    ui->doCompression->setChecked(cp.doCompression);
    ui->doFilter->setChecked(cp.doFilter);
}


void KeyerMain::window_valueChanged( )
{
    masterConfig.compression.window = windowFrame->getValue();
    setVolumeMults();
    writeConfig(false);
}


void KeyerMain::threshold_valueChanged( )
{
    masterConfig.compression.threshold = thresholdFrame->getValue();
    setVolumeMults();
    writeConfig(false);
}


void KeyerMain::ratio_valueChanged()
{
    double rrange = ratioFrame->maximum() - ratioFrame->minimum() + 1;
    masterConfig.compression.ratio = 1 - ratioFrame->getValue()/rrange;

    setVolumeMults();
    writeConfig(false);
}


void KeyerMain::attack_valueChanged( )
{
    masterConfig.compression.attack = attackFrame->getValue();
    setVolumeMults();
    writeConfig(false);
}


void KeyerMain::release_valueChanged()
{
    masterConfig.compression.release = releaseFrame->getValue();
    setVolumeMults();
    writeConfig(false);
}


void KeyerMain::makeUpGain_valueChanged( )
{
    masterConfig.compression.makeUpGain = makeUpGainFrame->getValue();
    setVolumeMults();
    writeConfig(false);
}

void KeyerMain::recordChanged()
{
    masterConfig.recordSliderPosition = recordFrame->getIntValue();
    setVolumeMults();
    writeConfig(false);
}
void KeyerMain::replayChanged()
{
    masterConfig.replaySliderPosition = replayFrame->getIntValue();
    setVolumeMults();
    writeConfig(false);
}
void KeyerMain::passthroughChanged()
{
    masterConfig.passthroughSliderPosition = passthroughFrame->getIntValue();
    setVolumeMults();
    writeConfig(false);
}


void KeyerMain::on_doFilter_stateChanged(int )
{
    masterConfig.compression.doFilter = ui->doFilter->isChecked();
    setVolumeMults();
    writeConfig(false);
}


void KeyerMain::on_doCompression_stateChanged(int )
{
    masterConfig.compression.doCompression = ui->doCompression->isChecked();
    setVolumeMults();
    writeConfig(false);
}


void KeyerMain::on_inputCombo_activated(int /*index*/)
{
    QSettings settings;
    settings.setValue("inputDevice", ui->inputCombo->currentText());
    trace("About to re-initialise audio");
    SoundSystemDriver::getSbDriver()->closedown();
    SoundSystemDriver::getSbDriver()->initialise(
                ui->inputCombo->currentText()
                , ui->outputCombo->currentText()
                , ui->portEdit->text());
}


void KeyerMain::on_outputCombo_activated(int /*index*/)
{
    QSettings settings;
    settings.setValue("outputDevice", ui->outputCombo->currentText());
    trace("About to re-initialise audio");
    SoundSystemDriver::getSbDriver()->closedown();
    SoundSystemDriver::getSbDriver()->initialise(ui->inputCombo->currentText()
                                                 , ui->outputCombo->currentText()
                                                 , ui->portEdit->text());
}

