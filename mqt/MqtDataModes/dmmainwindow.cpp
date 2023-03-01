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
#include "enginewindow.h"
#include "engineconfigure.h"
#include "RPCCommandConstants.h"
#include "remotelogs.h"
#include "MonitoredLog.h"
#include "monitoredlogs.h"

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

    logsTreeView = new MonitoredLogs(this);
    logsTreeView->setVisible(false);

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

    QString appName = getAppStartupName();
    MinosRPC *rpc = MinosRPC::getMinosRPC(appName);
    connect(rpc, &MinosRPC::routerCall, this, &DMMainWindow::on_routerCall);

    QStringList svr = {rpcConstants::rigControlCategory
                      ,rpcConstants::rigDetailsCategory
                      ,rpcConstants::rigStateCategory
                     };
    rpc->initialiseRouters(svr);

    // let the logger know we exist
    rpc->publish(rpcConstants::DMCat, rpcConstants::DMSender, "me", psPublished);

    createCloseEvent();

    connect(&LogTimer, &QTimer::timeout, this, &DMMainWindow::LogTimerTimer);
    LogTimer.start(100);

    QSettings settings;
    geoStr = QString("dataModes/geometry");
    QByteArray geometry = settings.value(geoStr).toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    connect(RemoteLogs::getRemoteLogs(), &RemoteLogs::newMonitoredLog, this, &DMMainWindow::onNewLog);
}

DMMainWindow::~DMMainWindow()
{
    delete ui;
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
    testAutoStart();
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

    QSettings settings;
    settings.setValue(geoStr, saveGeometry());

    on_stopButton_clicked();

    trace("Minos Data Modes App Closing");
}

void DMMainWindow::testAutoStart()
{
    for ( auto const &s: qAsConst(RemoteLogs::getRemoteLogs()->stationList) )
    {
       for ( auto &ml: s->slotList )
       {
            if (!ml->enabled())
            {
                if (ml->testAutoStart())
                {
                    ml->startMonitor();
                }
            }
       }
    }
}
void DMMainWindow::onNewLog(MonitoredLog *ml)
{
    connect(ml, &MonitoredLog::newStanzas, this, &DMMainWindow::onNewStanzas, Qt::QueuedConnection);
}

void DMMainWindow::onNewStanzas()
{

}

void DMMainWindow::on_configureButton_clicked()
{
    EngineConfigure ec(this);
    ec.exec();
}

void DMMainWindow::on_startButton_clicked()
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
}

void DMMainWindow::on_stopButton_clicked()
{
    // stop all configured engines

    for (const auto &e:qAsConst(engines))
    {
        e->close();
        e->deleteLater();
    }
    engines.clear();
}

void DMMainWindow::sendPressed(QString d, int c)
{
    QSettings settings("./Configuration/DataModes.ini", QSettings::IniFormat);

    QString m = settings.value("Sender").toString();

    // and send to this engine

    // search the engines for the one selected as sender
    for (const auto &e:qAsConst(engines))
    {
        if (e->engineName == m)
        {
            e->doSendCharacters(d, c);
            break;
        }
    }

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
