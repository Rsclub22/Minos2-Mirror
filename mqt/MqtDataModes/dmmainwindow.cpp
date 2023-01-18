#include <QSettings>
#include <QTimer>
#include <QFileSystemWatcher>
#include <QKeyEvent>

#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>


#ifdef Q_OS_WIN
#include <windows.h>
#include <mmsystem.h>
#endif

#include "AppStartup.h"
#include "MinosRPC.h"
#include "LogEvents.h"
#include "MTrace.h"
#include "fileutils.h"
#include "waitcursor.h"
#include "engineconfigure.h"
#include "rxbuffer.h"
#include "RPCCommandConstants.h"
#include "ServerEvent.h"
#include "delayedaction.h"

#include "dmmainwindow.h"
#include "ui_dmmainwindow.h"

DMMainWindow *mainWindow = nullptr;

/*

To Do

Analysis and colouring of callsigns etc
? callsign stack

Frequency following and notification - can we even do this?

PSK as well as RTTY; any other modes? (FLDigi supports lots!) and its configuration
Mode selection from logger? RY and PS modes (RSGB required). Cabrillo just has "DG", Reg1Test just has
7 - RTTY-MGM

Contest definitions will give us RY/PS (don't know about BARTG!). How do we set the engine correctly?

MMVARI - buttons and menu

FLDigi - read characters

configuration and running multiple copies of engines - each app should have its own set of INI files
app name in ini file section

*/

const QString DMMainWindow::mmvari = "MMVARI";
const QString DMMainWindow::mmtty = "MMTTY";
const QString DMMainWindow::twotone = "2Tone";
const QString DMMainWindow::gritty = "Gritty";
const QString DMMainWindow::fldigi = "FLDigi";
const QString DMMainWindow::test = "Test";

DMMainWindow::DMMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::DMMainWindow)
{
    ui->setupUi(this);

    mainWindow = this;

    ui->FButtonFrame->setEnabled(false);
    ui->variFrame->setVisible(false);

    connect(RxBuffer::getRxBuffer(), &RxBuffer::newCharacter, this, &DMMainWindow::onNewCharacter);
    connect(ui->rxChars, &DataPainter::wordSelected, this, &DMMainWindow::wordSelected);
#ifdef Q_OS_WIN
    UINT devs = waveInGetNumDevs();
    inChannels = devs;
    QString defname = tr("Default Device");
    inputDevices.append(defname);
    inDeviceIds[defname] = -1;
    for (UINT dev = 0; dev < devs; dev++) {
        WAVEINCAPS caps = {};
        MMRESULT mmr = waveInGetDevCaps(dev, &caps, sizeof(caps));
        if (MMSYSERR_NOERROR != mmr) {
            return /*mmr*/;
        }

        QString name = QString::fromWCharArray(caps.szPname);
        inputDevices.append(name);
        inDeviceIds[name] = dev;
        trace( "input device = "  + QString::number(dev) +  " " + name);
    }
    devs = waveOutGetNumDevs();
    outChannels = devs;
    outputDevices.append(defname);
    outDeviceIds[defname] = -1;        // repeated...
    for (UINT dev = 0; dev < devs; dev++) {
        WAVEOUTCAPS caps = {};
        MMRESULT mmr = waveOutGetDevCaps(dev, &caps, sizeof(caps));

        if (MMSYSERR_NOERROR != mmr) {
            return /*mmr*/;
        }
        QString name = QString::fromWCharArray(caps.szPname);
        outputDevices.append(name);
        outDeviceIds[name] = dev;
        trace( "output device = "  + QString::number(dev) +  " " + name);
    }

#endif

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    connect(stdinReader, &StdInReader::stdinLine, this, &DMMainWindow::onStdInRead);

    appName = getAppStartupName();
    MinosRPC *rpc = MinosRPC::getMinosRPC(appName);
    connect(rpc, &MinosRPC::routerCall, this, &DMMainWindow::on_routerCall);
    connect(rpc, &MinosRPC::notify, this, &DMMainWindow::on_notify);
    //connect(rpc, &MinosRPC::provider, this, &DMMainWindow::on_provider);

    QStringList svr = {rpcConstants::rigControlCategory
                      ,rpcConstants::rigDetailsCategory
                      ,rpcConstants::rigStateCategory
                     };
    rpc->initialiseRouters(svr);

    QStringList svs = {rpcConstants::DMSender};
    rpc->findProviders(rpcConstants::DMCat, svs);

    router = MinosConfig::getMinosConfig()->getThisRouterName();

    QString a = rpc->getAppName();
    me = a + "@" + router;

    createCloseEvent();

    connect(&LogTimer, &QTimer::timeout, this, &DMMainWindow::LogTimerTimer);
    LogTimer.start(100);

    QSettings settings;
    geoStr = QString("dataModes/geometry");
    QByteArray geometry = settings.value(geoStr).toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    clearAction = newAction("Clear Decodes", ui->menuClear, &DMMainWindow::onMenuClear);

    actionConfigure_Engines = newAction(QT_TR_NOOP("Configure Engines"), ui->menuEngine, &DMMainWindow::onActionConfigure_Engines_triggered);

    ui->menuEngine->addSeparator();

#ifdef Q_OS_WIN
    QString exePath = QCoreApplication::applicationDirPath() + QString("/MMVARI.ocx");
    EngineConfigure::setEnginePath(mmvari, exePath);

    actionMMVARI = newCheckableAction(mmvari, ui->menuEngine, &DMMainWindow::onActionMMVARI_triggered);
    action2Tone = newCheckableAction(twotone, ui->menuEngine, &DMMainWindow::onAction2Tone_triggered);
    actionMMTTY = newCheckableAction(mmtty, ui->menuEngine, &DMMainWindow::onActionMMTTY_triggered);
    actionGritty = newCheckableAction(gritty, ui->menuEngine, &DMMainWindow::onActionGritty_triggered);
#endif
    actionFLDigi = newCheckableAction(fldigi, ui->menuEngine, &DMMainWindow::onActionFLDigi_triggered);
    actionTest = newCheckableAction(test, ui->menuEngine, &DMMainWindow::onActionTest_triggered);


    checkEnginesAvailable();

    QString sender = settings.value("Sender").toString();
    if (sender == me)
    {
        ui->sendercb->setChecked(true);
    }
    qfsw = new QFileSystemWatcher(this);
    qfsw->addPath("./Configuration/DataModes.ini");
    connect(qfsw, &QFileSystemWatcher::fileChanged, this, &DMMainWindow::iniFileChanged);


    fButtons << ui->F1Button << ui->F2Button << ui->F3Button << ui->F4Button << ui->F5Button << ui->F6Button;
    fButtons << ui->F7Button << ui->F8Button << ui->F9Button << ui->F10Button << ui->F11Button << ui->F12Button;

    int i = Qt::Key_F1;
    for (auto b: qAsConst(fButtons))
    {
        b->setProperty("KeyNo", i++);
        b->setText("");
        connect(b, &QPushButton::clicked, this, &DMMainWindow::fButtonClicked);
    }

    installEventFilter(this);

    baseTitle = tr("Minos Data Modes");
    setWindowTitle(getAppStartupName() + ": " + baseTitle);

    startPreviousEngine();

    mainRig = getRig();
}

DMMainWindow::~DMMainWindow()
{
    delete ui;
}
void DMMainWindow::startPreviousEngine()
{
    // If you do this in the constructor, the menu stops responding
    // so delay it until the event loop is running

    delayedAction(this, [=]()
    {
        QString active = EngineConfigure::getAppCurrent();

#ifdef Q_OS_WIN
        if (active == mmvari)
        {
            onActionMMVARI_triggered(true);
        }
        else if (active == mmtty)
        {
            onActionMMTTY_triggered(true);
        }
        else if (active == twotone)
        {
            onAction2Tone_triggered(true);
        }
        else if (active == gritty)
        {
            onActionGritty_triggered(true);
        }
        else
#endif
        if (active == fldigi)
        {
            onActionFLDigi_triggered(true);
        }
        else if (active == test)
        {
            onActionTest_triggered(true);
        }
    }
    );
}

void DMMainWindow::on_routerCall(bool err, QSharedPointer<MinosRPCObj>mro, const QString /*from*/ )
{
    if ( !err )
    {
        QString call = mro->getMethodName();
        if (call == rpcConstants::DMTransmit)
        {

            RPCArgs *args = mro->getCallArgs();

            if (args)
            {
                QSharedPointer<RPCParam> psMess;
                QSharedPointer<RPCParam> piCarr;

                if (
                        args->getStructArgMember(0, rpcConstants::DMTransmit, psMess)
                        && args->getStructArgMember(0, rpcConstants::DMCarrier, piCarr)
                        )
                {
                    QString pmess;
                    int carr;
                    if (psMess->getString(pmess) && piCarr->getInt(carr))
                    {
                        ui->sendEdit->setText(pmess);
                        if (!pmess.isEmpty())
                        {
                            doSendButton_clicked(pmess, carr);
                        }
                    }
                }
            }
        }
        else if (call == rpcConstants::DMStopTransmit)
        {
            ui->sendEdit->clear();
            doSendButton_clicked("", 0);
        }
    }
}
void DMMainWindow::on_notify(AnalysePubSubNotify an, const QString from )
{
    // pubsub notify
    trace( "Notify callback from " + from + ( !an.getOK() ? ":Error" : ":Normal" ) );

    if ( an.getOK() )
    {
        PublishState state = an.getState();
        if (state != psPublished)
        {
            return;
        }
        QString key = an.getKey();
        QString value = an.getValue();

        if ( an.getCategory() == rpcConstants::DMCat && key == rpcConstants::DMSender)
        {
            if (value != me)
            {
                ui->sendercb->setChecked(false);
                ui->sendEdit->clear();
            }
        }
        else if ( an.getCategory() == rpcConstants::DMCat && key == rpcConstants::DMFKeys)
        {
            QJsonParseError err;
            QJsonDocument json = QJsonDocument::fromJson(value.toUtf8(), &err);
            if (!err.error)
            {
                if( json.isArray())
                {
                    int i = 0;
                    QJsonArray keyarray = json.array();
                    for (auto const &k: qAsConst(keyarray))
                    {
                        if (i >= 12)
                        {
                            break;
                        }
                        QJsonObject ko = k.toObject();
                        QString f = ko.value(QString("F%1").arg(i + 1)).toString();
                        fButtons[i]->setText(f);
                        i++;
                    }
                    ui->FButtonFrame->setEnabled(true);
                }
            }
        }
        else if ( an.getCategory() == rpcConstants::DMCat && key == rpcConstants::DMMode)
        {
            // send the new mode to our engine

            emit rigModeFreq(value, Frequency());
        }
        else if ( an.getCategory() == rpcConstants::rigStateCategory)
        {
            rigCache.setStateString(an);
        }
        else if ( an.getCategory() == rpcConstants::rigDetailsCategory)
        {
            rigCache.setDetailsString(an);
        }
        else if ( an.getCategory() == rpcConstants::rigControlCategory && an.getKey() == rpcConstants::rigControlRadioList )
        {
            rigCache.addRigList(an.getValue());

            ui->mainRigComboBox->clear();

            QStringList cb = populateRig();
            ui->mainRigComboBox->clear();
            ui->mainRigComboBox->addItems(cb);
            ui->mainRigComboBox->setCurrentText(mainRig.toString());
        }

        RigState &selState = rigCache.getState(mainRig);

        if (selState.radioMode().isDirty() || selState.radioFreq().isDirty())
        {
            QString rigMode = selState.radioMode().getValue().remove(":");
            ui->rigMode->setText(rigMode);
            Frequency rigFreq = selState.radioFreq().getValue();
            ui->rigFreq->setText(rigFreq.pretty_frequency_MHz_string());

            emit rigModeFreq(rigMode, rigFreq);

            // NB on RTTY the "real" frewquncy is the "mark" frequency
            // so using LSB the frequency is rigfreq - markfreq

            // PSK63 is easier - (USB)rigFreq + tone offset

            trace(QString("main frequency changed to %1").arg(rigFreq.traceStr()));

            selState.clearDirty();
        }

    }
}
QStringList DMMainWindow::populateRig()
{
    QStringList cb;
    cb.append("");
    for (const auto &r: qAsConst(rigCache.getRigList()))
    {
        if (!r.isEmpty() )
        {
            cb.append( r.toString());
        }
    }
    cb.removeDuplicates();
    cb.sort();
    return cb;
}
void DMMainWindow::configureRig(const QString s)
{
    QSettings config;

    config.setValue("Rig", s);
}

QString DMMainWindow::getRig()
{
    QSettings config;

    return config.value("Rig").toString();
}

void DMMainWindow::iniFileChanged()
{
    // as other sister apps may have changed it
    checkEnginesAvailable();
}


void DMMainWindow::checkEnginesAvailable()
{
    QString m;
    bool b = false;

#ifdef Q_OS_WIN
    m = EngineConfigure::getEnginePath(mmvari);
    b = FileExists(m);
    actionMMVARI->setEnabled(b);

    m = EngineConfigure::getEnginePath(twotone);
    b = FileExists(m);
    action2Tone->setEnabled(b);

    m = EngineConfigure::getEnginePath(mmtty);
    b = FileExists(m);
    actionMMTTY->setEnabled(b);

    m = EngineConfigure::getEnginePath(gritty);
    b = FileExists(m);
    actionGritty->setEnabled(b);
#endif
    m = EngineConfigure::getEnginePath(fldigi);
    b = FileExists(m);
    actionFLDigi->setEnabled(b);

}
QMenu *DMMainWindow::newMenu(QMenu *m, const char *text)
{
    QMenu *menu = m->addMenu(tr(text));
    menuList[menu] = text;
    return menu;
}
QAction *DMMainWindow::newAction(const char *text, QMenu *m, void (DMMainWindow::*slotparam)() )
{
    QAction * newAct = new QAction( tr(text), this );
    actionList[newAct] = text;
    m->addAction( newAct );
    if (slotparam)
    {
        connect( newAct, &QAction::triggered, this, slotparam );
    }
    return newAct;
}
QAction *DMMainWindow::newCheckableAction( const char *text, QMenu *m, void (DMMainWindow::*slotparam)(bool) )
{
    QAction * newAct = new QAction( tr(text), this );
    actionList[newAct] = text;
    newAct->setCheckable( true );
    m->addAction( newAct );
    if (slotparam)
    {
        connect( newAct, &QAction::triggered, this, slotparam );
    }
    return newAct;
}
QAction *DMMainWindow::newCheckableAction(const QString text, QMenu *m, void (DMMainWindow::*slotparam)(bool) )
{
    QAction * newAct = new QAction( text, this );
    newAct->setCheckable( true );
    m->addAction( newAct );
    if (slotparam)
    {
        connect( newAct, &QAction::triggered, this, slotparam );
    }
    return newAct;
}

void DMMainWindow::LogTimerTimer()
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
void DMMainWindow::closeEvent(QCloseEvent *event)
{
    doCloseEvent();
    QWidget::closeEvent(event);
}
void DMMainWindow::moveEvent(QMoveEvent * event)
{
    QSettings settings;
    settings.setValue(geoStr, saveGeometry());
    QWidget::moveEvent(event);
}
void DMMainWindow::resizeEvent(QResizeEvent * event)
{
    QSettings settings;
    settings.setValue(geoStr, saveGeometry());
    QWidget::resizeEvent(event);
}
void DMMainWindow::changeEvent( QEvent* e )
{
    if( e->type() == QEvent::WindowStateChange )
    {
        QSettings settings;
        settings.setValue(geoStr, saveGeometry());
    }
    if (e->type() == QEvent::LanguageChange)
    {
        // when language changes force a complete rebuild
        TWaitCursor wc(this);

        for(QMap<QMenu *, const char *>::iterator i = menuList.begin(); i != menuList.end(); i++)
        {
            i.key()->setTitle(tr(i.value()));
        }
        for(QMap<QAction *, const char *>::iterator i = actionList.begin(); i != actionList.end(); i++)
        {
            i.key()->setText(tr(i.value()));
        }
        ui->retranslateUi(this);
        baseTitle = tr("Minos Data Modes");
        setWindowTitle(getAppStartupName() + ": " + baseTitle);
    }
    QMainWindow::changeEvent(e);

}

void DMMainWindow::onStdInRead(QString cmd)
{
    if (cmd.indexOf("Shutdown", 0, Qt::CaseInsensitive) >= 0)
    {
        close();
    }
}
void DMMainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);


}
void DMMainWindow::closeAllEngines(bool clearCurrent)
{
    ui->variFrame->setVisible(false);
#ifdef Q_OS_WIN

    if (mmvariFrame)
    {
        delete mmvariFrame;
        mmvariFrame = nullptr;
        actionMMVARI->setChecked(false);
    }

    actionMMVARI->setChecked(false);

    if (mmttyFrame)
    {
        mmttyFrame->closeFrame();

        mmttyFrame->deleteLater();
        mmttyFrame = nullptr;
        actionMMTTY->setChecked(false);
        action2Tone->setChecked(false);
    }

    if (grittyFrame)
    {
        grittyFrame->closeFrame();

        grittyFrame->deleteLater();
        grittyFrame = nullptr;
        actionGritty->setChecked(false);
    }
#endif
    if (fldigiFrame)
    {
        fldigiFrame->closeFrame();
        fldigiFrame->deleteLater();
        fldigiFrame = nullptr;
        actionFLDigi->setChecked(false);

    }
    if (testFrame)
    {
        testFrame->closeFrame();
        testFrame->deleteLater();
        testFrame = nullptr;
        actionTest->setChecked(false);
    }
    if (clearCurrent)
    {
        EngineConfigure::setAppCurrent(QString());
    }
    setWindowTitle(getAppStartupName() + ": " + baseTitle);
}
#ifdef Q_OS_WIN

void DMMainWindow::onActionMMVARI_triggered(bool checked)
{
    closeAllEngines(true);

    if (!checked)
    {
        return;
    }

    trace("Select MMVARI Engine");

    ui->variFrame->setVisible(true);

    QString idev = EngineConfigure::getAppPath(mmvari + "/input");
    int inId = inDeviceIds[idev];

    QString odev = EngineConfigure::getAppPath(mmvari + "/output");
    int outId = outDeviceIds[odev];

    mmvariFrame = new MMVARIFrame(this, ui->variFrame, ui->sendEdit, inId, outId);
    actionMMVARI->setChecked(true);
    EngineConfigure::setAppCurrent(mmvari);
    ui->sendFrame->setVisible(true);

    setWindowTitle(getAppStartupName() + ": " + baseTitle + ": " + mmvari);

}

void DMMainWindow::onActionMMTTY_triggered(bool checked)
{
    closeAllEngines(true);

    if (!checked)
    {
        return;
    }

    trace("Select MMTTY Engine");

    QString m = EngineConfigure::getEnginePath(mmtty);

    mmttyFrame = new MMTTYFrame(this, false, ui->sendEdit, m);
    actionMMTTY->setChecked(true);
    EngineConfigure::setAppCurrent(mmtty);
    ui->sendFrame->setVisible(true);
    setWindowTitle(getAppStartupName() + ": " + baseTitle + ": " + mmtty);
}

void DMMainWindow::onAction2Tone_triggered(bool checked)
{
    closeAllEngines(true);

    if (!checked)
    {
        return;
    }

    trace("Select 2Tone Engine");

    QString m = EngineConfigure::getEnginePath(twotone);

    mmttyFrame = new MMTTYFrame(this, false, ui->sendEdit, m);
    action2Tone->setChecked(true);
    EngineConfigure::setAppCurrent(twotone);
    ui->sendFrame->setVisible(true);
    setWindowTitle(getAppStartupName() + ": " + baseTitle + ": " + twotone);
}
#endif
void DMMainWindow::onActionFLDigi_triggered(bool /*checked*/)
{
    closeAllEngines(true);
    trace("Select FLDigi Engine");

    QString m = EngineConfigure::getEnginePath(fldigi);

    fldigiFrame = new FLDigiFrame(this, ui->sendEdit, m);
    actionFLDigi->setChecked(true);
    EngineConfigure::setAppCurrent(fldigi);
    ui->sendFrame->setVisible(true);
    setWindowTitle(getAppStartupName() + ": " + baseTitle + ": " + fldigi);
}

void DMMainWindow::onActionTest_triggered(bool checked)
{
    closeAllEngines(true);

    if (!checked)
    {
        return;
    }

    QString m = EngineConfigure::getEnginePath(test);

    testFrame = new TestFrame(this, ui->sendEdit, m);
    actionTest->setChecked(true);
    EngineConfigure::setAppCurrent(test);
    ui->sendFrame->setVisible(true);
    setWindowTitle(getAppStartupName() + ": " + baseTitle + ": " + test);
}
#ifdef Q_OS_WIN

void DMMainWindow::onActionGritty_triggered(bool checked)
{
    closeAllEngines(true);

    if (!checked)
    {
        return;
    }

    trace("Select Gritty Engine");

    QString m = EngineConfigure::getEnginePath(gritty);

    grittyFrame = new GrittyFrame(this, ui->sendEdit, m);
    actionGritty->setChecked(true);
    EngineConfigure::setAppCurrent(gritty);

    ui->sendFrame->setVisible(false);
    ui->sendercb->setChecked(false);
    setWindowTitle(getAppStartupName() + ": " + baseTitle + ": " + gritty);

}
#endif
void DMMainWindow::onActionExit_triggered()
{
    close();
}
void DMMainWindow::doCloseEvent()
{
    closeAllEngines(false);

    LogTimer.stop();

    // and tidy up all loose ends

    QSettings settings;
    settings.setValue(geoStr, saveGeometry());


    trace("Minos Data Modes App Closing");

}

void DMMainWindow::on_sendButton_clicked()
{
    QString data = ui->sendEdit->text().trimmed();

    doSendButton_clicked(data, 0);
}
void DMMainWindow::doSendButton_clicked(QString d, int c)
{
    emit sendCharacters(d, c);

}
void DMMainWindow::onActionConfigure_Engines_triggered()
{
    EngineConfigure ec(this);
    ec.exec();

    checkEnginesAvailable();
}

void DMMainWindow::onNewCharacter()
{
    ui->rxChars->setText();
}

void DMMainWindow::onMenuClear()
{
    RxBuffer::getRxBuffer()->reset();
    onNewCharacter();
}

void DMMainWindow::wordSelected(QString word, int carrier)
{
// word has been clicked on the datapainter; we need to send it
// on to the logger
    bool routerRunning = checkRouterReady();

    if (routerRunning)
    {
        QString router = MinosConfig::getMinosConfig( )->getThisRouterName();

        RPCGeneralClient rpc(rpcConstants::DMWord);
        QSharedPointer<RPCParam>st(new RPCParamStruct);
        st->addMember( word, rpcConstants::DMWord );
        st->addMember( carrier, rpcConstants::DMCarrier );
        rpc.getCallArgs() ->addParam( st );
        rpc.queueCall( rpcConstants::loggerApp + "@" + router );
    }
}

void DMMainWindow::on_sendercb_stateChanged(int /*arg1*/)
{
    MinosRPC *rpc = MinosRPC::getMinosRPC();
    if (ui->sendercb->isChecked())
    {
        // publish ourself as sender
        QSettings settings;
        settings.setValue("Sender", me);

        rpc->publish(rpcConstants::DMCat, rpcConstants::DMSender, me, psPublished);

        ui->sendercb->setEnabled(false);    // you aren't allowed to uncheck it yourself
    }
    else
    {
        QSettings settings;
        settings.setValue("Sender", QString());
        rpc->publish(rpcConstants::DMCat, rpcConstants::DMSender, "not" + me, psRevoked);

        ui->sendercb->setEnabled(true);    // you aren't allowed to uncheck it yourself
    }
}

void DMMainWindow::fButtonClicked()
{
    QPushButton *b = dynamic_cast<QPushButton *>(sender());
    int kno = b->property("KeyNo").toInt();
    fKey(kno);
}

void DMMainWindow::fKey(int key)
{
    RPCGeneralClient rpc(rpcConstants::DMKeyPress);
    QSharedPointer<RPCParam>st(new RPCParamStruct);
    st->addMember( key, rpcConstants::DMFKey );
    rpc.getCallArgs() ->addParam( st );
    rpc.queueCall( rpcConstants::loggerApp + "@" + router );

}
bool DMMainWindow::eventFilter(QObject */*obj*/, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *ke = dynamic_cast<QKeyEvent *>(event);
        return doKeyPressEvent(ke);
    }
    return false;
}
bool DMMainWindow::doKeyPressEvent( QKeyEvent* event )
{
    if (!event)
        return false;

    int Key = event->key();

//    Qt::KeyboardModifiers mods = event->modifiers();
//    bool shift = mods & Qt::ShiftModifier;
//    bool ctrl = mods & Qt::ControlModifier;
//    bool alt = mods & Qt::AltModifier;

    if (Key >= Qt::Key_F1 && Key <= Qt::Key_F12)
    {
        fKey(Key);
        return true;
    }
    return false;
}

void DMMainWindow::on_stopButton_clicked()
{
    trace("Stop button clicked");
    ui->sendEdit->clear();
    doSendButton_clicked("", 0);
}


void DMMainWindow::on_mainRigComboBox_activated(const QString &psn)
{
    mainRig = psn;
    configureRig(psn);
}

