/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
#ifndef sbdriverH
#define sbdriverH
#include "keyctrl.h"

enum sbControls {ePTT, eL1, eL2};

#define DOFILE_PIP -1
#define DOFILE_T1 -2
#define DOFILE_T2 -3
#define DOFILE_CW -4

class RtAudioSoundSystem;
class SoundSystemDriver
{
   private:
      // another singleton to handle the sb card
      // it may need callbacks to interested parties!
      static SoundSystemDriver *singleton_sb;
      RtAudioSoundSystem *soundSystem = nullptr;
      //===============================================================
      // working values, read from control on entry, written back on exit

      unsigned long CurrMasterLevel = 0;
      unsigned long CurrRecLevel = 0;
      unsigned long CurrMicLevel = 0;

      //===============================================================

      int oldpip = -1;
      int oldpipVolume = -1;
      int oldpipLength = 0;
      double lastCWRate = 0;

      int ihand = -1;
      int isave = -1;

      void unload( );
public:
      volatile int recording = false;
      bool ready = false;
      bool loadFailed = false;

      bool CW_ACTIVE = false;
      bool init_done = false;
      bool init_OK = false;

      int cwTone = -1;

      unsigned long pipSamples = 0;
      int16_t * pipptr = nullptr;

      unsigned long toneSamples = 0;
      int16_t * t1ptr = nullptr;
      int16_t * t2ptr = nullptr;

      unsigned long cwSamples = 0;
      int16_t * cwptr = nullptr;

      int16_t *ptr = nullptr;       /* data for current file */
      uint32_t samples = 0;   /* fsample for current file  */
      char play;  /* Play or record */

      unsigned int rate = 0;   /* rate in Hertz -- this gets reset to nearest available value */

      VUCallBack WinVUCallback = nullptr;

      void setVolumeMults(int record, int replay, int passThrough);

      bool dofile( int i, int clipRecord = 0 );
      void stoprec();
      void record_file( const QString &filename );
      bool play_file( const QString &filename, bool xmit );
      void stopall();
      void stopDMA();
      bool startMicPassThrough();
      bool stopMicPassThrough();

      bool rdenv( QString &errmess, QString &in );

      void genTone(int16_t *tptr, bool add
                       , int tone, unsigned int samples, unsigned int ramptime, double vmult , int16_t *enddest);
      bool createPipTone( QString &errmess, int pipTone, int pipVolume, int pipLength );
      static void sbdvp_unload( );
      void initTone1( int );
      void initTone2( int, int );
      void startTone1();
      void startTone2();
      void createCWBuffer( const char *message, int speed, int tone );

      bool sbdvp_init(QString &errmess, unsigned int rate, int pipTone, int pipVolume, int pipLength , int filterCorner);
      SoundSystemDriver();
      ~SoundSystemDriver();

      static SoundSystemDriver *getSbDriver();
};
#endif
