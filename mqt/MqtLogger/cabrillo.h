#ifndef CABRILLO_H
#define CABRILLO_H

#include <QFile>
#include <QSharedPointer>
class LoggerContestLog;


class Cabrillo
{
    LoggerContestLog * ct;
    QSharedPointer<QFile> regContestFile;
public:
    Cabrillo( LoggerContestLog * const ct );
    ~Cabrillo();

    bool exportTest( QSharedPointer<QFile> expfd );
};

#endif // CABRILLO_H
