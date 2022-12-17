#include <QSettings>
#include <QTimer>

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

#include "dmmainwindow.h"
#include "ui_dmmainwindow.h"

/*

To Do

Analysis and colouring of callsigns etc
? callsign stack

Frequency following and notification

TX - select which app/window does TX, and drive it,
get TX text from Minos and send it

Collection of QSO parts and transfer to Minos

TX messages and their maintenance

F keys - do we implement here or in Minos Logger?
Here allows stand-alone use...
Pass F keys from any of our windows to Minos
(which will then trigger TX messages)

PSK as well as RTTY; any other modes? (FLDigi supports lots!)

configuration and running multiple copies of engines

auto-start "my" engine on load

MMVARI - buttons and menu

FLDigi - read characters

*/

DMMainWindow::DMMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::DMMainWindow)
{
    ui->setupUi(this);

    connect(RxBuffer::getRxBuffer(), &RxBuffer::newCharacter, this, &DMMainWindow::onNewCharacter);
    connect(ui->rxChars, &DataPainter::wordSelected, this, &DMMainWindow::wordSelected);
#ifdef Q_OS_WIN
    UINT devs = waveInGetNumDevs();
    inChannels = devs;
    QString defname = tr("Default Device");
    inputDevices.append(defname);
    deviceIds[defname] = -1;
    for (UINT dev = 0; dev < devs; dev++) {
        WAVEINCAPS caps = {};
        MMRESULT mmr = waveInGetDevCaps(dev, &caps, sizeof(caps));
        if (MMSYSERR_NOERROR != mmr) {
            return /*mmr*/;
        }

        QString name = QString::fromWCharArray(caps.szPname);
        inputDevices.append(name);
        deviceIds[name] = dev;
        trace( "input device = "  + QString::number(devs) +  " " + name);
    }
    devs = waveOutGetNumDevs();
    outChannels = devs;
    outputDevices.append(defname);
    deviceIds[defname] = -1;        // repeated...
    for (UINT dev = 0; dev < devs; dev++) {
        WAVEOUTCAPS caps = {};
        MMRESULT mmr = waveOutGetDevCaps(dev, &caps, sizeof(caps));

        if (MMSYSERR_NOERROR != mmr) {
            return /*mmr*/;
        }
        QString name = QString::fromWCharArray(caps.szPname);
        outputDevices.append(name);
        deviceIds[name] = dev;
        trace( "output device = "  + QString::number(devs) +  " " + name);
    }

#endif

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    connect(stdinReader, &StdInReader::stdinLine, this, &DMMainWindow::onStdInRead);

    MinosRPC *rpc = MinosRPC::getMinosRPC(getAppStartupName());
    Q_UNUSED(rpc)

    createCloseEvent();

    connect(&LogTimer, &QTimer::timeout, this, &DMMainWindow::LogTimerTimer);
    LogTimer.start(100);

    QSettings settings;
    geoStr = QString("dataModes/geometry");
    QByteArray geometry = settings.value(geoStr).toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    clearAction = newAction("Clear Decodes", ui->menuClear, &DMMainWindow::onMenuClear);

#ifdef Q_OS_WIN
    actionMMVARI = newCheckableAction("MMVARI", ui->menuEngine, &DMMainWindow::onActionMMVARI_triggered);
    action2Tone = newCheckableAction("2Tone", ui->menuEngine, &DMMainWindow::onAction2Tone_triggered);
    actionMMTTY = newCheckableAction("MMTTY", ui->menuEngine, &DMMainWindow::onActionMMTTY_triggered);
    actionGritty = newCheckableAction("Gritty", ui->menuEngine, &DMMainWindow::onActionGritty_triggered);
#endif
    actionFLDigi = newCheckableAction("FLDigi", ui->menuEngine, &DMMainWindow::onActionFLDigi_triggered);
    actionTest = newCheckableAction("Test", ui->menuEngine, &DMMainWindow::onActionTest_triggered);

    actionConfigure_Engines = newAction(QT_TR_NOOP("Configure Engines"), ui->menuConfigure, &DMMainWindow::onActionConfigure_Engines_triggered);

    checkEnginesAvailable();
}

DMMainWindow::~DMMainWindow()
{
    delete ui;
}
void DMMainWindow::checkEnginesAvailable()
{
    QSettings settings;
    QString eStr = QString("dataModes/engines/");

    //QString m = QCoreApplication::applicationDirPath() + "/MMVARI.ocx";
    QString m;
    bool b = false;

#ifdef Q_OS_WIN
    m = settings.value(eStr + "MMVARI").toString();
    b = FileExists(m);
    actionMMVARI->setEnabled(b);

    m = settings.value(eStr + "2Tone").toString();
    b = FileExists(m);
    action2Tone->setEnabled(b);

    m = settings.value(eStr + "MMTTY").toString();
    b = FileExists(m);
    actionMMTTY->setEnabled(b);

    m = settings.value(eStr + "Gritty").toString();
    b = FileExists(m);
    actionGritty->setEnabled(b);
#endif
    m = settings.value(eStr + "FLDigi").toString();
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
        setWindowTitle(tr("Minos Data Modes App"));
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
void DMMainWindow::closeAllEngines()
{
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
}
#ifdef Q_OS_WIN

void DMMainWindow::onActionMMVARI_triggered(bool /*checked*/)
{
    closeAllEngines();

    trace("Select MMVARI Engine");

    QSettings settings;
    QString eStr = QString("dataModes/engines/");
    QString exePath = QCoreApplication::applicationDirPath() + QString("/MMVARI.ocx");
    QString m = settings.value(eStr + "MMVARI", exePath).toString();

    actionMMVARI->setChecked(true);

    QString idev = settings.value(eStr + "MMVARI/input").toString();
    int inId = deviceIds[idev];

    QString odev = settings.value(eStr + "MMVARI/output").toString();
    int outId = deviceIds[odev];


    mmvariFrame = new MMVARIFrame(this, dynamic_cast<QVBoxLayout *>(ui->centralwidget->layout()), ui->sendEdit, m, inId, outId);
}

void DMMainWindow::onActionMMTTY_triggered(bool /*checked*/)
{
    closeAllEngines();
    trace("Select MMTTY Engine");

    QSettings settings;
    QString eStr = QString("dataModes/engines/");
    QString m = settings.value(eStr + "MMTTY").toString();

    mmttyFrame = new MMTTYFrame(false, ui->sendEdit, m);
    actionMMTTY->setChecked(true);
}

void DMMainWindow::onAction2Tone_triggered(bool /*checked*/)
{
    closeAllEngines();
    trace("Select 2Tone Engine");

    QSettings settings;
    QString eStr = QString("dataModes/engines/");
    QString m = settings.value(eStr + "2Tone").toString();

    mmttyFrame = new MMTTYFrame(false, ui->sendEdit, m);
    action2Tone->setChecked(true);
}
#endif
void DMMainWindow::onActionFLDigi_triggered(bool /*checked*/)
{
    closeAllEngines();
    trace("Select FLDigi Engine");
    QSettings settings;
    QString eStr = QString("dataModes/engines/");
    QString m = settings.value(eStr + "FLDigi").toString();

    fldigiFrame = new FLDigiFrame(this, ui->sendEdit, m);
}

void DMMainWindow::onActionTest_triggered(bool /*checked*/)
{
    closeAllEngines();
    QSettings settings;
    QString eStr = QString("dataModes/engines/");
    QString m = settings.value(eStr + "Test").toString();

    testFrame = new TestFrame(this, ui->sendEdit, m);
}
#ifdef Q_OS_WIN

void DMMainWindow::onActionGritty_triggered(bool /*checked*/)
{
    closeAllEngines();
    trace("Select Gritty Engine");

    QSettings settings;
    QString eStr = QString("dataModes/engines/");
    QString m = settings.value(eStr + "Gritty").toString();

    grittyFrame = new GrittyFrame(this, ui->sendEdit, m);
}
#endif
void DMMainWindow::onActionExit_triggered()
{
    close();
}
void DMMainWindow::doCloseEvent()
{
    closeAllEngines();


    LogTimer.stop();

    // and tidy up all loose ends

    QSettings settings;
    settings.setValue(geoStr, saveGeometry());


    trace("Minos Data Modes App Closing");

}

void DMMainWindow::on_sendButton_clicked()
{
    QString data = ui->sendEdit->text().trimmed();
#ifdef Q_OS_WIN

    if (mmvariFrame)
    {
        mmvariFrame->sendCharacters(data);
    }
    if (mmttyFrame)
    {
        mmttyFrame->sendCharacters(data);
    }
    if (grittyFrame)
    {
        grittyFrame->sendCharacters(data);
    }
#endif
    if (fldigiFrame)
    {
        fldigiFrame->sendCharacters(data);
    }
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

void DMMainWindow::wordSelected(QString word)
{
// word has been clicked on the datapainter; we need to send it
// on to the logger
    qDebug() << word;
}
