#ifndef MONITOREDSTATION_H
#define MONITOREDSTATION_H

#include <QVector>
#include <QSharedPointer>

class MonitoredLog;

class MonitoredStation
{
   public:
      QVector< QSharedPointer<MonitoredLog> > slotList;
      QSharedPointer<MonitoredLog> currentLog;

      MonitoredStation();
      ~MonitoredStation();
};

#endif // MONITOREDSTATION_H
