
#include "AppStartup.h"
#include "callsign.h"
#include <QSettings>
#include <QTimer>
#include <QKeyEvent>

#ifdef Q_OS_WIN
#include <windows.h>
#include <mmsystem.h>
#endif

#include "MTrace.h"
#include "rxbuffer.h"
#include "cutils.h"
#include "engineconfigure.h"
#include "dmmainwindow.h"
#include "remotelogs.h"
#include "ServerEvent.h"
#include "RPCCommandConstants.h"
#include "RPCPubSub.h"

#include "enginewindow.h"
#include "ui_enginewindow.h"

#define WATCHDOG_TIME 2000

const QString EngineWindow::mmvari = "MMVARI";
const QString EngineWindow::mmtty = "MMTTY";
const QString EngineWindow::twotone = "2Tone";
const QString EngineWindow::gritty = "Gritty";
const QString EngineWindow::fldigi = "FLDigi";
const QString EngineWindow::test = "Test";

const QString EngineWindow::i1("_1");
const QString EngineWindow::i2("_2");

const QStringList EngineWindow::enginesList = {
    mmvari + i1, mmvari + i2,
    mmtty + i1, mmtty + i2,
    twotone + i1, twotone + i2,
    gritty + i1, gritty + i2,
    fldigi + i1, fldigi + i2,
    test + i1, test + i2
};

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

EngineWindow::EngineWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::EngineWindow)
{
    ui->setupUi(this);

    ui->rxChars->initialise(this);

    ui->FButtonFrame->setEnabled(false);
    ui->variFrame->setVisible(false);

    connect(&rxBuff, &RxBuffer::newCharacter, this, &EngineWindow::onNewCharacter);
    connect(ui->rxChars, &DataPainter::wordSelected, this, &EngineWindow::wordSelected);
    connect(&rxBuff, &RxBuffer::newBackLine, this, &EngineWindow::onNewBackLine);

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    fButtons << ui->F1Button << ui->F2Button << ui->F3Button << ui->F4Button << ui->F5Button << ui->F6Button;
    fButtons << ui->F7Button << ui->F8Button << ui->F9Button << ui->F10Button << ui->F11Button << ui->F12Button;

    int i = Qt::Key_F1;
    for (auto b: qAsConst(fButtons))
    {
        b->setProperty("KeyNo", i++);
        b->setText("");
        connect(b, &QPushButton::clicked, this, &EngineWindow::fButtonClicked);
    }

    installEventFilter(this);

    ui->backData->setVisible(false);

    watchDog = new QTimer(this);
    connect(watchDog, &QTimer::timeout, this, &EngineWindow::onWatchdogTimer);

    appName = getAppStartupName();
    MinosRPC *rpc = MinosRPC::getMinosRPC(appName);
    connect(rpc, &MinosRPC::notify, this, &EngineWindow::on_notify);
    RPCPubSub::reconnectPubSub();
}

EngineWindow::~EngineWindow()
{
    delete ui;
}
void EngineWindow::selectEngine(QString name)
{
    closeAllEngines();
    engineName = name;
    setWindowTitle(name);
    if (name.contains(mmvari))
    {
        selectMMVARI(name);
    }
    else if (name.contains(mmtty))
    {
        selectMMTTY(name);
    }
    else if (name.contains(twotone))
    {
        select2Tone(name);
    }
    else if (name.contains(gritty))
    {
        selectGritty(name);
    }
    else if (name.contains(fldigi))
    {
        selectFLDigi(name);
    }
    else if (name.contains(test))
    {
        selectTest(name);
    }
    QSettings settings;
    geoStr = QString("dataModes/%1/geometry").arg(name);
    QByteArray geometry = settings.value(geoStr).toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

}

void EngineWindow::closeEvent(QCloseEvent *event)
{
    doCloseEvent();
    QWidget::closeEvent(event);
}
void EngineWindow::moveEvent(QMoveEvent * event)
{
    if (!geoStr.isEmpty())
    {
        QSettings settings;
        settings.setValue(geoStr, saveGeometry());
        QWidget::moveEvent(event);
    }
}
void EngineWindow::resizeEvent(QResizeEvent * event)
{
    QSettings settings;
    settings.setValue(geoStr, saveGeometry());
    QWidget::resizeEvent(event);
}
void EngineWindow::changeEvent( QEvent* e )
{
    if( e->type() == QEvent::WindowStateChange )
    {
        QSettings settings;
        settings.setValue(geoStr, saveGeometry());
    }
    QDialog::changeEvent(e);

}
void EngineWindow::on_notify(AnalysePubSubNotify an, const QString from )
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

        if ( an.getCategory() == rpcConstants::DMCat && key == rpcConstants::DMFKeys)
        {
            router = an.getPublisherRouter();
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
            mainRig = rigCache.getSelected("");
            ui->mainRigComboBox->setCurrentText(mainRig.toString());
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
            QString rigMode = selState.radioMode().getValue();
            ui->rigMode->setText(rigMode);
            Frequency rigFreq = selState.radioFreq().getValue();
            ui->rigFreq->setText(rigFreq.pretty_frequency_MHz_string());

            emit rigModeFreq(rigMode, rigFreq);

            // NB on RTTY the "real" frequency is the "mark" frequency
            // so using LSB the frequency is rigfreq - markfreq

            // PSK63 is easier - (USB)rigFreq + tone offset

            trace(QString("main frequency changed to %1").arg(rigFreq.traceStr()));

            selState.clearDirty();
        }

    }
}
QStringList EngineWindow::populateRig()
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

void EngineWindow::onNewBackLine(QString s)
{
    ui->backData->addItem(s);
    ui->backData->scrollToBottom();
}

void EngineWindow::onTxChanged(bool ptt)
{
    // start/stop the TX watchdog

    if (ptt)
    {
        watchDog->start(WATCHDOG_TIME);
    }
    else
    {
        watchDog->stop();
    }
}
void EngineWindow::onWatchdogTimer()
{
    watchDog->stop();
    on_stopButton_clicked();
}

void EngineWindow::closeAllEngines()
{
    ui->variFrame->setVisible(false);
#ifdef Q_OS_WIN

    if (mmvariFrame)
    {
        delete mmvariFrame;
        mmvariFrame = nullptr;
    }

    if (mmttyFrame)
    {
        mmttyFrame->closeFrame();

        mmttyFrame->deleteLater();
        mmttyFrame = nullptr;
    }

    if (grittyFrame)
    {
        grittyFrame->closeFrame();

        grittyFrame->deleteLater();
        grittyFrame = nullptr;
    }
#endif
    if (fldigiFrame)
    {
        fldigiFrame->closeFrame();
        fldigiFrame->deleteLater();
        fldigiFrame = nullptr;

    }
    if (testFrame)
    {
        testFrame->closeFrame();
        testFrame->deleteLater();
        testFrame = nullptr;
    }
}
void EngineWindow::selectMMVARI(QString name)
{
#ifdef Q_OS_WIN

    trace("Select MMVARI Engine");

    ui->variFrame->setVisible(true);

    QString idev = EngineConfigure::getEngineSound(name,"input");
    int inId = mainWindow->inDeviceIds[idev];

    QString odev = EngineConfigure::getEngineSound(name, "output");
    int outId = mainWindow->outDeviceIds[odev];

    mmvariFrame = new MMVARIFrame(ui->variFrame, this, ui->variFrame, ui->sendEdit, inId, outId, name);
    ui->sendFrame->setVisible(true);
#endif
}

void EngineWindow::selectMMTTY(QString name)
{
#ifdef Q_OS_WIN
    trace("Select MMTTY Engine");

    QString m = EngineConfigure::getEnginePath(name);

    mmttyFrame = new MMTTYFrame(this, false, ui->sendEdit, m, name);
    ui->sendFrame->setVisible(true);
#endif
}

void EngineWindow::select2Tone(QString name)
{
#ifdef Q_OS_WIN
    trace("Select 2Tone Engine");

    QString m = EngineConfigure::getEnginePath(name);

    mmttyFrame = new MMTTYFrame(this, false, ui->sendEdit, m, name);
    ui->sendFrame->setVisible(true);
#endif
}

void EngineWindow::selectFLDigi(QString name)
{
    trace("Select FLDigi Engine");

    QString m = EngineConfigure::getEnginePath(name);

    fldigiFrame = new FLDigiFrame(this, ui->sendEdit, m, name);
    ui->sendFrame->setVisible(true);
}

void EngineWindow::selectTest(QString name)
{
    QString m = EngineConfigure::getEnginePath(name);

    testFrame = new TestFrame(this, ui->sendEdit, m, name );
    ui->sendFrame->setVisible(true);

    setWindowTitle(name);
}

void EngineWindow::selectGritty(QString name)
{
#ifdef Q_OS_WIN
    trace("Select Gritty Engine");

    QString m = EngineConfigure::getEnginePath(name);

    grittyFrame = new GrittyFrame(this, ui->sendEdit, m, name);

    ui->sendFrame->setVisible(false);
#endif
}

void EngineWindow::doCloseEvent()
{
    closeAllEngines();

    LogTimer.stop();

    // and tidy up all loose ends

    QSettings settings;
    settings.setValue(geoStr, saveGeometry());


    trace("Minos Data Modes Engine Closing");

}

void EngineWindow::on_sendButton_clicked()
{
    QString data = ui->sendEdit->text().trimmed();

    doSendButton_clicked(data, 0);
}
void EngineWindow::doSendButton_clicked(QString d, int c)
{
    emit sendCharactersUp(d, c);

}

void EngineWindow::onNewCharacter()
{
    if (watchDog->isActive())
    {
        watchDog->start(WATCHDOG_TIME);

    }
    // we now need to parse the line for callsigns, numbers, etc

    int curLine = rxBuff.getCurLine();
    RxLine *rline = rxBuff.getRxLine(curLine);
    rline->clearFlags();
    QString line = rline->toString();
    QStringList words = line.split(" ");
    int offset = 0;
    for(const auto &w:qAsConst(words))
    {
        RXChar *r = rline->getCharRef(offset);
        if (r)
        {
            if (w.isEmpty())
            {
                offset++;
            }
            else if (isPureNumeric(w))
            {
                if (w == "599")
                {
                    // RST
                    r->setRST(true);
                }
                else if (w.size() <= 4)
                {
                    // possible serial number
                    r->setSerial(true);
                }
            }
            else
            {
                // look for callsigns, including our own

                Callsign cs;
                int res = cs.setFullCall(w);
                if (res == CS_OK)
                {
                    Callsign mycall = RemoteLogs::getRemoteLogs()->myCall();
                    if (mycall == cs)
                    {
                        r->setMyCall(true);
                    }
                    else
                    // look for it...
                    if (RemoteLogs::getRemoteLogs()->hasWorked(cs, "", ""))
                    {
                        r->setWorkedCall(true);
                    }
                    else
                    {
                        r->setUnworkedCall(true);
                    }
                }
            }
        }
        offset += w.size() + 1;
    }


    ui->rxChars->setText();
}

void EngineWindow::clear()
{
    rxBuff.reset();
    onNewCharacter();
}

void EngineWindow::doSendCharacters(QString d, int c)
{
    // send on down to actual engine

    ui->sendEdit->setText(d);
    emit sendCharactersDown(d, c);

}

void EngineWindow::wordSelected(QString word, int carrier)
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

void EngineWindow::fButtonClicked()
{
    QPushButton *b = dynamic_cast<QPushButton *>(sender());
    int kno = b->property("KeyNo").toInt();
    fKey(kno);
}

void EngineWindow::fKey(int key)
{
    RPCGeneralClient rpc(rpcConstants::DMKeyPress);
    QSharedPointer<RPCParam>st(new RPCParamStruct);
    st->addMember( key, rpcConstants::DMFKey );
    rpc.getCallArgs() ->addParam( st );
    rpc.queueCall( rpcConstants::loggerApp + "@" + router );

}
bool EngineWindow::eventFilter(QObject */*obj*/, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *ke = dynamic_cast<QKeyEvent *>(event);
        return doKeyPressEvent(ke);
    }
    return false;
}
bool EngineWindow::doKeyPressEvent( QKeyEvent* event )
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

void EngineWindow::on_stopButton_clicked()
{
    trace("Stop button clicked");
    ui->sendEdit->clear();
    doSendButton_clicked("", 0);
}

void EngineWindow::on_backDataButton_clicked()
{
    if (ui->backData->isVisible())
    {
        ui->backDataButton->setText(tr("Show Back Data"));
        ui->backData->setVisible(false);
    }
    else
    {
        ui->backDataButton->setText(tr("Hide Back Data"));
        ui->backData->setVisible(true);
    }
}

//void EngineWindow::on_actionBPSK31_triggered()
//{
//    ui->actionBPSK63->setChecked(false);
//    EngineConfigure::setSpeed("BPSK", 31);
//}


//void EngineWindow::on_actionBPSK63_triggered()
//{
//    ui->actionBPSK31->setChecked(false);
//    EngineConfigure::setSpeed("BPSK", 63);
//}


//void EngineWindow::on_actionRTTY45_triggered()
//{
//    ui->actionRTTY75->setChecked(false);
//    EngineConfigure::setSpeed("RTTY", 45);
//}


//void EngineWindow::on_actionRTTY75_triggered()
//{
//    ui->actionRTTY45->setChecked(false);
//    EngineConfigure::setSpeed("RTTY", 75);
//}

