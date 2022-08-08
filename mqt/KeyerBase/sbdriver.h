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
#include <QObject>
#include <QVector>
#include "CompressorParams.h"

class dvkFile
{
   public:
      QString fileName;
      bool loaded = false;
      bool frec = false;          // flag set to true if audio has been recorded
      unsigned int sampleRate = 0;       // system required sample rate
      unsigned long fsample = 0;        // number of bytes for each sound files
      int16_t *fptr = nullptr;          // data area for each sound file
      unsigned int rate = 0;
      int BitsPerSample = 0;
      int NumChannels = 0;

      bool LoadFile( QString &errmess );
      dvkFile();
      ~dvkFile();
};
#define MAXFILES 8

enum sbControls {ePTT, eL1, eL2};

#define DOFILE_PIP -1
#define DOFILE_T1 -2
#define DOFILE_T2 -3
#define DOFILE_CW -4

class RtAudioSoundSystem;
class SoundSystemDriver:public QObject
{
   Q_OBJECT
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

      QVector <dvkFile *> recfil;

      void unload( );
public:
      SoundSystemDriver();
      virtual ~SoundSystemDriver();

      static SoundSystemDriver *getSbDriver();

      const dvkFile *getFile(int fno);


      int recording = false;
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
      bool play;  /* Play or record */

      unsigned int rate = 0;   /* rate in Hertz -- this gets reset to nearest available value */

      void setVolumeMults(int record, int replay, int passThrough, const CompressorParams &compression, bool df, bool dc);

      int getMessageLen(int buttonNumber);
      bool dofile( int i, int clipRecord = 0 );
      void stoprec();
      void record_file( const QString &filename );
      bool play_file(const QString &filename, bool xmit , int clipRecord);
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

      bool initialise(QString ind, QString outd);
      bool sbdvp_init(QString ind, QString outd, QString &errmess, unsigned int rate, int pipTone, int pipVolume, int pipLength );
      QStringList getInputDevices();
      QStringList getOutputDevices();
      void closedown();
private slots:
      void interruptOK();
      void outputFinished();
      void actionQueueFinished();
      void doSetVU(unsigned int a, unsigned int b, unsigned int c);

signals:
      void ptt(bool);
      void recpbFinished();
      void setVU(unsigned int a, unsigned int b, unsigned int c);
};
#endif
