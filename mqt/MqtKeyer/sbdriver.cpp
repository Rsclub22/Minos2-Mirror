/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
#include "base_pch.h"

#include "riff.h"
#include "cutils.h"
#include "keyers.h" 
#include "sbdriver.h"
#include "soundsys.h"

//==============================================================================
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

      bool LoadFile( QString &errmess )
      {
         if ( sblog )
         {
            trace( "Trying to open file " + fileName );
         }
         loaded = false;
         delete[] fptr;
         fptr = nullptr;
         // should be initiated by keyer, which should call the sound engine
         WaveFile inWave;
         int ret = inWave.OpenForRead( fileName );
         if ( ret != DDC_SUCCESS )
         {
            errmess = "Invalid WAV file " + fileName + "\n";
            if ( sblog )
            {
               trace( errmess );
            }
            return false;
         }
         else
         {
            rate = inWave.SamplingRate();
            BitsPerSample = inWave.BitsPerSample();
            NumChannels = inWave.NumChannels();
            frec = false;
            fsample = inWave.NumSamples();

            if ( rate == static_cast<unsigned int>(sampleRate) && BitsPerSample == 16 && NumChannels == 2 )
            {
               fptr = new int16_t[ fsample * 2 ];
               if ( inWave.ReadData( fptr, fsample * 2 ) == DDC_SUCCESS )
               {
                  if ( sblog )
                  {
                     trace( "File " + fileName + " opened samples = " + QString::number( fsample ) );
                  }
                  loaded = true;
               }
               else
               {
                  loaded = false;
               }
            }
            else
            {
               if ( sblog )
               {
                  trace( "File " + fileName + " wrong data format" );
               }
               loaded = false;
            }

         }
         return loaded;
      }
      dvkFile()
      {}
      ~dvkFile()
      {
         delete[] fptr;
         fptr = nullptr;
      }
};
#define MAXFILES 10
static QVector <dvkFile *> recfil;
//==============================================================================

bool SoundSystemDriver::dofile( int i, int clipRecord )
{
   // mixer set should be set already

   ptr = nullptr;
   if ( sblog )
   {
      trace( "dofile(" + QString::number( i ) + ")" + "play = " + QString::number( play ) );
   }
   ihand = i;

   // stop the DMA before we start it again
   stopDMA();

   if ( ihand >= 0 )
   {
      if ( !recording && !recfil[ ihand ] ->loaded )
      {
         return false;
      }
      samples = static_cast<uint32_t>((recfil[ ihand ] ->fsample - clipRecord * ( rate / 1000.0 )) * 2);
      ptr = recfil[ ihand ] ->fptr;

      if ( sblog )
      {
         trace( "samples = " + QString::number( samples ) );
      }
   }
   else
      if ( play && ihand == DOFILE_PIP )
      {
         // set mic off, wave out on
         ptr = pipptr;
         samples = pipSamples * 2;
         if ( sblog )
         {
            trace( "pipSamples = " + QString::number( pipSamples ) );
         }
      }
      else
         if ( play && ihand == DOFILE_T1 )
         {
            ptr = t1ptr;
            samples = toneSamples * 2;
         }
         else
            if ( play && ihand == DOFILE_T2 )
            {
               ptr = t2ptr;
               samples = toneSamples * 2;
            }
            else
               if ( play && ihand == DOFILE_CW )
               {
                  ptr = cwptr;
                  samples = cwSamples * 2;
               }
   soundSystem->samples = samples;
   soundSystem->dataptr = ptr;

   /*
    start record/playback!
   */ 
   unsigned long psamples = 0;
   int16_t *pdataptr = nullptr;
   KeyerAction * sba = KeyerAction::getCurrentAction();
   int pipSDS = 0;
   if (sba && sba->tailWithPip)
   {
       psamples = pipSamples * 2;
       pdataptr = pipptr;
       pipSDS = sba->pipStartDelaySamples;
   }

   return soundSystem->startDMA( play, ( ( ihand >= 0 ) ? recfil[ ihand ] ->fileName : QString( "" ) ),
                                 psamples, pdataptr, pipSDS);
}


void SoundSystemDriver::stoprec()
{
   if ( sblog )
   {
      trace( "stoprec()" );
   }

   if ( recording )
   {
      if ( currentKeyer )
         currentKeyer->ptt( 0 );
      stopDMA();  // stop - eventually
      CW_ACTIVE = false;
      ihand = isave;
      recfil[ isave ] ->fsample = samples;
      recording = false;
      QString err;
      recfil[ isave ] ->LoadFile( err );
   }
}

void SoundSystemDriver::record_file( const QString &filename )
{
   if ( sblog )
   {
      trace( "record_file(" + filename + ")" );
   }
   stoprec();
   stopDMA();  // stop - eventually
   CW_ACTIVE = false;
   if ( currentKeyer )
      currentKeyer->ptt( 0 );
   int i;
   for ( i = 0;i < recfil.size();i++ )
   {
      if ( filename.compare(recfil[ i ] ->fileName, Qt::CaseInsensitive ) == 0 )
         break;
   }
   if ( i < recfil.size() )
   {
      delete [] recfil[ i ] ->fptr;  // clear out the data buffer
      recfil[ i ] ->fptr = nullptr;
   }
   else
   {
      i = recfil.size() + 1;
      if ( i > MAXFILES )
      {
         return ;
      }
      recfil[ i ] ->fileName = filename;
   }

   recfil[ i ] ->loaded = false;
   recfil[ i ] ->rate = rate;
   recfil[ i ] ->frec = true;
   play = false;
   isave = i;
   recording = true;
   dofile( i, 0 );
}
bool SoundSystemDriver::play_file( const QString &filename, bool xmit )
{
   if ( sblog )
   {
      trace( "play_file(" + filename + ", " + makeStr( xmit ) + ")" );
   }
   stoprec();
   stopDMA();  // stop - eventually
   CW_ACTIVE = false;
   if ( !xmit && currentKeyer )
      currentKeyer->ptt( 0 );

   if ( filename.compare("AudioCWFile", Qt::CaseInsensitive ) == 0 )
   {
      if ( cwptr && cwSamples )
      {
         if ( xmit && currentKeyer )
         {
            currentKeyer->ptt( 1 );
         }
         play = true;
         recording = false;
         dofile( DOFILE_CW, 0 );
      }
      return cwSamples;
   }
   else
   {
      int i;
      for ( i = 0;i < recfil.size();i++ )
      {
         if ( filename.compare(recfil[ i ] ->fileName, Qt::CaseInsensitive ) == 0 )
            break;
      }
      if ( i < recfil.size() )
      {
         if ( recfil[ i ] ->loaded && xmit && currentKeyer )
         {
            currentKeyer->ptt( 1 );
         }
         play = true;
         recording = false;
         if (currentKeyer && dofile( i, currentKeyer->kconf.clipRecord ) )
            return recfil[ i ] ->fsample;
      }
      return false;
   }
}
void SoundSystemDriver::stopall()
{
    trace("stopall");
   ptr = nullptr;
   stoprec();
   stopDMA();  // stop - eventually
   CW_ACTIVE = false;
   if ( currentKeyer )
      currentKeyer->ptt( 0 );

   delete [] t1ptr;
   delete [] t2ptr;
   delete [] cwptr;
   t1ptr = nullptr;
   t2ptr = nullptr;
   toneSamples = 0;
   cwptr = nullptr;
   cwSamples = 0;
}
bool SoundSystemDriver::startMicPassThrough()
{
   return soundSystem->startMicPassThrough();
}
bool SoundSystemDriver::stopMicPassThrough()
{
    return soundSystem->stopMicPassThrough();
}
void SoundSystemDriver::setVolumeMults(int record, int replay, int passThrough)
{
    soundSystem->setVolumeMults(record, replay, passThrough);
}

bool SoundSystemDriver::sbdvp_init( QString &errmess, unsigned int srate, int pipTone, int pipVolume, int pipLength, int filterCorner )
{
   // should be done from config when the sb is defined as in use.

   // make sure we don't do this twice!
   if ( loadFailed )
   {
      errmess = "Sound card not initialised successfully\n";
      return false;
   }
   if ( !init_done )
   {
      init_done = true;
      loadFailed = true;

      init_OK = true;	// it will have to be undone properly

      // should be in each sound class

      rate = soundSystem->setRate(srate);
      soundSystem->setFilter(filterCorner);

      if ( !soundSystem->initialise( errmess ) )
         return false;

      if ( currentKeyer )
         currentKeyer->ptt( 0 );

      play = true;
      recording = false;

      stopDMA();  // stop - eventually
      CW_ACTIVE = false;

      // clear the recfil structure

      for ( int i = 0; i < recfil.size(); i++ )
         delete recfil[ i ];
      recfil.clear();
      // attempt to set up all recorded files

      for ( unsigned int fileno = 0;fileno < MAXFILES;fileno++ )
      {
         dvkFile *dvk = new dvkFile;

         char buff[ 100 ];
         sprintf( buff, "CQF%d.WAV", fileno );
         dvk->fileName = buff;
         dvk->sampleRate = rate;

         dvk->LoadFile( errmess );
         recfil.push_back( dvk );
      }
      loadFailed = false;

   }
   createPipTone( errmess, pipTone, pipVolume, pipLength ); // outside conditional to allow for CW calls with -1

   return init_OK;
}


// need to call this from dvpctrl
/*static*/void SoundSystemDriver::sbdvp_unload( )
{
   if ( singleton_sb )
   {
      singleton_sb->unload();
      delete singleton_sb;
      singleton_sb = nullptr;
   }
}
void SoundSystemDriver::unload( )
{
   if ( sblog )
   {
      trace( "sbDriver::unload()" );
   }
   // should be initiated by keyer, which should call the sound engine
   // when we close everything down (i.e. unregister the monitors, etc.
   if ( init_done && init_OK )
   {

      init_done = false;
      init_OK = false;

      stopall();
      KeyerAction::currentAction.freeAll();

      //   CLOSE_PCKEYER();
      for ( int i = 0; i < recfil.size(); i++ )
         delete recfil[ i ];
      recfil.clear();
   }

   delete [] pipptr;
   delete [] t1ptr;
   delete [] t2ptr;
   delete [] cwptr;
   if ( soundSystem )
   {
      delete soundSystem;
      soundSystem = nullptr;
   }
}
//==============================================================================

void SoundSystemDriver::genTone(int16_t *dest, bool add
                           , int tone, unsigned int samples, unsigned int rtime, double volmult, int16_t *enddest )
{
   if ( sblog )
   {
      trace( "genTone(" + QString::number( tone ) + ", "
             + QString::number( samples ) + ", "
             + QString::number( rtime ) + ", "
             + QString::number( volmult ) + ")" );
   }

   double phaseStep = (2 * pi * tone) / rate;
   double phase = 0.0;

#define CHUNKSIZE 1024

   int16_t *buff = new int16_t [ CHUNKSIZE ];


   for ( unsigned long buffstart = 0; buffstart < samples * 2; buffstart += CHUNKSIZE * 2 )
   {
      int16_t * destptr = dest + buffstart;
      unsigned long i;
      for ( i = 0; i < CHUNKSIZE && buffstart + i*2 < samples * 2; i++ )
      {
          double x = volmult * sin(phase);
         if ( buffstart + i * 2 < rtime * 2 )
         {
             // ramp up
             int offset = ( buffstart + i * 2)/2;
            buff[ i ] = int16_t( x * (offset / rtime) );
            if (abs(buff[i]) > 32767)
            {
               mShowMessage( "Big sum...", nullptr );
            }
         }
         else
         {
            if ( buffstart + i * 2 > ( samples - rtime ) * 2 )
            {
                // ramp down
                int offset = (samples * 2 - ( buffstart + i * 2 ) )/2;
               buff[ i ] = int16_t( x * ( offset / rtime ) );
               if (abs(buff[i]) > 32767)
               {
                  mShowMessage( "Big sum...", nullptr );
               }
            }
            else
            {
                // constant level
               buff[ i ] = int16_t( x );	// not full volume
               if (abs(buff[i]) > 32767)
               {
                  mShowMessage( "Big sum...", nullptr );
               }
            }
         }

         phase += phaseStep;
         while(phase > 2*pi)
         {
             phase -= 2*pi;
         }

      }
      // write (or add in place) i bytes to the handle

      if ( add )
         {
            for ( unsigned long j = 0; j < i ; j++ )
            {
               if ( destptr[ j * 2 ] + buff[ j ] > 32767 || destptr[ j * 2] + buff[ j ] < -32768 )
               {
                  mShowMessage( "Big sum...", nullptr );
               }
               destptr[ j * 2 ] = destptr[ j * 2 ] + buff[ j ];
               destptr[ j * 2 + 1] = destptr[ j * 2 + 1] + buff[ j ];
            }
         }
      else
      {
          for (unsigned long j = 0; j < i; j++)
          {
              if (&destptr[j * 2 + 1] > enddest)
                  mShowMessage( "bad data...", nullptr );
              destptr[j * 2] = buff[j];
              destptr[j * 2 + 1] = buff[j];
          }
      }
   }
   delete [] buff;
   buff = nullptr;
}
bool SoundSystemDriver::createPipTone( QString &/*errmess*/, int pipTone, int pipVolume, int pipLength )
{
   // should be initiated by keyer, which should call the sound engine
   if ( pipTone > 0 && pipVolume > 0 && pipLength > 0 && ( pipTone != oldpip || pipVolume != oldpipVolume || pipLength != oldpipLength ) )
   {
      oldpip = pipTone;
      oldpipVolume = pipVolume;

      delete [] pipptr;

      pipSamples = static_cast<unsigned long>(rate * ( pipLength / 1000.0 ));
      unsigned int ramptime = pipSamples / 6;
      pipptr = new int16_t [ pipSamples * 2 ];

      const double volmult = 32767.0 * pipVolume / 100.0;
      genTone( pipptr, false, pipTone, pipSamples, ramptime, volmult, &pipptr[pipSamples * 2 - 1] );
   }
   return true;
}
//==============================================================================
SoundSystemDriver *SoundSystemDriver::singleton_sb = nullptr;
SoundSystemDriver *SoundSystemDriver::getSbDriver()
{
   if ( singleton_sb )
      return singleton_sb;

   singleton_sb = new SoundSystemDriver();
   singleton_sb->ready = true;      // not until we are out of the constructor
   return singleton_sb;
}
SoundSystemDriver::SoundSystemDriver()
{
   soundSystem = RtAudioSoundSystem::createSoundSystem();
   connect(soundSystem, &RtAudioSoundSystem::interruptOK, this, &SoundSystemDriver::interruptOK);
   connect(soundSystem, &RtAudioSoundSystem::setActionTime1, this, &SoundSystemDriver::setActionTime1);
   connect(soundSystem, &RtAudioSoundSystem::actionQueueFinished, this, &SoundSystemDriver::actionQueueFinished);
   connect(soundSystem, &RtAudioSoundSystem::setVU, this, &SoundSystemDriver::setVU);
}
SoundSystemDriver::~SoundSystemDriver()
{
   if ( sblog )
   {
      trace( "sbDriver::~sbDriver" );
   }
   delete soundSystem;
}
void SoundSystemDriver::interruptOK()
{
    KeyerAction * sba = KeyerAction::getCurrentAction();
    if ( sba )
       sba->interruptOK();	// so as we do not time it out immediately

}
void SoundSystemDriver::setActionTime1()
{
    KeyerAction * sba = KeyerAction::getCurrentAction();
     if ( sba )
     {
        sba->actionTime = 1;    // force to stop
     }
}
void SoundSystemDriver::actionQueueFinished()
{
    KeyerAction * sba = KeyerAction::getCurrentAction();
     if ( sba )
     {
        sba->queueFinished();
     }
}
void SoundSystemDriver::setVU(unsigned int a, unsigned int b, unsigned int c)
{
    WinVUCallback( a, b, c );

}
//==============================================================================
void SoundSystemDriver::initTone1( int t1 )
{
   delete [] t1ptr;
   toneSamples = rate * static_cast<unsigned long>(tuneTime);

   t1ptr = new int16_t [ toneSamples * 2 ];

   genTone( t1ptr, false, t1, toneSamples, 5000 /* ramp time*/, ( tuneLevel / 100 ) * 32767.0, &t1ptr[toneSamples * 2 - 1] );
}
void SoundSystemDriver::initTone2( int t1, int t2 )
{
   delete [] t2ptr;

   toneSamples = rate * static_cast<unsigned long>(tuneTime);
   t2ptr = new int16_t [ toneSamples * 2 ];
   // ((tuneLevel/100) * 32767.0) / sqrt( 2 ) is an attempt to get equal power - but it can result
   // in peak levels that are out of range, so what to do? Flat top it?
   // Or insist on lower levels?
   genTone( t2ptr, false, t1, toneSamples, 5000 /* ramp time*/, ( ( tuneLevel / 100 ) * 32767.0 ) / 2, &t2ptr[toneSamples * 2 - 1] );
   genTone( t2ptr, true, t2, toneSamples, 5000 /* ramp time*/, ( ( tuneLevel / 100 ) * 32767.0 ) / 2, &t2ptr[toneSamples * 2 - 1] );
}
void SoundSystemDriver::startTone1()
{
   play = true;
   dofile( DOFILE_T1, 0 );
}
void SoundSystemDriver::startTone2()
{
   play = true;
   dofile( DOFILE_T2, 0 );
}
// WEIGHT gives the number of elements that make a dit/dah.
// The reminder of 32 (dit) or 64(dah) is the inter-element gap
/*
static unsigned char WEIGHT[ 4 ][ 2 ] =
   {{14, 46},           //0.9:1
    {16, 48},           //1.0:1
    {18, 50},           //1.1:1
    {20, 52}};      //1.2:1
*/
// letterspace is 32 (+ prior interelement gap)
// wordspace is 64 (plus prior element/letter gap)

// Using PARIS as standard word, u is unit (dit) length in seconds, c is wpm

// u = c / 1.2

void SoundSystemDriver::createCWBuffer( const char *message, int speed, int tone )
{
   if ( cwptr )
      delete [] cwptr;
   cwSamples = 0;

   unsigned int unitlength = static_cast<unsigned int>(rate * ( 1.2 / speed ));   // number of samples per unit

   unsigned int ditlength = unitlength;
   unsigned int dahlength = unitlength * 3;
   unsigned int ramptime = unitlength / 8;     // length of ramp on each end of element

   unsigned int fulldit = ditlength + unitlength;
   unsigned int fulldah = dahlength + unitlength;
   unsigned int letterspace = unitlength * 2;
   unsigned int wordspace = unitlength * 4;

   int16_t *ditbuff = new int16_t [ fulldit ];
   memset ( ditbuff, 0, static_cast<size_t>(fulldit * 2) );
   int16_t *dahbuff = new int16_t [ fulldah * 2 ];
   memset ( dahbuff, 0, static_cast<size_t>(fulldah * 2) );

   genTone( ditbuff, false, tone, ditlength, ramptime, 32767.0 * 0.9, &ditbuff[fulldit * 2 - 1] );
   genTone( dahbuff, false, tone, dahlength, ramptime, 32767.0 * 0.9, &dahbuff[fulldah * 2 - 1] );

   size_t messlen = strlen( message );
   unsigned int messsamples = 0;
   for ( size_t i = 0; i < messlen; i++ )
   {
      if ( message[ i ] == ' ' )
      {
         messsamples += wordspace;
      }
      else
      {
         try
         {
            QString codestr = MORSECODE[ toupper( message[ i ] ) ];
            for ( int B = 0; B < codestr.size(); B++ )
            {
               if ( codestr[ B ] == '\x40' )
                  messsamples += fulldit;
               else
                  if ( codestr[ B ] == '\x80' )
                     messsamples += fulldah;
            }
            messsamples += letterspace;
         }
         catch ( ... )
         {
            // what should we do if the letter was not found?
         }
      }
   }

   // now we know how large a buffer to allocate

   cwSamples = messsamples;
   cwptr = new int16_t [ messsamples ];
   memset( cwptr, 0, cwSamples * 2 );
   messsamples = 0;
   for ( size_t i = 0; i < messlen; i++ )
   {
      if ( message[ i ] == ' ' )
      {
         messsamples += wordspace;
      }
      else
      {
         try
         {
            QString codestr = MORSECODE[ toupper( message[ i ] ) ];
            for ( int B = 0; B < codestr.size(); B++ )
            {
               if ( codestr[ B ] == '\x40' )
               {
                  memcpy( &cwptr[ messsamples ], ditbuff, fulldit * 2 );
                  messsamples += fulldit;
               }
               else
                  if ( codestr[ B ] == '\x80' )
                  {
                     memcpy( &cwptr[ messsamples ], dahbuff, fulldah * 2 );
                     messsamples += fulldah;
                  }
            }
            messsamples += letterspace;
         }
         catch ( ... )
         {
            // what should we do if the letter was not found?
         }
      }
   }

   delete [] ditbuff;
   delete [] dahbuff;
   // and now we have a sample filled buffer ready to send
   if ( sblog )
   {
      trace( "cwTone = " + QString::number( tone ) );
      trace( "rate = " + QString::number( speed ) );
      trace( "cwSamples = " + QString::number( cwSamples ) );
      trace( "ramptime = " + QString::number( ramptime ) );
   }
}
//==============================================================================
void SoundSystemDriver::stopDMA()
{
   soundSystem->stopDMA();
}
//==============================================================================
