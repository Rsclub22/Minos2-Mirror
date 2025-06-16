/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////

#ifndef SendRPCDMH
#define SendRPCDMH 
#include "RPCCommandConstants.h"
#include "RigCache.h"
#include "RotatorCache.h"
#include "clustercommon.h"
#include "rotatorcommon.h"

//---------------------------------------------------------------------------
class MinosRPCObj;
class TSingleLogFrame;
//---------------------------------------------------------------------------
enum RotateDirection : int;
class TSendDM : public QObject
{
    Q_OBJECT
   private:  	// User declarations
      RigCache rigCache;
      RotatorCache rotatorCache;

      bool radioLoaded = false;
      bool rotatorLoaded = false;
      bool keyerLoaded = false;
      bool clusterServerLoaded = false;
      bool clusterConnected = false;

      bool pcCwKeyerLoaded = false;
      bool pcCwKeyerConnected = false;

      PubSubName keyerApp;
      PubSubName clusterApp;
      PubSubName pcCwKeyerApp;

      QString loggerUuid;

      void traceMsg(QString msg);
      void getRouterAppCatMap();
      void sendKeyerUser();



  public:  		// User declarations
      TSendDM( QWidget* Owner );
      ~TSendDM();

      void subscribeApps();

      void invalidateCache();
      void invalidateRigCache(const PubSubName &name);
      void invalidateRotatorCache(const PubSubName &name);

      QString getLoggerUuid()
      {
          return loggerUuid;
      }
      PubSubName getSelectedRig(QString loggerUuid);
      PubSubName getSelectedRot(QString loggerUuid);

      //void sendBandMap( TSingleLogFrame *tslf,const QString &freq, const QString &call, const QString &utc, const QString &loc, const QString &qth );

      void sendSpotToClusterServer(  const Frequency &freq, const QString &call, const QString &loc );
      void sendRequestSpotsResentFromClusterServer(resendFrameId frameId, const QString &cmd, const QString bandMask, const QString &uuid);
      void sendReconnectFlagToClusterServer(const bool state);

      void sendKeyerPlay( TSingleLogFrame *tslf,int fno );
      void sendKeyerRecord(TSingleLogFrame *tslf, int fno );

      void publishKeyerConfig(const QString &config);
      void publishKeyerMS(bool send);

      void sendKeyerTone(TSingleLogFrame *tslf);
      void sendKeyerTwoTone(TSingleLogFrame *tslf);
      void sendKeyerStop(TSingleLogFrame *tslf);
      void sendRotatorPreset(QString);
      void changeRotatorSelectionTo(const PubSubName &name, const QString &uuid);
      void sendRotatorSelection(const PubSubName &name, const QString &uuid);
      void sendRotator(TSingleLogFrame *tslf,rpcConstants::RotateDirection direction, int angle );


      void changeRigSelectionTo(const PubSubName &name, const QString&band, const Frequency &freq, const QString &mode, const QString &uuid);
      void sendRigSelection(const PubSubName &name, const QString&band, const Frequency &freq, const QString &mode, const QString &uuid);
      void sendRigControlFreq(TSingleLogFrame *tslf,const Frequency &freq);
      void sendRigControlBand(TSingleLogFrame *tslf, const QString &band);
      void sendRigTxVoiceMessage(TSingleLogFrame *tslf, const QString &msgNum);
      void sendRigStopTxVoiceMessage(TSingleLogFrame *tslf, const QString &msg);
      void sendRigTxCwMessage(TSingleLogFrame *tslf, const QString &msg);

      void sendPcKeyerTxCwMessage(const QString &msg);
      void sendPcKeyerTxCwStop(const QString &msg);


      void sendRigControlMode(TSingleLogFrame *tslf, const QString &mode);
      void sendRigControlVolumeLevel(TSingleLogFrame *tslf, int level);
      void sendRigControlPassBandState(TSingleLogFrame *tslf,const int state);
      void sendRigControlRitFreq(TSingleLogFrame *tslf, ShortFreq freq);
      void sendRigControlRitStatus(TSingleLogFrame *tslf,const bool &status);
      void sendRigControlPttOnOff(TSingleLogFrame *tslf, const bool &onOff);

      QStringList rotators();
      QStringList rigs();

      const RigState &getRigState(const QString &);
      const RigDetails &getRigDetails(const QString &);

      void notifyRigChanges();
      void notifyRigDetailChanges();

      
      void notifyRotChanges();

      RigCache* getRigCache()
      {
          return &rigCache;
      }

      bool isRadioLoaded()
      {
          return radioLoaded;
      }
      bool isRotatorLoaded()
      {
          return rotatorLoaded;
      }
      bool isKeyerLoaded()
      {
          return keyerLoaded;
      }
      bool isClusterServerLoaded()
      {
          return clusterServerLoaded;
      }
      bool isClusterConnected()
      {
          return clusterConnected;
      }
      bool isPcCWkeyerLoaded()
      {
          return pcCwKeyerLoaded;
      }
      bool isPcCwKeyerConnected()
      {
          return pcCwKeyerConnected;
      }


      void sendSkyScanControlPanelButtonState(TSingleLogFrame *tslf, const SkyScanButtonState state);
      void sendRotatorSkyScanPresetNumberToRotator(int buttonNumber);

  private slots:
      void on_routerCall( bool err, QSharedPointer<MinosRPCObj>mro, const QString from );
      void on_notify(AnalysePubSubNotify an, const QString from );

      void routerClosed();
  signals:
      //void setBandMapLoaded();

      void RotatorLoaded();
      void RotatorList();

      void setRadioLoaded();
      void setRadioList();
      void setClusterState(QString);
      void setClusterTXSpotEnableState(QString);

      void keyerConfig(QString, QString);
      void keyerReport(QString);

      void pcCwKeyerComport(QString);
      void pcCwKeyerConnectionState(QString);
      void pcCwKeyerErrorMsg(QString);
      void pcCwKeyerPttEnabled(QString);
      void pcCwKeyerTxOn(QString);

};
#endif
