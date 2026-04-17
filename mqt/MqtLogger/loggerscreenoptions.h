#ifndef LOGGERSCREENOPTIONS_H
#define LOGGERSCREENOPTIONS_H

#include <QObject>

#include "ScreenConfigElement.h"

class LoggerScreenOptions
{
    static QVector <SCTypeOption> loggerScreenOptions;
    static QString defaultConfig;
    static QString protectedConfig;

public:
    LoggerScreenOptions();
};

#endif // LOGGERSCREENOPTIONS_H
