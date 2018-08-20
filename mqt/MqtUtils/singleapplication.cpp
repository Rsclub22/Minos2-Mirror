#include "singleapplication.h"
#include <QLocalSocket>
#include <QFileInfo>
#include <QMessageBox>
#include <QSettings>
#include <QThread>

#define TIME_OUT                (500)    // 500ms

SingleApplication::SingleApplication(QString serverName, int &argc, char **argv)
    :QApplication(argc, argv)
    , _isRunning(false)
    , _localServer(nullptr),
    _serverName(serverName)
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
        if (socket->waitForReadyRead(2*TIME_OUT))
        {
            QByteArray dataread = socket->read(1024);
            QString d = QString(dataread);
            emit argsReceived(d);
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

    QLocalSocket socket;
    socket.connectToServer(_serverName);
    if(socket.waitForConnected(TIME_OUT)) {
        fprintf(stderr, "%s already running.\n",
                _serverName.toLocal8Bit().constData());
        _isRunning = true;
        // Other treatments, such as: the start-up parameters are sent to the server
        return;
    }

    //Failed to connect to server, create a
    _newLocalServer();
}

////////////////////////////////////////////////////////////////////////////////
// Explain:
// Create LocalServer
////////////////////////////////////////////////////////////////////////////////
void SingleApplication::_newLocalServer() {
    _localServer = new QLocalServer(this);
    connect(_localServer, SIGNAL(newConnection()), this, SLOT(_newLocalConnection()));
    if(!_localServer->listen(_serverName)) {
        // The monitor failure, may beWhen a program crashes, residual process service led, removal
        if(_localServer->serverError() == QAbstractSocket::AddressInUseError) {
            QLocalServer::removeServer(_serverName); // <-- A key
            _localServer->listen(_serverName); // Listen again
        }
    }
}

void SingleApplication::clearRegistry()
{
#ifdef Q_OS_WIN
    QThread::msleep(1000);
    QSettings reg("HKEY_CURRENT_USER\\Software\\Minos2Qt", QSettings::NativeFormat);
    reg.clear();
#endif
}
void SingleApplication::sendArgs()
{
    QLocalSocket  *socket = new QLocalSocket;
    socket->connectToServer(_serverName);
    if(socket->waitForConnected(TIME_OUT))
    {
        QString args = arguments()[1];
        socket->write(args.toLatin1().data());
        socket->waitForBytesWritten(TIME_OUT);
        socket->deleteLater();
    }
}
