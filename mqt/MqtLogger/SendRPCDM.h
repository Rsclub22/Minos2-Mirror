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
#include "base_pch.h"

#include "ConfigFile.h"
#include "RigCache.h"
#include "RotatorCache.h"

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
      QMap<QString,QVector< QSharedPointer<Connectable> > > catMap;
      QVector<QSharedPointer<Connectable> > connectables;
      QVector<QString> servers;

      PubSubName keyerApp;

      QString loggerUuid;


public:  		// User declarations
      TSendDM( QWidget* Owner );
      ~TSendDM();

      void subscribeApps();

      bool radioLoaded = false;
      bool rotatorLoaded = false;

      void invalidateCache();
      void invalidateRigCache(const PubSubName &name);
      void invalidateRotatorCache(const PubSubName &name);

      QString getLoggerUuid()
      {
          return loggerUuid;
      }
      PubSubName getSelectedRig(QString loggerUuid);
      PubSubName getSelectedRot(QString loggerUuid);

      void sendKeyerPlay( TSingleLogFrame *tslf,int fno );
      void sendKeyerRecord(TSingleLogFrame *tslf, int fno );
      void sendBandMap( TSingleLogFrame *tslf,const QString &freq, const QString &call, const QString &utc, const QString &loc, const QString &qth );
      void sendKeyerTone(TSingleLogFrame *tslf);
      void sendKeyerTwoTone(TSingleLogFrame *tslf);
      void sendKeyerStop(TSingleLogFrame *tslf);
      void sendRotatorPreset(QString);
      void changeRotatorSelectionTo(const PubSubName &name, const QString &uuid);
      void sendRotatorSelection(const PubSubName &name, const QString &uuid);
      void sendRotator(TSingleLogFrame *tslf,rpcConstants::RotateDirection direction, int angle );

      void changeRigSelectionTo(const PubSubName &name, const QString &mode, const QString &uuid);
      void sendRigSelection(const PubSubName &name, const QString &mode, const QString &uuid);
      void sendRigControlFreq(TSingleLogFrame *tslf,const QString &freq);
      void sendRigControlMode(TSingleLogFrame *tslf, const QString &mode);
      void sendRigControlVolumeLevel(TSingleLogFrame *tslf, int level);
      void sendRigControlPassBandState(TSingleLogFrame *tslf,const int state);
      void sendRigControlRitFreq(TSingleLogFrame *tslf, int freq);
      void sendRigControlRitStatus(TSingleLogFrame *tslf,const bool &status);
      void sendRigControlTpm(TSingleLogFrame *tslf,int tpm, QString &freq);

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


private slots:
      void on_serverCall( bool err, QSharedPointer<MinosRPCObj>mro, const QString &from );
      void on_notify( bool err, QSharedPointer<MinosRPCObj>mro, const QString &from );

signals:
      void setBandMapLoaded();

      void RotatorLoaded();
      void RotatorList();

      void setRadioLoaded();
      void setRadioList();

      void setKeyerLoaded();

};
#endif
