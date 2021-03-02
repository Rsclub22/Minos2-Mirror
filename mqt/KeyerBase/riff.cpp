/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
#include "base_pch.h"

/*==========================================================================
 
	 riff.cpp  -  Don Cross, April 1993.
 
	 Contains code for reading/writing RIFF files, including .WAV files!
 
	 See also:
		 ..\include\riff.h
		 ..\include\ddc.h
 
	 Revision history:
 
1993 April 12 [Don Cross]
	 Started writing this code.
 
1994 October 6 [Don Cross]
	 Added WriteData, ReadData.
	 Added const modifiers to some function parameters.
 
1997 September 14 [Don Cross]
     Fixed a bug in WaveFile::Close.  It was calling Backpatch
	 and getting confused when the the file had been opened for read.
	 (Backpatch returns an error in that case, which prevented
	 WaveFile::Close from calling RiffFile::Close.)
 
==========================================================================*/

// DDCLIB includes
#include "riff.h"


uint32_t FourCC ( const char *ChunkName )
{
   uint32_t retbuf = 0x20202020L;   // four spaces (padding)
   char *p = ( reinterpret_cast<char *>(& retbuf) );

   // Remember, this is Intel format!
   // The first character goes in the LSB

   for ( int i = 0; i < 4 && ChunkName[ i ]; i++ )
   {
      *p++ = ChunkName[ i ];
   }

   return retbuf;
}


//----------------------------------------------------------------------


RiffFile::RiffFile()
{
   file = nullptr;
   fmode = RFM_UNKNOWN;

   riff_header.ckID = FourCC( "RIFF" );
   riff_header.ckSize = 0;
}


RiffFile::~RiffFile()
{
   if ( fmode != RFM_UNKNOWN )
   {
      Close();
   }
}


DDCRET RiffFile::Open ( const QString &Filename, RiffFileMode NewMode )
{
   DDCRET retcode = DDC_SUCCESS;

   if ( fmode != RFM_UNKNOWN )
   {
      retcode = Close();
   }

   if ( retcode == DDC_SUCCESS )
   {
      switch ( NewMode )
      {
         case RFM_WRITE:
            file = fopen ( Filename.toLatin1(), "wb" );
            if ( file )
            {
               // Write the RIFF header...
               // We will have to come back later and patch it!

               if ( fwrite ( &riff_header, sizeof( riff_header ), 1, file ) != 1 )
               {
                  fclose( file );
                  unlink( Filename.toLatin1() );
                  fmode = RFM_UNKNOWN;
                  file = nullptr;
                  retcode = DDC_FILE_ERROR;
               }
               else
               {
                  fmode = RFM_WRITE;
               }
            }
            else
            {
               fmode = RFM_UNKNOWN;
               retcode = DDC_FILE_ERROR;
            }
            break;

         case RFM_READ:
            {
               file = fopen ( Filename.toLatin1(), "rb" );
               if ( file )
               {
                  // Try to read the RIFF header...

                  if ( fread ( &riff_header, sizeof( riff_header ), 1, file ) != 1 )
                  {
                     fclose( file );
                     fmode = RFM_UNKNOWN;
                     file = nullptr;
                     retcode = DDC_FILE_ERROR;
                  }
                  else
                  {
                     fmode = RFM_READ;
                  }
               }
               else
               {
                  fmode = RFM_UNKNOWN;
                  retcode = DDC_FILE_ERROR;
               }
            }
            break;

         default:
            retcode = DDC_INVALID_CALL;
      }
   }

   return retcode;
}


DDCRET RiffFile::Write ( const void *Data, unsigned NumBytes )
{
   if ( fmode != RFM_WRITE )
   {
      return DDC_INVALID_CALL;
   }

   if ( fwrite ( Data, NumBytes, 1, file ) != 1 )
   {
      return DDC_FILE_ERROR;
   }

   riff_header.ckSize += NumBytes;

   return DDC_SUCCESS;
}


DDCRET RiffFile::Close()
{
   DDCRET retcode = DDC_SUCCESS;

   switch ( fmode )
   {
      case RFM_WRITE:
         if ( fflush( file ) ||
              fseek( file, 0, SEEK_SET ) ||
              fwrite ( &riff_header, sizeof( riff_header ), 1, file ) != 1 ||
              fclose( file ) )
         {
            retcode = DDC_FILE_ERROR;
         }
         break;

      case RFM_READ:
         fclose( file );
         break;

       case RFM_UNKNOWN:
           break;

   }

   file = nullptr;
   fmode = RFM_UNKNOWN;

   return retcode;
}


long RiffFile::CurrentFilePosition() const
{
   return ftell ( file );
}


DDCRET RiffFile::Seek ( long offset )
{
   fflush ( file );

   DDCRET rc;

   if ( fseek ( file, offset, SEEK_SET ) )
   {
      rc = DDC_FILE_ERROR;
   }
   else
   {
      rc = DDC_SUCCESS;
   }

   return rc;
}


DDCRET RiffFile::Backpatch ( long FileOffset,
                             const void *Data,
                             unsigned NumBytes )
{
   if ( !file )
   {
      return DDC_INVALID_CALL;
   }

   if ( fflush( file ) ||
        fseek ( file, FileOffset, SEEK_SET ) )
   {
      return DDC_FILE_ERROR;
   }

   DDCRET dcr = Write ( Data, NumBytes );
   riff_header.ckSize -= NumBytes;	// as Write upped it...

   return dcr;
}


DDCRET RiffFile::Expect ( const void *Data, unsigned NumBytes )
{
   const char * p = reinterpret_cast<const char *>(Data);

   while ( NumBytes-- )
   {
      if ( fgetc( file ) != *p++ )
      {
         return DDC_FILE_ERROR;
      }
   }

   return DDC_SUCCESS;
}


DDCRET RiffFile::Read ( void *Data, unsigned NumBytes )
{
   DDCRET retcode = DDC_SUCCESS;

   if ( fread( Data, NumBytes, 1, file ) != 1 )
   {
      retcode = DDC_FILE_ERROR;
   }

   return retcode;
}


//-----------------------------------------------------------------------

WaveFile::WaveFile()
{
   pcm_data.ckID = FourCC( "data" );
   pcm_data.ckSize = 0;
   num_samples = 0;
}


DDCRET WaveFile::OpenForRead ( const QString &Filename )
{
   // Verify filename parameter as best we can...
   if ( Filename.isEmpty() )
   {
      return DDC_INVALID_CALL;
   }

   qint64 flen = FileLength(Filename);

   DDCRET retcode = Open ( Filename, RFM_READ );

   if ( retcode == DDC_SUCCESS )
   {
      retcode = Expect ( "WAVE", 4 );

      if ( retcode == DDC_SUCCESS )
      {
         retcode = Read ( &wave_format, sizeof( wave_format ) );

         if ( retcode == DDC_SUCCESS &&
              !wave_format.VerifyValidity() )
         {
            // This isn't standard PCM, so we don't know what it is!

            retcode = DDC_FILE_ERROR;
         }

         if ( retcode == DDC_SUCCESS )
         {
            pcm_data_offset = CurrentFilePosition();

            // Figure out number of samples from
            // file size, current file position, and
            // WAVE header.



            retcode = Read ( &pcm_data, sizeof( pcm_data ) );
            num_samples = flen - CurrentFilePosition();
            num_samples /= NumChannels();
            num_samples /= ( BitsPerSample() / 8 );
         }
      }
   }

   return retcode;
}


DDCRET WaveFile::OpenForWrite ( const QString &Filename,
                                uint32_t SamplingRate,
                                uint16_t BitsPerSample,
                                uint16_t NumChannels )
{
   // Verify parameters...

   if ( Filename.isEmpty() ||
        ( BitsPerSample != 8 && BitsPerSample != 16 ) ||
        NumChannels < 1 || NumChannels > 2 )
   {
      return DDC_INVALID_CALL;
   }

   wave_format.data.Config ( SamplingRate, BitsPerSample, NumChannels );

   DDCRET retcode = Open ( Filename, RFM_WRITE );

   if ( retcode == DDC_SUCCESS )
   {
      retcode = Write ( "WAVE", 4 );

      if ( retcode == DDC_SUCCESS )
      {
         retcode = Write ( &wave_format, sizeof( wave_format ) );

         if ( retcode == DDC_SUCCESS )
         {
            pcm_data_offset = CurrentFilePosition();
            retcode = Write ( &pcm_data, sizeof( pcm_data ) );
         }
      }
   }

   return retcode;
}


DDCRET WaveFile::Close()
{
   DDCRET rc = DDC_SUCCESS;

   if ( fmode == RFM_WRITE )
   {
      rc = Backpatch ( pcm_data_offset, &pcm_data, sizeof( pcm_data ) );
      if ( rc == DDC_SUCCESS )
      {
         rc = RiffFile::Close();
      }
   }
   return rc;
}


DDCRET WaveFile::WriteSample ( const int16_t Sample [ MAX_WAVE_CHANNELS ] )
{
   DDCRET retcode = DDC_SUCCESS;

   switch ( wave_format.data.nChannels )
   {
      case 1:
         switch ( wave_format.data.nBitsPerSample )
         {
            case 8:
               pcm_data.ckSize += 1;
               retcode = Write ( &Sample[ 0 ], 1 );
               break;

            case 16:
               pcm_data.ckSize += 2;
               retcode = Write ( &Sample[ 0 ], 2 );
               break;

            default:
               retcode = DDC_INVALID_CALL;
         }
         break;

      case 2:
         switch ( wave_format.data.nBitsPerSample )
         {
            case 8:
               retcode = Write ( &Sample[ 0 ], 1 );
               if ( retcode == DDC_SUCCESS )
               {
                  retcode = Write ( &Sample[ 1 ], 1 );
                  if ( retcode == DDC_SUCCESS )
                  {
                     pcm_data.ckSize += 2;
                  }
               }
               break;

            case 16:
               retcode = Write ( &Sample[ 0 ], 2 );
               if ( retcode == DDC_SUCCESS )
               {
                  retcode = Write ( &Sample[ 1 ], 2 );
                  if ( retcode == DDC_SUCCESS )
                  {
                     pcm_data.ckSize += 4;
                  }
               }
               break;

            default:
               retcode = DDC_INVALID_CALL;
         }
         break;

      default:
         retcode = DDC_INVALID_CALL;
   }

   return retcode;
}


DDCRET WaveFile::WriteMonoSample ( int16_t SampleData )
{
   switch ( wave_format.data.nBitsPerSample )
   {
      case 8:
         pcm_data.ckSize += 1;
         return Write ( &SampleData, 1 );

      case 16:
         pcm_data.ckSize += 2;
         return Write ( &SampleData, 2 );
   }

   return DDC_INVALID_CALL;
}


DDCRET WaveFile::WriteStereoSample ( int16_t LeftSample,
                                     int16_t RightSample )
{
   DDCRET retcode = DDC_SUCCESS;

   switch ( wave_format.data.nBitsPerSample )
   {
      case 8:
         retcode = Write ( &LeftSample, 1 );
         if ( retcode == DDC_SUCCESS )
         {
            retcode = Write ( &RightSample, 1 );
            if ( retcode == DDC_SUCCESS )
            {
               pcm_data.ckSize += 2;
            }
         }
         break;

      case 16:
         retcode = Write ( &LeftSample, 2 );
         if ( retcode == DDC_SUCCESS )
         {
            retcode = Write ( &RightSample, 2 );
            if ( retcode == DDC_SUCCESS )
            {
               pcm_data.ckSize += 4;
            }
         }
         break;

      default:
         retcode = DDC_INVALID_CALL;
   }

   return retcode;
}



DDCRET WaveFile::ReadSample ( int16_t Sample [ MAX_WAVE_CHANNELS ] )
{
   DDCRET retcode = DDC_SUCCESS;

   switch ( wave_format.data.nChannels )
   {
      case 1:
         switch ( wave_format.data.nBitsPerSample )
         {
            case 8:
               unsigned char x;
               retcode = Read ( &x, 1 );
               Sample[ 0 ] = int16_t( x );
               break;

            case 16:
               retcode = Read ( &Sample[ 0 ], 2 );
               break;

            default:
               retcode = DDC_INVALID_CALL;
         }
         break;

      case 2:
         switch ( wave_format.data.nBitsPerSample )
         {
            case 8:
               unsigned char x[ 2 ];
               retcode = Read ( x, 2 );
               Sample[ 0 ] = int16_t ( x[ 0 ] );
               Sample[ 1 ] = int16_t ( x[ 1 ] );
               break;

            case 16:
               retcode = Read ( Sample, 4 );
               break;

            default:
               retcode = DDC_INVALID_CALL;
         }
         break;

      default:
         retcode = DDC_INVALID_CALL;
   }

   return retcode;
}


DDCRET WaveFile::ReadSamples ( uint32_t num, WaveFileSample sarray[] )
{
   DDCRET retcode = DDC_SUCCESS;

   switch ( wave_format.data.nChannels )
   {
      case 1:
         switch ( wave_format.data.nBitsPerSample )
         {
            case 8:
               for ( unsigned i = 0; i < num && retcode == DDC_SUCCESS; i++ )
               {
                  unsigned char x;
                  retcode = Read ( &x, 1 );
                  sarray[ i ].chan[ 0 ] = static_cast<uint16_t>( x );
               }
               break;

            case 16:
               for ( unsigned i = 0; i < num && retcode == DDC_SUCCESS; i++ )
               {
                  retcode = Read ( &sarray[ i ].chan[ 0 ], 2 );
               }
               break;

            default:
               retcode = DDC_INVALID_CALL;
         }
         break;

      case 2:
         switch ( wave_format.data.nBitsPerSample )
         {
            case 8:
               for ( unsigned i = 0; i < num && retcode == DDC_SUCCESS; i++ )
               {
                  unsigned char x[ 2 ];
                  retcode = Read ( x, 2 );
                  sarray[ i ].chan[ 0 ] = static_cast<uint16_t> ( x[ 0 ] );
                  sarray[ i ].chan[ 1 ] = static_cast<uint16_t> ( x[ 1 ] );
               }
               break;

            case 16:
               retcode = Read ( sarray, 4 * num );
               break;

            default:
               retcode = DDC_INVALID_CALL;
         }
         break;

      default:
         retcode = DDC_INVALID_CALL;
   }

   return retcode;
}


DDCRET WaveFile::ReadMonoSample ( int16_t *Sample )
{
   DDCRET retcode = DDC_SUCCESS;

   switch ( wave_format.data.nBitsPerSample )
   {
      case 8:
         unsigned char x;
         retcode = Read ( &x, 1 );
         *Sample = int16_t( x );
         break;

      case 16:
         retcode = Read ( Sample, 2 );
         break;

      default:
         retcode = DDC_INVALID_CALL;
   }

   return retcode;
}


DDCRET WaveFile::ReadStereoSample ( int16_t *L, int16_t *R )
{
   DDCRET retcode = DDC_SUCCESS;
   uint8_t x[ 2 ];
   int16_t y[ 2 ];

   switch ( wave_format.data.nBitsPerSample )
   {
      case 8:
         retcode = Read ( x, 2 );
         *L = int16_t ( x[ 0 ] );
         *R = int16_t ( x[ 1 ] );
         break;

      case 16:
         retcode = Read ( y, 4 );
         *L = int16_t ( y[ 0 ] );
         *R = int16_t ( y[ 1 ] );
         break;

      default:
         retcode = DDC_INVALID_CALL;
   }

   return retcode;
}


DDCRET WaveFile::SeekToSample ( unsigned long SampleIndex )
{
   if ( SampleIndex >= NumSamples() )
   {
      return DDC_INVALID_CALL;
   }

   unsigned SampleSize = ( BitsPerSample() + 7 ) / 8;

   DDCRET rc = Seek (static_cast<long>( pcm_data_offset + static_cast<long>(sizeof( pcm_data )) +
                     static_cast<long>( SampleSize * NumChannels() * SampleIndex )));

   return rc;
}


uint32_t WaveFile::SamplingRate() const
{
   return wave_format.data.nSamplesPerSec;
}


uint16_t WaveFile::BitsPerSample() const
{
   return wave_format.data.nBitsPerSample;
}


uint16_t WaveFile::NumChannels() const
{
   return wave_format.data.nChannels;
}


uint32_t WaveFile::NumSamples() const
{
   return static_cast<uint32_t>(num_samples);
}


DDCRET WaveFile::WriteData ( const int16_t *data, uint32_t numData )
{
   uint32_t extraBytes = numData * sizeof( int16_t );
   pcm_data.ckSize += extraBytes;
   return RiffFile::Write ( data, extraBytes );
}


DDCRET WaveFile::WriteData ( const uint8_t *data, uint32_t numData )
{
   pcm_data.ckSize += numData;
   return RiffFile::Write ( data, numData );
}


DDCRET WaveFile::ReadData ( int16_t *data, uint32_t numData )
{
   return RiffFile::Read ( data, numData * sizeof( int16_t ) );
}


DDCRET WaveFile::ReadData ( uint8_t *data, uint32_t numData )
{
   return RiffFile::Read ( data, numData );
}


/*--- end of file riff.cpp ---*/
