#ifndef MONITOREDLOG_H
#define MONITOREDLOG_H
#include <QSet>
#include "MonitoredContestLog.h"
#include "PublishState.h"
#include "monitoredstation.h"

class MonitoringFrame;
class MinosTestImport;
class MinosRPCObj;
struct MonitoredLogCmp
{
   QString cmpstr;
   MonitoredLogCmp( const QString &s ) : cmpstr( s )
   {}

   bool operator() ( QSharedPointer<MonitoredLog> s1 ) const;
};
class MonitoredLog : public QObject
{
    Q_OBJECT
   private:
      void getLogStanza( int sno );
      bool monitorEnabled = false;
      bool manualClose = false;

      qint64 inStanzaRequest = 0;
      QSet <int> stanzasPulled;

      int lastScannedStanza = -1;

      PublishState state = psNotConnected;
      QString router;
      QString publishedName;
      QString displayName;
      int expectedStanzaCount = 0;
      QDateTime startTime;
      QDateTime endTime;
      MinosTestImport *mt = nullptr;
      MonitoredContestLog * contest = nullptr;
      MonitoringFrame *frame = nullptr;

      MonitoredStation *station = nullptr;

      QSet<DupContact >callsigns;

   public:
      MonitoredLog(MonitoredStation *);
      ~MonitoredLog();

      bool enabled() const
      {
         return monitorEnabled;
      }
      void setEnabled(bool s)
      {
          monitorEnabled = s;
      }

      void initialise( const QString &router, const QString &name );
      BaseContestLog * getContest() const
      {
         return contest;
      }
      QString getPublishedName() const
      {
         return publishedName;
      }
      void setExpectedStanzaCount( int ec )
      {
         expectedStanzaCount = ec;
      }
      void setFrame( MonitoringFrame *f )
      {
         frame = f;
      }
      MonitoringFrame *getFrame()
      {
          return frame;
      }

      void setState(PublishState s)
      {
         state = s;
      }
      PublishState getState() const
      {
         return state;
      }
      void setStartEnd(QString s, QString e);
      void startMonitor();
      void checkMonitor();
      void processLogStanza( int stanza, const QString &stanzaData );
      QString getDisplayName() const;
      void setDisplayName(const QString &value);
      bool testAutoStart();

      bool getManualClose() const;
      void setManualClose(bool newManualClose);
      QSet<DupContact > &getCallsigns();

private slots:
      void on_routerCall(bool err, QSharedPointer<MinosRPCObj> mro, const QString from);

signals:
      void newStanzas(MonitoredLog *);
      void newLastContact(MonitoredLog *);
      void contactChanged(MonitoredLog *);
};

#endif // MONITOREDLOG_H
