#ifndef MONITOREDLOG_H
#define MONITOREDLOG_H
#include "base_pch.h"
#include "MonitoredContestLog.h"

class MonitoringFrame;
class MinosTestImport;

class MonitoredLog
{
   private:
      void getLogStanza( int sno );
      bool monitorEnabled = false;

      qint64 inStanzaRequest = 0;
      QSet <int> stanzasPulled;

      int lastScannedStanza = -1;

      PublishState state = psNotConnected;
      QString server;
      QString publishedName;
      QString displayName;
      int expectedStanzaCount = 0;
      MinosTestImport *mt = nullptr;
      MonitoredContestLog * contest = nullptr;
      MonitoringFrame *frame = nullptr;

   public:


      MonitoredLog();
      ~MonitoredLog();

      bool enabled() const
      {
         return monitorEnabled;
      }
      void setEnabled(bool s)
      {
          monitorEnabled = s;
      }

      void initialise( const QString &server, const QString &name );
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
      void startMonitor();
      void stopMonitor();
      void checkMonitor();
      void processLogStanza( int stanza, const QString &stanzaData );
      QString getDisplayName() const;
      void setDisplayName(const QString &value);
};

#endif // MONITOREDLOG_H
