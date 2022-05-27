/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
#ifndef keyersH
#define keyersH
#include "base_pch.h"

#include "keyerBase.h"
#include "keyconf.h"
#include "keyctrl.h"
#include "keyerlog.h"
#include "sbdriver.h"

enum LineModes{
    elmNone = 0,
    elmTones,
    elmPlay01Pip,
    elmPlay01NoPip,
    elm01Record,
    elmPlay23Pip,
    elmPlay23NoPip,
    elm23Record,
    elmPlay45Pip,
    elmPlay45NoPip,
    elm45Record,
    elmPlay67Pip,
    elmPlay67NoPip,
    elm67Record,
    elmAppsRestartClose,    // 14
    elmOSRestartClose,      // 15
    elmMax                  // 16
};

extern unsigned long MORSEINTCOUNT;
extern qint64 currTick;

class SoundSystemDriver;

class commonPort;
class commonKeyer;

class timerTicker: public QObject
{
    Q_OBJECT
      QTimer b;
   public:
      timerTicker();
      ~timerTicker() override;
      virtual void tickEvent() = 0;       // this will often be an interrupt routine

private slots:
      void tick()
      {
          tickEvent();
      }
};
class lineMonitor: public timerTicker
{
    Q_OBJECT
   public:
      commonPort *cp = nullptr;
      PortConfig pconf;
      KeyerConfig kconf;
      QString pName;
      lineMonitor( const QString pname );
      lineMonitor( const KeyerConfig &keyer, const PortConfig &port );
      virtual ~lineMonitor();

      virtual void ptt( int state );

      virtual bool pttChanged( int state ) = 0;
      virtual bool L1Changed( int state ) = 0;
      virtual bool L2Changed( int state ) = 0;
      virtual bool linesModeChanged(int lmode) = 0;
      virtual bool transverterSwitchChanged(int /*s*/)
      {
          return true;
      }

      virtual bool initialise( const KeyerConfig &keyer, const PortConfig &port ) = 0;

      virtual void checkControls( );
};

// all keyers want timer ticks to sequence the state machine
class commonKeyer: public lineMonitor
{
    Q_OBJECT
   public:
      commonKeyer( const KeyerConfig &keyer, const PortConfig &port );
      ~commonKeyer() override;

      virtual bool pttChanged( int state ) override;
      virtual bool L1Changed( int state ) override;
      virtual bool L2Changed( int state ) override;
      virtual bool linesModeChanged(int lmode) override;
      virtual bool transverterSwitchChanged(int s) override;

      virtual void tickEvent() override;       // this will often be an interrupt routine
      virtual bool getInfo( KeyerInfo * ) = 0;

      virtual bool initialise( const KeyerConfig &keyer, const PortConfig &port ) override = 0;
      virtual void select( bool ) = 0;

      virtual bool docommand( const KeyerCtrl &dvp_ctrl ) = 0;

      virtual void getTransverterSwitch(QString &buff);
      virtual bool getPTT( );
      virtual void getActionState( QString & );
      virtual bool getStatus( QString &a );
      virtual bool startMicPassThrough() = 0;
      virtual bool stopMicPassThrough() = 0;
      virtual bool sendCW( const char *message, int speed, int tone ) = 0;

      virtual void initTone1( int ) = 0;
      virtual void initTone2( int, int ) = 0;
      virtual void startTone1() = 0;
      virtual void startTone2() = 0;
      virtual void queueFinished();

      bool started = false;
      int startcount = 20;

      bool L1State = false;
      bool L2State = false;
      bool pttState = false;
      LineModes linesMode = elmNone;
      int transverterSwitch = 0;
      virtual void enableQueue( bool /*b*/ )
      {}
      bool boxRecPending = false;
      bool recPending = false;

      double cwRate = 0.0;
      unsigned long lastIntCount = 0;

      int tone1 = 650;
      int tone2 = 1250;

      bool inTone = false;
};

class sbKeyer
{
   public:
      sbKeyer();
      virtual ~sbKeyer();
      void sbTickEvent();       // this will often be an interrupt routine
      bool sbInitialise(unsigned int rate, int pipTone, int pipVolume, int pipLength , int filterCorner);
      void sbInitTone1( int );
      void sbInitTone2( int, int );
      void sbStartTone1();
      void sbStartTone2();
};

class voiceKeyer: public commonKeyer, public sbKeyer
{
   public:

      voiceKeyer( const KeyerConfig &keyer, const PortConfig &port );
      ~voiceKeyer();
      bool docommand( const KeyerCtrl &dvp_ctrl );

      virtual bool getInfo( KeyerInfo * );
      virtual bool pttChanged( int state );
      virtual bool L1Changed( int state );
      virtual bool L2Changed( int state );
      virtual bool L12Changed( int state, sbControls sbc );
      virtual bool linesModeChanged(int lmode);
      virtual bool initialise( const KeyerConfig &keyer, const PortConfig &port );
      virtual void select( bool );
      virtual bool sendCW( const char *message, int speed, int tone );
      virtual bool startMicPassThrough();
      virtual bool stopMicPassThrough();
      void tickEvent();       // this will often be an interrupt routine
      virtual void initTone1( int );
      virtual void initTone2( int, int );
      virtual void startTone1();
      virtual void startTone2();
};

//=============================================================================


extern commonKeyer *currentKeyer;


//=============================================================================
class ToneAction: public KeyerAction
{
      enum ToneActionStates {etasInitial = -1, etasPTTDelay, etasStartTone, etasStopTone} actionState;
      int nTone;
   public:
      long delayTime;

      virtual void getActionState( QString &s ) override;
      virtual void LxChanged( int line, bool state ) override;
      virtual void pttChanged( bool state ) override;
      virtual void linesModeChanged(int lmode) override;
      virtual void queueFinished() override;
      virtual void timeOut() override;
      virtual void interruptOK( ) override;
      QString statusLetter() override
      {
         return "t";
      }
      ToneAction( int tones, long pdelayStart );
      virtual ~ToneAction() override;
};
//=============================================================================
class VoiceAction: public KeyerAction
{
   public:
      virtual void getActionState( QString &s ) override = 0;
      virtual void LxChanged( int line, bool state ) override = 0;
      virtual void pttChanged( bool state ) override = 0;
      virtual void linesModeChanged(int lmode) override = 0;
      virtual void queueFinished() override = 0;
      virtual void timeOut() override = 0;
      virtual void interruptOK( ) override;
      VoiceAction();
      virtual ~VoiceAction() override;
};
//=============================================================================
class InitialPTTAction: public VoiceAction
{
      enum initPTTActionStates {einitPTTInitial = -1, einitPTTStart, einitPTTEnd, einitPTTFlickRelease, einitPTTRelease} actionState;
   public:
      virtual void getActionState( QString &s ) override;
      virtual void LxChanged( int line, bool state ) override;
      virtual void pttChanged( bool state ) override;
      virtual void linesModeChanged(int lmode) override;
      virtual void queueFinished() override;
      virtual void timeOut() override;
      InitialPTTAction();
      virtual ~InitialPTTAction() override;
      QString statusLetter() override
      {
         return "T";
      }
};
//=============================================================================
class InterruptingPTTAction: public VoiceAction
{
      enum InterruptPTTActionStates {einterPTTInitial = -1, einterPTTWaitDelay,
                                     einterPTTWaitDelayFinish, einterPTTDoPip, einterPTTQuickRelease} actionState;
   public:
      virtual void getActionState( QString &s ) override;
      virtual void LxChanged( int line, bool state ) override;
      virtual void pttChanged( bool state ) override;
      virtual void linesModeChanged(int lmode) override;
      virtual void queueFinished() override;
      virtual void timeOut() override;
      InterruptingPTTAction();
      virtual ~InterruptingPTTAction() override;
      QString statusLetter() override
      {
         return "I";
      }
};
//=============================================================================
class PlayAction: public VoiceAction
{
      int mno;
      QString fileName;
      QString keyName;
      long delayTime;
      long repeatTime;
      bool testMode;
      bool CW;
      enum PlayActionStates {epasInitial = -1, epasWaitRepeat, epasWaitLinear, epasPlayFile,
                             epasEndPlayFile} actionState;
      QString ActionStateString;
   public:
      virtual void getActionState( QString &s ) override;
      virtual void LxChanged( int line, bool state ) override;
      virtual void pttChanged( bool state ) override;
      virtual void linesModeChanged(int lmode) override;
      virtual void queueFinished() override;
      virtual void timeOut() override;
      PlayAction(int mno, const QString &keyName, const QString &fileName, bool noPTT, long delayStart, long repeatDelay, bool firstTime, bool CW );
      virtual ~PlayAction() override;
      QString statusLetter() override
      {
         return "F" + QString::number(mno + 1);
      }
      virtual bool playingFile( const QString & ) override;
};
//=============================================================================
class PipAction: public VoiceAction
{
      enum PipActionStates {epipasInitial = -1, epipasPip, epipasEndPip} actionState;
   public:
      virtual void getActionState( QString &s)  override;
      virtual void LxChanged( int line, bool state ) override;
      virtual void pttChanged( bool state ) override;
      virtual void linesModeChanged(int lmode) override;
      virtual void queueFinished() override;
      virtual void timeOut() override;
      PipAction();
      virtual ~PipAction() override;
      QString statusLetter() override
      {
         return "P";
      }
};
//=============================================================================
class RecordAction: public VoiceAction
{
      QString fileName;
      enum RecordActionStates {erasInitial = -1, erasStartRec, erasStopRec, erasRecFinished, erasRecWaitPTT} actionState;
   public:
      virtual void getActionState( QString &s ) override;
      virtual void LxChanged( int line, bool state ) override;
      virtual void pttChanged( bool state ) override;
      virtual void linesModeChanged(int lmode) override;
      virtual void queueFinished() override;
      virtual void timeOut() override;
      RecordAction( const QString &fileName );
      virtual ~RecordAction() override;
      QString statusLetter() override
      {
         return "R";
      }
};
//=============================================================================
class BoxRecordAction: public VoiceAction
{
      enum BoxRecordActionStates {ebrasInitial = -1} actionState;
   public:
      virtual void getActionState( QString &s ) override;
      virtual void LxChanged( int line, bool state ) override;
      virtual void pttChanged( bool state ) override;
      virtual void linesModeChanged(int lmode) override;
      virtual void queueFinished() override;
      virtual void timeOut() override;
      BoxRecordAction();
      virtual ~BoxRecordAction() override;
      QString statusLetter() override
      {
         return "B";
      }
};
//=============================================================================
#endif
