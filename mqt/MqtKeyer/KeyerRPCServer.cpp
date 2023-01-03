/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////

#include "AppStartup.h"
#include "MTrace.h"
#include "MinosRPC.h"
#include "KeyerJson.h"
#include "RPCCommandConstants.h"
#include "keyctrl.h"
#include "RPCPubSub.h"
#include "KeyerRPCServer.h"

KeyerServer *KS = nullptr;

static bool sendMeters = false;
static bool sendSliders = false;
static int metersSeq = 0;
static int slidersSeq = 0;
//---------------------------------------------------------------------------
KeyerServer::KeyerServer()
{
    MinosRPC *rpc = MinosRPC::getMinosRPC(getAppStartupName());

    connect(rpc, &MinosRPC::routerCall, this, &KeyerServer::on_routerCall);
    connect(rpc, &MinosRPC::notify, this, &KeyerServer::on_notify);

    RPCPubSub::subscribe( rpcConstants::lineControlCategory );
}
//---------------------------------------------------------------------------
KeyerServer::~KeyerServer()
{
}
//---------------------------------------------------------------------------
/*static*/void KeyerServer::checkConnection()
{
   if ( !KS )
      KS = new KeyerServer;
}
//---------------------------------------------------------------------------
void KeyerServer::doPublishState( const QString &state )
{
  static QString old;

  if ( state != old )
  {
     old = state;
     RPCPubSub::publish( rpcConstants::KeyerCategory, rpcConstants::keyerReport, state, psPublished );
  }
}
/*static*/void KeyerServer::publishState(  const QString &state )
{
   checkConnection();

   KS->doPublishState( state );
}
//---------------------------------------------------------------------------
void KeyerServer::doPublishCommand( const QString &cmd )
{
      static QString old;

      if ( cmd != old )
      {
         old = cmd;
         RPCPubSub::publish( rpcConstants::KeyerCategory, "Command", cmd, psPublished );
      }
}
/*static*/void KeyerServer::publishCommand(  const QString &cmd )
{
    checkConnection();

    KS->doPublishCommand( cmd );
}
//---------------------------------------------------------------------------
void KeyerServer::doPublishConfig(const QString &config)
{
    static QString old;
    if (config != old)
    {
        old = config;
        RPCPubSub::publish(rpcConstants::KeyerCategory, rpcConstants::keyerConfig, config, psPublished);
    }
}
void KeyerServer::publishConfig(const QString &config)
{
    checkConnection();

    KS->doPublishConfig(config);
}
//---------------------------------------------------------------------------
void KeyerServer::doPublishSliders(int rec, int replay, int passthrough, CompressorParams &cp, int seq)
{
    static QString old;
    QString sliders;
    sliders = QString("%1;%2;%3;%4;%5;%6;%7;%8;%9;%10;%11;%12")
            .arg(rec).arg(replay).arg(passthrough)
            .arg(cp.window)
            .arg(cp.threshold)
            .arg(cp.ratio)
            .arg(cp.attack)
            .arg(cp.release)
            .arg(cp.makeUpGain)
            .arg(cp.doFilter)
            .arg(cp.doCompression)
            .arg(seq);
    if (sliders != old)
    {
        old = sliders;
        RPCPubSub::publish(rpcConstants::KeyerCategory, rpcConstants::keyerSliders, sliders, psPublished);
    }
}
void KeyerServer::publishSliders(int rec, int replay, int passthrough, CompressorParams &comp)
{
    checkConnection();

    if (sendSliders)
        KS->doPublishSliders(rec, replay, passthrough, comp, slidersSeq);
}
//---------------------------------------------------------------------------
void KeyerServer::doPublishVUMeter(unsigned int rmsLevel, unsigned int peakLevel, unsigned int numSamples, int seq)
{
    static QString old;
    QString vu;
    vu = QString("%1;%2;%3;%4").arg(rmsLevel).arg(peakLevel).arg(numSamples).arg(seq);
    if (vu != old)
    {
        old = vu;
        RPCPubSub::publish(rpcConstants::KeyerCategory, rpcConstants::keyerMeter, vu, psPublished);
    }
}
void KeyerServer::publishVUMeter(unsigned int rmsLevel, unsigned int peakLevel, unsigned int numSamples)
{
    checkConnection();

    if (sendMeters)
        KS->doPublishVUMeter(rmsLevel, peakLevel, numSamples, metersSeq);
}
//---------------------------------------------------------------------------
void KeyerServer::on_routerCall(bool err, QSharedPointer<MinosRPCObj>mro, const QString from )
{
    //trace( "Keyer callback from " + from + ( err ? ":Error" : ":Normal" ) );

    if ( !err )
    {
        QSharedPointer<RPCParam> psName;
        QSharedPointer<RPCParam>p1Value;
        RPCArgs *args = mro->getCallArgs();
        if ( args->getStructArgMember( 0, rpcConstants::paramName, psName ) && args->getStructArgMember( 0, rpcConstants::paramValue, p1Value ) )
        {
            QString commandName;
            int Value;
            bool nameOk = psName->getString( commandName );

            if (nameOk && commandName == rpcConstants::keyerUser)
            {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
                QStringList p = from.split(QChar('@'), Qt::KeepEmptyParts);
#else
                QStringList p = from.split(QChar('@'), QString::KeepEmptyParts);
#endif
                if (p.size() > 1)
                {
                    RPCPubSub::subscribeRemote(p[1], rpcConstants::KeyerConfigCategory );
                }

            }

            else if ( nameOk && p1Value->getInt( Value ) )
            {
                if ( Value >= 0 && Value < KEYERKEYS )
                {
                    if ( commandName == rpcConstants::keyerPlayFile )
                    {
                        playKeyerFile( Value, true );    // do actual transmit, and repeat as required
                    }
                    else
                        if ( commandName == rpcConstants::keyerRecordFile )
                        {
                            startRecordDVPFile( Value );
                        }
                }
                if ( commandName ==rpcConstants::keyerTone )
                {
                    sendTone1();
                }
                else
                    if ( commandName == rpcConstants::keyerTwoTone)
                    {
                        sendTone2();
                    }
                    else
                        if ( commandName == rpcConstants::keyerStop )
                        {
                            stopKeyer();
                        }
                        else
                            if ( commandName == rpcConstants::keyerConfig )
                            {
                                QString config;
                                if (p1Value->getString(config))
                                {
                                    doConfig(config);
                                }
                            }

                mro->clearCallArgs();
                QSharedPointer<RPCParam>st(new RPCParamStruct);
            }
        }
    }
}
//---------------------------------------------------------------------------
void KeyerServer::on_notify(AnalysePubSubNotify an, const QString /*from*/ )
{
   //trace( "Notify callback from " + from + ( !an.getOK() ? ":Error" : ":Normal" ) );


   // called whenever line changes
   if ( an.getOK() )
   {
      if ( an.getCategory() == rpcConstants::lineControlCategory )
      {
         lineStates[ an.getKey() ] = ( ( an.getValue() == rpcConstants::lineSet ) ? true : false );
         trace( rpcConstants::lineControlCategory + " " + an.getKey() + ":" + an.getValue() );
         return;
      }
      if ( an.getState() == psPublished)
      {
          trace(QString("Category %1 key %2").arg(an.getCategory(), an.getKey()));

          if (an.getCategory() == rpcConstants::KeyerConfigCategory)
          {
              if (an.getKey() == rpcConstants::keyerSendConfig)
              {
                QString value = an.getValue();
                emit keyerConfig(value);
              }
              if (an.getKey() == rpcConstants::keyerSendMS)
              {
                QString value = an.getValue();
                if (value.isEmpty())
                {
                    sendSliders = false;
                    sendMeters = false;
                }
                else
                {
                    sendSliders = true;
                    sendMeters = true;
                    metersSeq++;
                    slidersSeq++;
                }
              }
              else if (an.getKey() == rpcConstants::keyerSliders)
              {
                  QString value = an.getValue();
                  QStringList vals = value.split(";");
                  int rec = vals[0].toInt();
                  int rep = vals[1].toInt();
                  int pass = vals[2].toInt();

                  CompressorParams cp;
                  cp.window = vals[3].toDouble();
                  cp.threshold = vals[4].toDouble();
                  cp.ratio = vals[5].toDouble();
                  cp.attack = vals[6].toDouble();
                  cp.release = vals[7].toDouble();
                  cp.makeUpGain = vals[8].toDouble();

                  cp.doFilter = vals[9].toDouble();
                  cp.doCompression = vals[10].toDouble();
                  emit sliders(rec, rep, pass, cp);
              }
          }

      }
      else
      {
          if (an.getCategory() == rpcConstants::KeyerConfigCategory && an.getKey() == rpcConstants::keyerSendMS)
          {
              sendSliders = false;
              sendMeters = false;
          }
      }
   }
}
//---------------------------------------------------------------------------
bool KeyerServer::getState( const QString &line )
{
   QMap<QString, bool>::iterator l = lineStates.find( line );
   if ( l != lineStates.end() )
   {
      return l.value();
   }
   return false;
}
