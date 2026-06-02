//#include <QSettings>
//#include <QTimer>
#include <QKeyEvent>
#include "MShowMessageDlg.h"
#include "QtUtils.h"

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
#include "RPCCommandConstants.h"
#include "RPCPubSub.h"
#include "BandList.h"
#include "regsettings.h"
#include "AppStartup.h"
#include "callsign.h"
#include "delayedaction.h"
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
#ifdef Q_OS_WIN
    mmvari + i1, mmvari + i2,
    mmtty + i1, mmtty + i2,
    twotone + i1, twotone + i2,
    gritty + i1, gritty + i2,
#endif
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

    setWindowFlags(windowFlags() & ~(Qt::WindowContextHelpButtonHint | Qt::WindowCloseButtonHint));

    ui->FButtonFrame->setEnabled(false);
    ui->variFrame->setVisible(false);

    connect(&rxBuff, &RxBuffer::newCharacter, this, &EngineWindow::onNewCharacter);

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    fButtons << ui->F1Button << ui->F2Button << ui->F3Button << ui->F4Button << ui->F5Button << ui->F6Button;
    fButtons << ui->F7Button << ui->F8Button << ui->F9Button << ui->F10Button << ui->F11Button << ui->F12Button;

    int i = Qt::Key_F1;
    for (auto b: QASCONST(fButtons))
    {
        b->setProperty("KeyNo", i++);
        b->setText("");
        connect(b, &QPushButton::clicked, this, &EngineWindow::fButtonClicked);
    }

    installEventFilter(this);

    watchDog = new QTimer(this);
    connect(watchDog, &QTimer::timeout, this, &EngineWindow::onWatchdogTimer);

    appName = getAppStartupName();
    MinosRPC *rpc = MinosRPC::getMinosRPC(appName);
    connect(rpc, &MinosRPC::notify, this, &EngineWindow::on_notify);
    RPCPubSub::reconnectPubSub();

    delayedAction(this, [=](){
        ui->rxChars->initialise(this);
        connect(ui->rxChars, &DataPainter::wordSelected, this, &EngineWindow::wordSelected);
        startEngine();
        started = true;
    });

    ui->testString->setText(EngineConfigure::getTestString());
}

EngineWindow::~EngineWindow()
{
    delete ui;
}

int EngineWindow::getRttyOffset()
{
    int rttyOffset = rigCache.getDetails(getSelectedRadio()).getRttyOffset().getValue();
    return rttyOffset;
}

int EngineWindow::getPSKOffset()
{
    int pskOffset = rigCache.getDetails(getSelectedRadio()).getPskOffset().getValue();
    return pskOffset;
}
void EngineWindow::startEngine()
{
    if (engineName.contains(mmvari))
    {
        selectMMVARI(engineName);
    }
    else if (engineName.contains(mmtty))
    {
        selectMMTTY(engineName);
    }
    else if (engineName.contains(twotone))
    {
        select2Tone(engineName);
    }
    else if (engineName.contains(gritty))
    {
        selectGritty(engineName);
    }
    else if (engineName.contains(fldigi))
    {
        selectFLDigi(engineName);
    }
    else if (engineName.contains(test))
    {
        selectTest(engineName);
    }
    emit setSpeeds(EngineConfigure::getSpeed("BPSK"), EngineConfigure::getSpeed("RTTY"));
}

void EngineWindow::selectEngine(QString name)
{
    closeAllEngines();
    engineName = name;
    setWindowTitle(name);

    ui->dataSplitter->setStretchFactor(0, 20);
    ui->dataSplitter->setStretchFactor(1, 1);

    RegSettings settings;
    geoStr = QString("dataModes/%1/geometry").arg(name);
    QByteArray geometry = settings.getSettings().value(geoStr).toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    QByteArray state;
    splitterStr = QString("dataModes/%1/splitter").arg(name);
    state = settings.getSettings().value(splitterStr).toByteArray();
    ui->splitter->restoreState(state);

    dataSplitterStr = QString("dataModes/%1/dataSplitter").arg(name);
    state = settings.getSettings().value(dataSplitterStr).toByteArray();
    ui->dataSplitter->restoreState(state);

}
QString EngineWindow::getSelectedRadio()
{
    return ui->mainRigComboBox->currentData().toString();
}

bool EngineWindow::setSelectedRadio(QString s)
{
    QStringList radios;
    int c = ui->mainRigComboBox->findData(s);
    if (c >= 0)
    {
        ui->mainRigComboBox->setCurrentIndex(c);
    }
    else
    {
        for (int i = 0; i < ui->mainRigComboBox->count(); i++)
        {
            radios.append(ui->mainRigComboBox->itemData(i).toString());
        }
    }
    return c >= 0;
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
        RegSettings settings;
        settings.getSettings().setValue(geoStr, saveGeometry());
        QWidget::moveEvent(event);
    }
}
void EngineWindow::resizeEvent(QResizeEvent * event)
{
    RegSettings settings;
    settings.getSettings().setValue(geoStr, saveGeometry());
    QWidget::resizeEvent(event);
}
void EngineWindow::changeEvent( QEvent* e )
{
    if( e->type() == QEvent::WindowStateChange )
    {
        RegSettings settings;
        settings.getSettings().setValue(geoStr, saveGeometry());
    }
    QDialog::changeEvent(e);

}
QColor EngineWindow::classifyPlaceHolder(QString call)
{
    Callsign cs;
    int res = cs.setFullCall(call);
    if (res == CS_OK)
    {
        Callsign mycall = RemoteLogs::getRemoteLogs()->myCall();
        if (mycall == cs)
        {
            return Qt::darkRed;
        }
        else
        {
            // look for it...
            if (RemoteLogs::getRemoteLogs()->hasWorked(cs, rigBand, rigMode))
            {
                return Qt::gray;
            }
            else
            {
                return Qt::blue;
            }
        }
    }
    return Qt::black;
}

void EngineWindow::setPlaceHolders()
{
    delayedAction(this, [=]()
    {
        // delay the setting to make sure we are in the GUI thread

        ui->placeHolderList->clear();

        if (!placeHolders.isEmpty())
        {
            QStringList placeHoldersList = placeHolders.split(";");

            if (placeHolders.count())
            {
                for (auto &p:QASCONST(placeHoldersList))
                {
                    QStringList pl = p.split("|");
                    ui->placeHolderList->addItem(pl[1]);
                    QColor c = classifyPlaceHolder(pl[1]);
                    ui->placeHolderList->item(ui->placeHolderList->count())
                        ->setForeground(c);
                }
            }
        }
    });
}

void EngineWindow::on_notify(AnalysePubSubNotify an, const QString /*from*/ )
{
    // pubsub notify
//    trace( "Notify callback from " + from + ( !an.getOK() ? ":Error" : ":Normal" ) );

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
                    for (auto const &k: QASCONST(keyarray))
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

            rigMode = value;
            emit rigModeFreq(value, Frequency());
        }
        else if ( an.getCategory() == rpcConstants::rigStateCategory)
        {
            rigCache.setStateString(an);
            mainRig = rigCache.getSelected("");
            setSelectedRadio(mainRig.toString());
        }
        else if ( an.getCategory() == rpcConstants::rigDetailsCategory)
        {
            int rttyOffset = rigCache.getDetails(getSelectedRadio()).getRttyOffset().getValue();
            int pskOffset = rigCache.getDetails(getSelectedRadio()).getPskOffset().getValue();
            rigCache.setDetailsString(an);
            int rttyOffset1 = rigCache.getDetails(getSelectedRadio()).getRttyOffset().getValue();
            int pskOffset1 = rigCache.getDetails(getSelectedRadio()).getPskOffset().getValue();

            if (rttyOffset != rttyOffset1 ||  pskOffset != pskOffset1)
            {
                // offsets have changed, so we need to tell the engines
                emit rigModeFreq(rigMode, rigFreq);

            }
        }
        else if ( an.getCategory() == rpcConstants::rigControlCategory && an.getKey() == rpcConstants::rigControlRadioList )
        {
            rigCache.addRigList(an.getValue());

            ui->mainRigComboBox->clear();

            QStringList cb = populateRig();
            comboSetUniqueNames(cb, ui->mainRigComboBox);
        }
        else if (an.getCategory() == rpcConstants::LoggerCategory && an.getKey() == rpcConstants::placeHolders)
        {
            QString anval = an.getValue();
            placeHolders = anval;
            // ; separated list of "near" calls
            setPlaceHolders();
        }

        RigState &selState = rigCache.getState(mainRig);


        if (selState.radioMode().isDirty() || selState.radioFreq().isDirty())
        {
            rigMode = selState.radioMode().getValue();
            ui->rigMode->setText(rigMode);
            if (rigMode.contains(":"))
            {
                rigMode = rigMode.left(rigMode.indexOf(":"));
            }
            rigFreq = selState.radioFreq().getValue();
            ui->rigFreq->setText(rigFreq.pretty_frequency_MHz_string());

            rigBand = BandList::getBand(rigFreq);

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
    for (const auto &r: QASCONST(rigCache.getRigList()))
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
    started = false;
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

    mmvariFrame = new MMVARIFrame(ui->variFrame, this, /*ui->variFrame, */ui->sendEdit, inId, outId, name);
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

    RegSettings settings;
    settings.getSettings().setValue(geoStr, saveGeometry());


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

void EngineWindow::setWordType(RxLine *rline, int offset, int endword)
{
    RXChar *set = rline->getCharRef(offset);
    for (int i = offset + 1; i < endword; i++)
    {
        RXChar *body = rline->getCharRef(i);
        body->setRST(set->getRST());
        body->setSerial(set->getSerial());
        body->setMyCall(set->getMyCall());
        body->setWorkedCall(set->getWorkedCall());
        body->setUnworkedCall(set->getUnworkedCall());
    }
}
void EngineWindow::onNewCharacter()
{
    if (watchDog->isActive())
    {
        watchDog->start(WATCHDOG_TIME);

    }
    // we now need to parse the line for callsigns, numbers, etc

    int curLine = rxBuff.getCurLine();

    scanLine(curLine);

    ui->rxChars->setText();
}
void EngineWindow::classifyNumeric(RxLine *&rline, int offset, int endword,
                             QString const &w, RXChar *&r) {
    if (w == "599")
    {
        // RST
        r->setRST(true);
        setWordType(rline, offset, endword);
    }
    else if (w.size() <= 4)
    {
        // possible serial number
        r->setSerial(true);
        setWordType(rline, offset, endword);
    }
}
int EngineWindow::classifyCall(RxLine *&rline, int offset, int endword,
                             QString const &w, RXChar *&r)
{
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
        {
            // look for it...
            if (RemoteLogs::getRemoteLogs()->hasWorked(cs, rigBand, rigMode))
            {
                r->setWorkedCall(true);
            }
            else
            {
                r->setUnworkedCall(true);
            }
        }
        setWordType(rline, offset, endword);
    }
    return res;
}
void EngineWindow::scanLine(int curLine) {
    RxLine *rline = rxBuff.getRxLine(curLine);
    rline->clearFlags();
    QString line = rline->toString();
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    static QRegExp separator("[( |\\-|_|)]");
#else
    static QRegularExpression separator("[( |\\-|_|)]");
#endif
    QStringList words = line.split(separator);
    int offset = 0;
    int endword = 0;
    for(const auto &w:QASCONST(words))
    {
        endword = offset + w.size();
        RXChar *r = rline->getCharRef(offset);
        if (!w.isEmpty() && r)
        {
            if (isPureNumeric(w))
            {
                classifyNumeric(rline, offset, endword, w, r);
            }
            else
            {
                // look for callsigns, including our own

                int res = classifyCall(rline, offset, endword, w, r);
                if (res != CS_OK)
                {
                    if (w.indexOf('/') > 0)
                    {
                        QStringList words2 = w.split('/');
                        int woffset = offset;
                        for(const auto &w:QASCONST(words2))
                        {
                            RXChar *r1 = rline->getCharRef(woffset);
                            if (isPureNumeric(w))
                            {
                                classifyNumeric(rline, woffset, woffset + w.size(), w, r1);
                            }
                            else
                            {
                                classifyCall(rline, woffset, woffset + w.size(), w, r1);
                            }
                            woffset += w.size() + 1; // clear word and separator
                        }

                    }
                }
            }
        }
        offset += w.size() + 1; // clear word and separator
    }
}
void EngineWindow::rescan()
{
    if (started)
    {
        for (int i = rxBuff.getCurLine() + 1; i < rxBuff.getLines(); i++)
        {
            scanLine(i);
            rxBuff.getRxLine(i)->setDirty(true);
        }

        for (int i = 0; i <= rxBuff.getCurLine(); i++)
        {
            scanLine(i);
            rxBuff.getRxLine(i)->setDirty(true);
        }
        ui->rxChars->setText();

        setPlaceHolders();
    }
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

void EngineWindow::wordSelected(QString word, int markFreq)
{
// word has been clicked on the datapainter; we need to send it
// on to the logger
    if (!router.isEmpty())
    {
        RPCGeneralClient rpc(rpcConstants::DMWord);
        QSharedPointer<RPCParam>st(new RPCParamStruct);
        st->addMember( word, rpcConstants::DMWord );
        st->addMember( markFreq, rpcConstants::DMMarkFreq );
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
    if (!router.isEmpty())
    {
        RPCGeneralClient rpc(rpcConstants::DMKeyPress);
        QSharedPointer<RPCParam>st(new RPCParamStruct);
        st->addMember( key, rpcConstants::DMFKey );
        rpc.getCallArgs() ->addParam( st );
        rpc.queueCall( rpcConstants::loggerApp + "@" + router );
    }

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
    if (Key == Qt::Key_Escape)
    {
        on_stopButton_clicked();
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


void EngineWindow::on_splitter_splitterMoved(int /*pos*/, int /*index*/)
{
    RegSettings settings;
    QByteArray state = ui->splitter->saveState();
    settings.getSettings().setValue(splitterStr , state);

}
void EngineWindow::on_dataSplitter_splitterMoved(int /*pos*/, int /*index*/)
{
    RegSettings settings;
    QByteArray state = ui->dataSplitter->saveState();
    settings.getSettings().setValue(dataSplitterStr , state);

}
void EngineWindow::on_setTestButton_clicked()
{
    EngineConfigure::setTestString(ui->sendEdit->text().trimmed());
    ui->testString->setText(EngineConfigure::getTestString());
}


void EngineWindow::on_testButton_clicked()
{
    QString data = EngineConfigure::getTestString();
    if (data.isEmpty())
    {
        data = "test";
    }
    ui->sendEdit->setText(data.trimmed());

    doSendButton_clicked(data, 0);

}


void EngineWindow::on_placeHolderList_clicked(const QModelIndex &index)
{
    if (index.isValid())
    {
        QString p = ui->placeHolderList->item(index.row())->text();
        wordSelected(p, rigFreq);
    }
}

