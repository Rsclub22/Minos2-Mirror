#include <QLocalSocket>
#include <QFileInfo>
#include <QMessageBox>
#include <QSettings>
#include <QThread>

#ifdef Q_OS_WIN
#include <windows.h>
//#include <tlhelp32.h>
#endif

#ifdef Q_OS_UNIX
#include <unistd.h>
#endif

#include "singleapplication.h"
#include "SecondInstall.h"

#define TIME_OUT                (500)    // 500ms
#define TEST_TIME_OUT           (10)    // 10ms

SingleApplication::SingleApplication(QString appName, int &argc, char **argv)
    :QApplication(argc, argv)
    , _isRunning(false)
    , _localServer(nullptr),
    _appName(appName)
{


    _initLocalConnection();
}


////////////////////////////////////////////////////////////////////////////////
// Explain:
// Check if there is already an instance running, true - a running example, false - no running instance
////////////////////////////////////////////////////////////////////////////////
bool SingleApplication::isRunning() {
    return _isRunning;
}

////////////////////////////////////////////////////////////////////////////////
// Explain:
// Single instance running program through socket communication, listening to the new connection is triggered when the function
////////////////////////////////////////////////////////////////////////////////
void SingleApplication::_newLocalConnection() {
    QLocalSocket *socket = _localServer->nextPendingConnection();
    if(socket) {
        sendPid(socket);
        if (socket->waitForReadyRead(2*TIME_OUT))
        {
            QByteArray dataread = socket->read(1024);
            pid = QString(dataread);
            emit argsReceived(pid);
        }
        delete socket;
    }
}

////////////////////////////////////////////////////////////////////////////////
// Explain:
// Realization of single instance running through socket communication program,
// Initialize the local connection, if the connection is not on the server, create, or exit
////////////////////////////////////////////////////////////////////////////////
void SingleApplication::_initLocalConnection() {
    _isRunning = false;

    bool running = testRunning(_appName, TIME_OUT, pid);
    if (running)
    {
        fprintf(stderr, "%s already running, PID %s.\n",
                _appName.toLocal8Bit().constData(), pid.toLocal8Bit().constData());
        _isRunning = true;
        // Other treatments, such as: the start-up parameters are sent to the server
        return;
    }

    //Failed to connect to server, create a
    _newLocalServer();
}
bool SingleApplication::testRunning(QString name, int timeout, QString &pid)
{
    QLocalSocket socket;
    socket.connectToServer(name);
    if(socket.waitForConnected((timeout > 0)?timeout:TEST_TIME_OUT))
    {
        if (socket.waitForReadyRead(2*TIME_OUT))
        {
            QByteArray dataread = socket.read(1024);
            pid = QString(dataread);
        }
         return true;
    }
    return false;
}
////////////////////////////////////////////////////////////////////////////////
// Explain:
// Create LocalServer
////////////////////////////////////////////////////////////////////////////////
void SingleApplication::_newLocalServer()
{
    _localServer = new QLocalServer(this);
    connect(_localServer, &QLocalServer::newConnection, this, &SingleApplication::_newLocalConnection);
    if(!_localServer->listen(_appName)) {
        // The monitor failure, may beWhen a program crashes, residual process service led, removal
        if(_localServer->serverError() == QAbstractSocket::AddressInUseError) {
            QLocalServer::removeServer(_appName); // <-- A key
            _localServer->listen(_appName); // Listen again
        }
    }
}

void SingleApplication::clearRegistry()
{
#ifdef Q_OS_WIN
    QThread::msleep(1000);
    QSettings reg("HKEY_CURRENT_USER\\Software\\" + SecondInstall::getOrgName(), QSettings::NativeFormat);
    reg.clear();
#endif
}
void SingleApplication::sendArgs()
{
    QLocalSocket  *socket = new QLocalSocket;
    socket->connectToServer(_appName);
    if(socket->waitForConnected(TIME_OUT))
    {
        QStringList sl = arguments();
        QString args = sl[1];
        socket->write(args.toLatin1().data());
        socket->waitForBytesWritten(TIME_OUT);
    }
    socket->deleteLater();
}

void SingleApplication::sendPid(QLocalSocket *socket)
{
    QString spid;
#ifdef Q_OS_WIN
    unsigned long pid = GetCurrentProcessId();
    spid = QString::number(pid);
#else
    pid_t pid = getpid();
    spid = QString::number(pid);
#endif
    socket->write(spid.toLatin1().data());
}

