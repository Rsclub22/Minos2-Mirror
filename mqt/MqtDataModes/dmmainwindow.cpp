#include "delayedaction.h"
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

#include "regsettings.h"
#include "AppStartup.h"
#include "MinosRPC.h"
#include "LogEvents.h"
#include "MTrace.h"
#include "enginewindow.h"
#include "engineconfigure.h"
#include "RPCCommandConstants.h"
#include "remotelogs.h"
#include "MonitoredLog.h"
#include "monitoredlogs.h"
#include "MShowMessageDlg.h"

#include "dmmainwindow.h"
#include "ui_dmmainwindow.h"

DMMainWindow *mainWindow = nullptr;

#define WATCHDOG_TIME 2000
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

DMMainWindow::DMMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::DMMainWindow)
{
    ui->setupUi(this);

    mainWindow = this;

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

    RemoteLogs::setSettingsFile(getDirectoryLocation(dlConfiguration) + "/DataModes.ini");

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    connect(stdinReader, &StdInReader::stdinLine, this, &DMMainWindow::onStdInRead);

    QString appName = getAppStartupName();
    MinosRPC *rpc = MinosRPC::getMinosRPC(appName);
    connect(rpc, &MinosRPC::routerCall, this, &DMMainWindow::on_routerCall);

    QStringList svr = {rpcConstants::rigControlCategory
                      ,rpcConstants::rigDetailsCategory
                      ,rpcConstants::rigStateCategory
                     };
    rpc->initialiseRouters(svr);

    QStringList svs = {};
    rpc->findProviders(rpcConstants::DMCat, svs);

    // let the logger know we exist
    rpc->publish(rpcConstants::DMCat, rpcConstants::DMSender, "me", psPublished);

    createCloseEvent();

    connect(&LogTimer, &QTimer::timeout, this, &DMMainWindow::LogTimerTimer);
    LogTimer.start(100);

    RegSettings settings;
    geoStr = QString("dataModes/geometry");
    QByteArray geometry = settings.getSettings().value(geoStr).toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    connect(RemoteLogs::getRemoteLogs(), &RemoteLogs::newMonitoredLog, this, &DMMainWindow::onNewLog);
    connect(RemoteLogs::getRemoteLogs(), &RemoteLogs::currentLogChanged, this, &DMMainWindow::onLogChanged);

    connect(ui->logsTreeView, &MonitoredLogs::logStarted, this, &DMMainWindow::onLogStarted);
    connect(ui->logsTreeView, &MonitoredLogs::logClosed, this, &DMMainWindow::onLogClosed);

    ui->startButton->setText(tr("Start All"));

    delayedAction(this, [=](){
        on_startButton_clicked();
    });
}

DMMainWindow::~DMMainWindow()
{
    delete ui;
}

void DMMainWindow::closeAllEngines()
{
    for (const auto &e:qAsConst(engines))
    {
        e->close();
        e->deleteLater();
    }
    engines.clear();
    ui->startButton->setText(tr("Start All"));

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
    RemoteLogs::getRemoteLogs()->testAutoStart();
}
void DMMainWindow::closeEvent(QCloseEvent *event)
{
    doCloseEvent();
    QWidget::closeEvent(event);
}
void DMMainWindow::moveEvent(QMoveEvent * event)
{
    RegSettings settings;
    settings.getSettings().setValue(geoStr, saveGeometry());
    QWidget::moveEvent(event);
}
void DMMainWindow::resizeEvent(QResizeEvent * event)
{
    RegSettings settings;
    settings.getSettings().setValue(geoStr, saveGeometry());
    QWidget::resizeEvent(event);
}
void DMMainWindow::changeEvent( QEvent* e )
{
    if( e->type() == QEvent::WindowStateChange )
    {
        RegSettings settings;
        settings.getSettings().setValue(geoStr, saveGeometry());
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
void DMMainWindow::doCloseEvent()
{
    LogTimer.stop();

    // and tidy up all loose ends

    RegSettings settings;
    settings.getSettings().setValue(geoStr, saveGeometry());

    closeAllEngines();

    trace("Minos Data Modes App Closing");
}

void DMMainWindow::onNewLog(QSharedPointer<MonitoredLog> ml)
{
    connect(ml.data(), &MonitoredLog::newStanzas, this, &DMMainWindow::onNewStanzas, Qt::QueuedConnection);
}
void DMMainWindow::onNewStanzas()
{
    // we need to re-analyse the display, especially when we have a new QSO

    for (const auto &e:qAsConst(engines))
    {
        e->rescan();
    }
}
void DMMainWindow::onLogChanged(QSharedPointer<MonitoredLog> /*ml*/)
{
    // we need to re-analyse the display, as current log has changed

    for (const auto &e:qAsConst(engines))
    {
        e->rescan();
    }
}
void DMMainWindow::onLogStarted(QSharedPointer<MonitoredLog> /*ml*/)
{
    for (const auto &e:qAsConst(engines))
    {
        e->rescan();
    }
}
void DMMainWindow::onLogClosed(QSharedPointer<MonitoredLog> /*ml*/)
{
    for (const auto &e:qAsConst(engines))
    {
        e->rescan();
    }
}

void DMMainWindow::on_configureButton_clicked()
{
    EngineConfigure ec(this);
    if (ec.exec() == QDialog::Accepted)
    {
        // If a lot has changed we may need to bounce everything
        // But for now, just broadcast speeds in case they have changed

        emit setSpeeds(EngineConfigure::getSpeed("BPSK"), EngineConfigure::getSpeed("RTTY"));
    }
}

void DMMainWindow::on_startButton_clicked()
{
    if (engines.empty())
    {
        // start all configured engines
        for(const auto &e:EngineWindow::enginesList)
        {
            if (EngineConfigure::getEngineEnabled(e))
            {
                EngineWindow *ew = new EngineWindow(/*this*/);
                connect(ew, &EngineWindow::sendCharactersUp, this, &DMMainWindow::sendPressed);

                ew->selectEngine(e);
                engines.push_back(ew);

                ew->show();
            }
        }
        emit setSpeeds(EngineConfigure::getSpeed("BPSK"), EngineConfigure::getSpeed("RTTY"));
        ui->startButton->setText(tr("Stop All"));
    }
    else
    {
        closeAllEngines();
    }
}

void DMMainWindow::sendPressed(QString d, int c)
{
    QSettings settings(getDirectoryLocation(dlConfiguration) + "/DataModes.ini", QSettings::IniFormat);

    QString m = settings.value("Sender").toString();

    // and send to this engine

    // search the engines for the one selected as sender
    bool sent = false;
    for (const auto &e:qAsConst(engines))
    {
        if (e->engineName == m)
        {
            trace(QString("Send <%1> to %2 mark %3").arg(d, m).arg(c));
            e->doSendCharacters(d, c);
            sent = true;
            break;
        }
    }
    if (!d.isEmpty() && !sent)
    {
        mShowMessage(tr("No DataMode sender configured"), this);
    }
}

void DMMainWindow::on_routerCall(bool err, QSharedPointer<MinosRPCObj>mro, const QString /*from*/ )
{
    if ( !err )
    {
        QString call = mro->getMethodName();
        trace("DMMainWindow routercall " + call);
        if (call == rpcConstants::DMTransmit)
        {

            RPCArgs *args = mro->getCallArgs();

            if (args)
            {
                QSharedPointer<RPCParam> psMess;
                QSharedPointer<RPCParam> piCarr;

                if (
                        args->getStructArgMember(0, rpcConstants::DMTransmit, psMess)
                        && args->getStructArgMember(0, rpcConstants::DMMarkFreq, piCarr)
                        )
                {
                    QString pmess;
                    int carr;
                    if (psMess->getString(pmess) && piCarr->getInt(carr))
                    {
                        sendPressed(pmess, carr);
                    }
                }
            }
        }
        else if (call == rpcConstants::DMStopTransmit)
        {
            sendPressed("", 0);
        }
    }
}
