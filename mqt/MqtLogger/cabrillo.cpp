#include "cabrillo.h"

Cabrillo::Cabrillo( LoggerContestLog * const ct )
      : ct( ct )
{}
Cabrillo::~Cabrillo()
{}


bool Cabrillo::exportTest(QSharedPointer<QFile> expfd)
{
    return false;
}
