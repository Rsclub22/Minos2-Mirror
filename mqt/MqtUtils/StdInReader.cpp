#include <QTextStream>
#include <QIODevice>
#include <QProcessEnvironment>
#include <QLocalServer>
#include <QLocalSocket>

#include "AppStartup.h"
#include "StdInReader.h"
#include "MTrace.h"

StdInReader::StdInReader(QMainWindow *m):qmw(m)
{
    connect(this, &StdInReader::stdinLine, this, &StdInReader::executeStdIn);

    appName = getAppStartupName();

    localServer = new QLocalServer(this);
    connect(localServer, &QLocalServer::newConnection, this, &StdInReader::newLocalConnection);
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
StdInReader::~StdInReader()
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
void StdInReader::newLocalConnection()
{
    trace(QString("StdInReader::newLocalConnection()"));
    if (!localSocket)
    {
        trace(QString("StdInReader::newLocalConnection() - no local socket"));
        localSocket = localServer->nextPendingConnection();
        if(localSocket)
        {
            trace(QString("StdInReader::newLocalConnection() - connecting readyRead"));
            connect(localSocket, &QLocalSocket::readyRead, this, &StdInReader::onReadyRead);
        }
    }
}
void StdInReader::onReadyRead()
{
    trace(QString("StdInReader::onReadyRead()"));

    QTextStream stdinStream(localSocket);

    QString line;

    line = stdinStream.readLine();

    while (!line.isNull())
    {
        trace(QString("StdInReader::onReadyRead() %1").arg(line));
        emit stdinLine(line);
        line = stdinStream.readLine();
    }
}
void StdInReader::setShowApp(bool state)
{
    if (qmw)
    {
        trace(QString("setting qmw visible to %1").arg(state));
        qmw->setVisible(state);
    }
}
void StdInReader::executeStdIn(QString cmd)
{
    trace("Command read from stdin: " + cmd);
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
