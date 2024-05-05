#include <QTextStream>
#include <QIODevice>

#include "AppStartup.h"
#include "StdInReader.h"
#include "MTrace.h"

StdInReader::StdInReader(QMainWindow *m):qmw(m)
{
    connect(this, &StdInReader::stdinLine, this, &StdInReader::executeStdIn);
    start();
}
StdInReader::~StdInReader()
{
    terminate();
    wait();
}

void StdInReader::run()
{
    // called by QThread from start()

    QTextStream stdinStream(stdin, QIODevice::ReadOnly);

    QString line;
    while (true)
    {
        line = stdinStream.readLine();

        if (line.isNull())
            break;

        emit stdinLine(line);

        if (line.indexOf("Shutdown", 0, Qt::CaseInsensitive) >= 0)
        {
            break;
        }
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
