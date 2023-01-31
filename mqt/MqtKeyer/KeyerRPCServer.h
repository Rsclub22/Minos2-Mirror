/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------

#ifndef KeyerRPCServerH
#define KeyerRPCServerH 
//---------------------------------------------------------------------------
#include <QObject>
#include <QString>
#include <QMap>
#include "AnalysePubSubNotify.h"
#include "CompressorParams.h"

class MinosRPCObj;
class KeyerServer:public QObject
{
    Q_OBJECT
   private:
      QMap<QString, bool> lineStates;
      void doPublishState( const QString &state );
      void doPublishCommand( const QString &cmd );
      void doPublishConfig(const QString &config);
      void doPublishSliders(int rec, int replay, int passthrough, CompressorParams &cp, int seq);
      void doPublishVUMeter(unsigned int rmsLevel, unsigned int peakLevel, unsigned int numSamples, int seq);
   public:
      KeyerServer();
      ~KeyerServer();
      static void checkConnection();
      static void publishState( const QString &state );
      static void publishCommand( const QString &cmd );
      static void publishConfig(const QString &config);
      static void publishSliders(int rec, int replay, int passthrough, CompressorParams &comp);
      static void publishVUMeter(unsigned int rmsLevel, unsigned int peakLevel, unsigned int numSamples);
      static void publishIPDetail(QString port);

      bool getState( const QString &line );

signals:
      void sliders(int, int, int, CompressorParams);
      void keyerConfig(QString);

private slots:
      void on_routerCall(bool err, QSharedPointer<MinosRPCObj> mro, const QString from );
      void on_notify(AnalysePubSubNotify an, const QString from );
};
extern KeyerServer *KS;
#endif
