#include <QTextStream>

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
    QTextStream stdinStream(stdin, QIODevice::ReadOnly);

    QString line;
    for (;;)
    {
        line = stdinStream.readLine();
        if (line.isNull())
            break;          // catch stdin being closed by the other end
        emit stdinLine(line);
    }
}
void StdInReader::setShowApp(bool state)
{
    if (qmw)
    {
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
