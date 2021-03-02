/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
#ifndef riffH
#define riffH 
/*===========================================================================
 
      riff.h  -  Don Cross, April 1993.
 
      RIFF file format classes.
      See Chapter 8 of "Multimedia Programmer's Reference" in
      the Microsoft Windows SDK.
 
      See also:
          ..\source\riff.cpp
          ddc.h
 
===========================================================================*/
#include <stdio.h>
#include <QString>
#include "ddc.h"


#pragma pack(push, 1)

uint32_t FourCC ( const char *ChunkName );


enum RiffFileMode
{
   RFM_UNKNOWN,          // undefined type (can use to mean "N/A" or "not open")
   RFM_WRITE,            // open for write
   RFM_READ          // open for read
};


struct RiffChunkHeader
{
   uint32_t ckID;       // Four-character chunk ID
   uint32_t ckSize;     // Length of data in chunk
};


class RiffFile
{
      RiffChunkHeader riff_header;      // header for whole file

   protected:
      RiffFileMode fmode;            // current file I/O mode
      FILE *file;             // I/O stream to use
      DDCRET Seek ( long offset );

   public:
      RiffFile();
      ~RiffFile();

      RiffFileMode CurrentFileMode() const
      {
         return fmode;
      }

      DDCRET Open (const QString &Filename, RiffFileMode NewMode );
      DDCRET Write ( const void *Data, unsigned NumBytes );
      DDCRET Read ( void *Data, unsigned NumBytes );
      DDCRET Expect ( const void *Data, unsigned NumBytes );
      DDCRET Close();

      long CurrentFilePosition() const;

      DDCRET Backpatch ( long FileOffset,
                         const void *Data,
                         unsigned NumBytes );
};


struct WaveFormat_ChunkData
{
   uint16_t wFormatTag;       // Format category (PCM=1)
   uint16_t nChannels;        // Number of channels (mono=1, stereo=2)
   uint32_t nSamplesPerSec;   // Sampling rate [Hz]
   uint32_t nAvgBytesPerSec;
   uint16_t nBlockAlign;
   uint16_t nBitsPerSample;

   void Config ( uint32_t NewSamplingRate = 44100L,
                 uint16_t NewBitsPerSample = 16,
                 uint16_t NewNumChannels = 2 )
   {
      nSamplesPerSec = NewSamplingRate;
      nChannels = NewNumChannels;
      nBitsPerSample = NewBitsPerSample;
      nAvgBytesPerSec = ( nChannels * nSamplesPerSec * nBitsPerSample ) / 8;
      nBlockAlign = ( nChannels * nBitsPerSample ) / 8;
   }

   WaveFormat_ChunkData()
   {
      wFormatTag = 1;     // PCM
      Config();
   }
};


struct WaveFormat_Chunk
{
   RiffChunkHeader header;
   WaveFormat_ChunkData data;

   WaveFormat_Chunk()
   {
      header.ckID = FourCC( "fmt" );
      header.ckSize = sizeof ( WaveFormat_ChunkData );
   }

   bool VerifyValidity()
   {
      return header.ckID == FourCC( "fmt" ) &&

             ( data.nChannels == 1 || data.nChannels == 2 ) &&

             data.nAvgBytesPerSec == ( data.nChannels *
                                       data.nSamplesPerSec *
                                       data.nBitsPerSample ) / 8 &&

             data.nBlockAlign == ( data.nChannels *
                                   data.nBitsPerSample ) / 8;
   }
};


#define  MAX_WAVE_CHANNELS   2


struct WaveFileSample
{
   uint16_t chan [ MAX_WAVE_CHANNELS ];
};


class WaveFile: private RiffFile
{
      WaveFormat_Chunk wave_format;
      RiffChunkHeader pcm_data;
      long pcm_data_offset;  // offset of 'pcm_data' in output file
      qint64 num_samples;

   public:
      WaveFile();
      ~WaveFile()
      {
         Close();
      }

      DDCRET OpenForWrite ( const QString &Filename,
                            uint32_t SamplingRate = 44100L,
                            uint16_t BitsPerSample = 16,
                            uint16_t NumChannels = 2 );

      DDCRET OpenForRead ( const QString &Filename );

      DDCRET ReadSample ( int16_t Sample [ MAX_WAVE_CHANNELS ] );
      DDCRET WriteSample (const int16_t Sample[] );
      DDCRET SeekToSample ( unsigned long SampleIndex );

      // The following work only with 16-bit audio
      DDCRET WriteData ( const int16_t *data, uint32_t numData );
      DDCRET ReadData ( int16_t *data, uint32_t numData );

      // The following work only with 8-bit audio
      DDCRET WriteData ( const uint8_t *data, uint32_t numData );
      DDCRET ReadData ( uint8_t *data, uint32_t numData );

      DDCRET ReadSamples ( uint32_t num, WaveFileSample[] );

      DDCRET WriteMonoSample ( int16_t ChannelData );
      DDCRET WriteStereoSample ( int16_t LeftChannelData, int16_t RightChannelData );

      DDCRET ReadMonoSample ( int16_t *ChannelData );
      DDCRET ReadStereoSample ( int16_t *LeftSampleData, int16_t *RightSampleData );

      DDCRET Close();

      uint32_t SamplingRate() const;    // [Hz]
      uint16_t BitsPerSample() const;
      uint16_t NumChannels() const;
      uint32_t NumSamples() const;

      // Open for write using another wave file's parameters...

      DDCRET OpenForWrite ( const char *Filename,
                            WaveFile &OtherWave )
      {
         return OpenForWrite ( Filename,
                               OtherWave.SamplingRate(),
                               OtherWave.BitsPerSample(),
                               OtherWave.NumChannels() );
      }

      long CurrentFilePosition() const
      {
         return RiffFile::CurrentFilePosition();
      }
};

#pragma pack(pop)

#endif /* __DDC_RIFF_H */


/*--- end of file riff.h ---*/
