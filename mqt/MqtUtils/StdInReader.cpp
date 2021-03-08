#include "mqtUtils_pch.h"
#include <QTextStream>

#include "StdInReader.h"

StdInReader::StdInReader()
{

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
static bool showApp = true;
bool getShowApp()
{
    return showApp;
}
void setShowApp(bool state)
{
    showApp = state;
}
