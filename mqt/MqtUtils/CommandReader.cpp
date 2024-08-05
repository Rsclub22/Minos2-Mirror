#include <QTextStream>
#include <QIODevice>
#include <QProcessEnvironment>
#include <QLocalServer>
#include <QLocalSocket>

#include "AppStartup.h"
#include "CommandReader.h"
#include "MTrace.h"

CommandReader::CommandReader(QMainWindow *m):qmw(m)
{
    connect(this, &CommandReader::commandLine, this, &CommandReader::executeCommand);

    appName = getAppStartupName();

    localServer = new QLocalServer(this);
    connect(localServer, &QLocalServer::newConnection, this, &CommandReader::newLocalConnection);
    trace(QString("About to listen on %1").arg(appName));
    if(!localServer->listen(appName))
    {

        // The monitor failure, may beWhen a program crashes, residual process service led, removal
        if(localServer->serverError() == QAbstractSocket::AddressInUseError) {
            QLocalServer::removeServer(appName); // <-- A key
            localServer->listen(appName); // Listen again
        }
    }

}
CommandReader::~CommandReader()
{
    if (localSocket)
    {
        localSocket->close();
    }
    if (localServer)
    {
        localServer->close();
    }
}
void CommandReader::newLocalConnection()
{
    trace(QString("CommandReader::newLocalConnection()"));
    if (!localSocket)
    {
        trace(QString("CommandReader::newLocalConnection() - no local socket"));
        localSocket = localServer->nextPendingConnection();
        if(localSocket)
        {
            trace(QString("CommandReader::newLocalConnection() - connecting readyRead"));
            connect(localSocket, &QLocalSocket::readyRead, this, &CommandReader::onReadyRead);
        }
    }
}
void CommandReader::onReadyRead()
{
    trace(QString("CommandReader::onReadyRead()"));

    QTextStream commandStream(localSocket);

    QString line;

    line = commandStream.readLine();

    while (!line.isNull())
    {
        trace(QString("CommandReader::onReadyRead() %1").arg(line));
        emit commandLine(line);
        line = commandStream.readLine();
    }
}
void CommandReader::setShowApp(bool state)
{
    if (qmw)
    {
        trace(QString("setting qmw visible to %1").arg(state));
        qmw->setVisible(state);
    }
}
void CommandReader::executeCommand(QString cmd)
{
    trace("Command read from commandReader: " + cmd);
    if (cmd.indexOf("ShowServers", 0, Qt::CaseInsensitive) >= 0)
        setShowApp(true);
    if (cmd.indexOf("HideServers", 0, Qt::CaseInsensitive) >= 0)
        setShowApp(false);
    if (cmd.indexOf("Font ", 0, Qt::CaseInsensitive) >= 0)
    {
        setAppFont(cmd);
    }
    if (cmd.indexOf("Shutdown", 0, Qt::CaseInsensitive) >= 0)
    {
        QApplication::closeAllWindows();
    }
}
