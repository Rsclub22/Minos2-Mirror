/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////

#include <QTextStream>
#include "LoggerContest.h"
#include "LoggerContacts.h"
#include "ContestApp.h"
#include "AdifImport.h"
#include "MShowMessageDlg.h"
#include "adifmanager.h"
#include "cutils.h"
#include "fileutils.h"
#include "reg1test.h"
#include "cabrillo.h"
#include "printfile.h"
#include "enqdlg.h"
#include "MinosTestImport.h"
#include "MinosTestExport.h"
#include "BandList.h"
#include "latlong.h"
#include "calcs.h"
#include "MinosParameters.h"
#include "MTrace.h"

#include "LoggerContest.h"

LoggerContestLog::LoggerContestLog(bool hf) : BaseContestLog(hf),
      minosFile( false ),
      GJVFile( false ),
      logFile( false ), adifFile( false ), ediFile( false ),
      needExport( false ),
      clusterFilterSettingsExist( false),
      bandmapFilterSettingsExist( false)
{

    // clusterClientFrame
    lastSpotTabTime = QDateTime::currentDateTimeUtc();
    lastCallsignTabTime = QDateTime::currentDateTimeUtc();
    lastLocatorTabTime = QDateTime::currentDateTimeUtc();

}
void LoggerContestLog::makeContact(bool timeNow, QSharedPointer<BaseContact> &lct )
{
    bool initReport;
    TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpReadabilityInit, initReport );
    lct = QSharedPointer<BaseContact>(new ContestContact( this, timeNow, initReport ));
}
LoggerContestLog::~LoggerContestLog()
{
    adifManager.reset();    // clear it earlier than would be natural
}
void LoggerContestLog::initialiseINI()
{
   entryBundle.setProfile( BundleFile::bundleFiles[ epENTRYPROFILE ] );
   QTHBundle.setProfile( BundleFile::bundleFiles[ epQTHPROFILE ] );
   stationBundle.setProfile( BundleFile::bundleFiles[ epSTATIONPROFILE ] );
   entryBundleName.setInitialValue( entryBundle.getSection() );
   QTHBundleName.setInitialValue( QTHBundle.getSection() );
   stationBundleName.setInitialValue( stationBundle.getSection() );
}
void LoggerContestLog::clearDirty()
{
   entryBundleName.clearDirty();
   QTHBundleName.clearDirty();
   stationBundleName.clearDirty();

   entrant.clearDirty();
   sqth1.clearDirty();
   sqth2.clearDirty();
   entTx.clearDirty();
   entRx.clearDirty();
   entAnt.clearDirty();
   entAGL.clearDirty();
   entASL.clearDirty();
   radioName.clearDirty();
   antennaName.clearDirty();
   ops1.clearDirty();
   ops2.clearDirty();
   entCondx1.clearDirty();
   entCondx2.clearDirty();
   entCondx3.clearDirty();
   entCondx4.clearDirty();
   entName.clearDirty();
   entCall.clearDirty();
   entSect.clearDirty();
   entAddr1.clearDirty();
   entAddr2.clearDirty();
   entPostCode.clearDirty();
   entCity.clearDirty();
   entCountry.clearDirty();
   entPhone.clearDirty();
   entEMail.clearDirty();
   sectionList.clearDirty();
   bearingOffset.clearDirty();

   zoomLevel.clearDirty();
   centreLat.clearDirty();
   centreLon.clearDirty();


   for(auto &m:runMemories)
   {
       for (auto &r: m)
       {
           r.clearDirty();
       }
   }
   for (int i = 0; i < rigMemories.size(); i++)
   {
       rigMemories[i].clearDirty();
   }


   clusterFilterSettings.clearDirty();
   bandmapFilterSettings.clearDirty();

   screenLayout.clearDirty();
   currentFKeySet.clearDirty();
   statsPeriod1.clearDirty();
   statsPeriod2.clearDirty();
   showContinentEU.clearDirty();
   showContinentAS.clearDirty();
   showContinentAF.clearDirty();
   showContinentOC.clearDirty();
   showContinentSA.clearDirty();
   showContinentNA.clearDirty();
   showWorkedCountries.clearDirty();
   showUnworkedCountries.clearDirty();
   showWorkedDistricts.clearDirty();
   showUnworkedDistricts.clearDirty();
   for (int i = 0; i < STACKITEMS; i++)
   {
        currentStackItems[i].clearDirty();
   }

   watchedADIFFile.clearDirty();
   watchedADIFLastOffset.clearDirty();
   BaseContestLog::clearDirty();
}
void LoggerContestLog::setDirty()
{
   entryBundleName.setDirty();
   QTHBundleName.setDirty();
   stationBundleName.setDirty();

   entrant.setDirty();
   sqth1.setDirty();
   sqth2.setDirty();
   entTx.setDirty();
   entRx.setDirty();
   entAnt.setDirty();
   entAGL.setDirty();
   entASL.setDirty();
   radioName.setDirty();
   antennaName.setDirty();
   sectionList.setDirty();
   ops1.setDirty();
   ops2.setDirty();
   entCondx1.setDirty();
   entCondx2.setDirty();
   entCondx3.setDirty();
   entCondx4.setDirty();
   entName.setDirty();
   entCall.setDirty();
   entSect.setDirty();
   entAddr1.setDirty();
   entAddr2.setDirty();
   entPostCode.setDirty();
   entCity.setDirty();
   entCountry.setDirty();
   entPhone.setDirty();
   entEMail.setDirty();
   bearingOffset.setDirty();

   zoomLevel.setDirty();
   centreLat.setDirty();
   centreLon.setDirty();


   for(auto &m:runMemories)
   {
       for (auto &r: m)
       {
           r.setDirty();
       }
   }
   for (int i = 0; i < rigMemories.size(); i++)
   {
       rigMemories[i].setDirty();
   }


   clusterFilterSettings.setDirty();
   bandmapFilterSettings.setDirty();

   screenLayout.setDirty();
   currentFKeySet.setDirty();
   statsPeriod1.setDirty();
   statsPeriod2.setDirty();
   showContinentEU.setDirty();
   showContinentAS.setDirty();
   showContinentAF.setDirty();
   showContinentOC.setDirty();
   showContinentSA.setDirty();
   showContinentNA.setDirty();
   showWorkedCountries.setDirty();
   showUnworkedCountries.setDirty();
   showWorkedDistricts.setDirty();
   showUnworkedDistricts.setDirty();
   for (int i = 0; i < STACKITEMS; i++)
   {
        currentStackItems[i].setDirty();
   }
   watchedADIFFile.setDirty();
   watchedADIFLastOffset.setDirty();
   BaseContestLog::setDirty();
}
bool LoggerContestLog::initialise( int sno )
{
   if ( !TContestApp::getContestApp() ->insertContest( this, sno ) )
   {
	  return false;
   }
   cslotno = sno;
   return true;
}

bool LoggerContestLog::initialise( const QString &fn, bool newFile, int slotno )
{
   if ( !initialise( slotno ) )
      return false;

   if ( TContestApp::getContestApp() ->isContestOpen( fn ) )
   {
      return false;
   }

   // open the settings bundle files
   initialiseINI();

   // preset the stacked info

   statsPeriod1.setInitialValue(MinosParameters::getMinosParameters() ->getStatsPeriod1());
   statsPeriod2.setInitialValue(MinosParameters::getMinosParameters() ->getStatsPeriod2());

   bool bTemp;
   MinosParameters::getMinosParameters() ->getBoolDisplayProfile( edpShowContinentEU, bTemp );
   showContinentEU.setInitialValue(bTemp);
   MinosParameters::getMinosParameters() ->getBoolDisplayProfile( edpShowContinentAS, bTemp );
   showContinentAS.setInitialValue(bTemp);
   MinosParameters::getMinosParameters() ->getBoolDisplayProfile( edpShowContinentAF, bTemp );
   showContinentAF.setInitialValue(bTemp);
   MinosParameters::getMinosParameters() ->getBoolDisplayProfile( edpShowContinentOC, bTemp );
   showContinentOC.setInitialValue(bTemp);
   MinosParameters::getMinosParameters() ->getBoolDisplayProfile( edpShowContinentSA, bTemp );
   showContinentSA.setInitialValue(bTemp);
   MinosParameters::getMinosParameters() ->getBoolDisplayProfile( edpShowContinentNA, bTemp );
   showContinentNA.setInitialValue(bTemp);

   MinosParameters::getMinosParameters() ->getBoolDisplayProfile( edpShowWorkedCountries, bTemp );
   showWorkedCountries.setInitialValue(bTemp);
   MinosParameters::getMinosParameters() ->getBoolDisplayProfile( edpShowUnworkedCountries, bTemp );
   showUnworkedCountries.setInitialValue(bTemp);
   MinosParameters::getMinosParameters() ->getBoolDisplayProfile( edpShowWorkedDistricts, bTemp );
   showWorkedDistricts.setInitialValue(bTemp);
   MinosParameters::getMinosParameters() ->getBoolDisplayProfile( edpShowUnworkedDistricts, bTemp );
   showUnworkedDistricts.setInitialValue(bTemp);

   QString temp;
   MinosParameters::getMinosParameters() -> getStringDisplayProfile( edpDefaultLayout, temp );
   screenLayout.setInitialValue(temp);

   temp.clear(); // Initial value now comes from the screen config

   currentFKeySet.setInitialValue("Default");

   for (int i = 0; i < STACKITEMS; i++)
   {
        currentStackItems[i].setInitialValue(temp);
   }

   // open the LoggerContestLog file


   cfileName = GetFullPath(fn);
   QString ext = ExtractFileExt( fn );
   publishedName = ExtractFileName( fn );
   if ( ext.compare(".gjv", Qt::CaseInsensitive ) == 0 )
   {
      GJVFile = true;
      setUnwriteable(true);
   }
   else
       if ( ext.compare(".minos", Qt::CaseInsensitive ) == 0 )
      {
         minosFile = true;
      }
      else
         if ( ext.compare(".log", Qt::CaseInsensitive ) == 0 )
         {
            logFile = true;
            setUnwriteable(true);
         }
         else
            if ( ext.compare(".adi", Qt::CaseInsensitive ) == 0 )
            {
               adifFile = true;
               setUnwriteable(true);
            }
            else
               if ( ext.compare(".edi", Qt::CaseInsensitive ) == 0 )
               {
                  ediFile = true;
                  setUnwriteable(true);
               }
               else
               {
                  MinosParameters::getMinosParameters() ->mshowMessage( tr("Not a known file type! (%1)").arg(ext) );
                  return false;
               }
   if ( !newFile )
   {
      QIODevice::OpenMode om = QIODevice::ReadWrite | QIODevice::Unbuffered;
      QSharedPointer<QFile> contestFile(new QFile(fn));

      if (!FileExists(fn) || !contestFile->open(om))
      {
          // isWriteable doesn't give good results on Windows
          om = QIODevice::ReadOnly;
          if (!contestFile->open(om))
          {
             QString lerr = contestFile->errorString();
             QString emess = tr("Failed to open Contest file %1 : %2 ").arg(fn, lerr);
             MinosParameters::getMinosParameters() ->mshowMessage( emess );
             return false;
          }
          setUnwriteable(true);
      }

      bool loadOK = false;
      needExport = false;

      if ( GJVFile )
      {
         GJVcontestFile = contestFile;
         if ( !GJVload() )     // load the header so that we can display it
            return false;
         loadOK = GJVloadContacts();
      }
      else
         if ( minosFile )
         {
            minosContestFile = contestFile;
            MinosTestImport mt( this );
            ct_stanzaCount = mt.importTest( minosContestFile );
            if ( ct_stanzaCount > 0 )
            {
               // set the bundles accordingly
               entryBundle.openSection( entryBundleName.getValue() );
               QTHBundle.openSection( QTHBundleName.getValue() );
               stationBundle.openSection( stationBundleName.getValue() );
               loadOK = true;
            }
            else
            {
               mShowMessage(tr("File %1 is empty, so cannot be opened").arg(fn), nullptr);
            }
         }
         else
            if ( logFile )
            {
               logContestFile = contestFile;
               loadOK = importLOG( logContestFile );
               needExport = true;
            }
            else
               if ( adifFile )
               {
                  adifContestFile = contestFile;
                  loadOK = importAdif( adifContestFile );
                  needExport = true;
               }
               else
                  if ( ediFile )
                  {
                     ediContestFile = contestFile;
                     loadOK = importReg1Test( ediContestFile );
                     needExport = true;
                  }

      scanContest();    // after log initialise/load/import
      clearDirty();  // what we have just read CAN'T be dirty
//      validateLoc();    // this was done in scanContest
      // run_contest_dialog has already loaded the LoggerContestLog and set log_count
      // here, we display a "loading" box
      if ( isUnwriteable() )     // Minos files can be unprotected if not realy RO
         closeFile();				// to preserve file handles

      if ( !loadOK )    // sets ct as well
      {
         return false;
      }
      // and create the ADIF manager if required
      QString wadif = watchedADIFFile.getValue();
      if (!wadif.isEmpty())
      {
          qint64 lo = watchedADIFLastOffset.getValue();
          adifManager = QSharedPointer<AdifManager>(new AdifManager(this, wadif, lo));
      }
   }
   else
      if ( minosFile )
      {
          //CreateDir(fn);
          QIODevice::OpenMode om = QIODevice::ReadWrite | QIODevice::Unbuffered;
          if (isUnwriteable())
          {
            om = QIODevice::ReadOnly;
          }
          QSharedPointer<QFile> contestFile(new QFile(fn));

          if (!contestFile->open(om))
          {
             QString lerr = contestFile->errorString();
             QString emess = tr("Failed to create Contest file %1 : %2").arg(fn, lerr);
             MinosParameters::getMinosParameters() ->mshowMessage( emess );
             return false;
          }

          if ( minosFile )
          {
             minosContestFile = contestFile;
          }
      }
      else
      {
         return false;
      }

   setVersion(STRINGVERSION);
   commonSave( newFile );

   checkAgeProtection();

   return true;
}
void LoggerContestLog::checkAgeProtection()
{
    // check last time in contest against current date/time and the

    bool doAge;
    TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpAgeProtectContests, doAge );
    if(!doAge)
    {
        ageProtected = false;
    }
    else
    {
        int ageDays;
        TContestApp::getContestApp() ->loggerBundle.getIntProfile( elpAgeToProtectContests, ageDays );
        if (ageDays >= 0)
        {
            QString t1 = DTGEnd.getValue();
            QDateTime end = CanonicalToTDT( t1 );

            QDate endDate = end.date();

            endDate = endDate.addDays(ageDays);
            if (endDate.isValid() && endDate < QDate::currentDate())
            {
                // e.g. if ageDays is 1, allow contest day and all the following day
                ageProtected = true;
            }
        }
        else
        {
            ageProtected = false;
        }
    }
}
qint64 LoggerContestLog::readBlock( int bno )
{
    bool sres = GJVcontestFile->seek(bno * bsize);
   if ( !sres)
   {
      MinosParameters::getMinosParameters() ->mshowMessage( tr("(read) seek failed!") );
   }

   qint64 rsize = GJVcontestFile->read(diskBuffer, bsize);

   diskBuffer[ rsize ] = 0;
   if ( rsize < bsize )
      MinosParameters::getMinosParameters() ->mshowMessage( tr("Short read") );

   return rsize;
}
qint64 LoggerContestLog::writeBlock(QSharedPointer<QFile> fd, int bno )
{
   // fd will not be contest_file if we are exporting a GJV file

   if ( !fd.data() )
      return bsize;		//tell lies

   int n = atoi( diskBuffer );
   if ( n != bno )
      MinosParameters::getMinosParameters() ->mshowMessage( tr("Invalid block number for write!!") );

   bool sres = fd->seek(bno * bsize);
   if ( !sres )
   {
      MinosParameters::getMinosParameters() ->mshowMessage( tr("(write) seek failed!") );
   }
   qint64 ret = fd->write(diskBuffer, bsize);
   if ( ret != bsize )
   {
      MinosParameters::getMinosParameters() ->mshowMessage( tr("bad reply from write!") );
   }
   return ret;
}
void LoggerContestLog::closeFile( )
{
   if ( GJVcontestFile.data() )
   {
      GJVcontestFile->close();
   }
   if ( minosContestFile.data() )
   {
      minosContestFile->close();
   }
   if ( logContestFile.data() )
   {
      logContestFile->close();
   }
   if ( adifContestFile.data() )
   {
      adifContestFile->close();
   }
   if ( ediContestFile.data() )
   {
      ediContestFile->close();
   }

   GJVcontestFile.reset();
   minosContestFile.reset();
   logContestFile.reset();
   adifContestFile.reset();
   ediContestFile.reset();
}
QSharedPointer<BaseContact> LoggerContestLog::addContact( int newctno, unsigned short extraFlags, bool saveNew, bool catchup,
                                                          QString mode, QString mgmSubmode, dtg ctTime, const Frequency &freq )
{
   // add the contact number as an new empty contact, with disk block and log_seq

   bool timenow = true;
   if ( ( extraFlags & TO_BE_ENTERED ) || catchup )
   {
      timenow = false;
   }

   QSharedPointer<BaseContact> bct;
   makeContact( timenow, bct );
   if (timenow == false)
   {
       bct->timeOn.setValue(ctTime);
       bct->timeOff.setValue(ctTime);
   }

   QString temp = QString( "%1" ).arg(newctno, 3, 10, QChar('0') );  //leading zeros
   bct->serials.setValue( temp );
   bct->setLogSequence( nextBlock << 16 );
   nextBlock++;
   if ( newctno > maxSerial )
   {
      maxSerial = newctno;
   }
   bct->contactFlags.setValue( bct->contactFlags.getValue() | extraFlags );
   bct->mode.setValue(mode);
   bct->mgmSubmode.setValue(mgmSubmode);

   if (!freq.isClear())
   {
        bct->setFrequency(freq, currentBand.getValue());
   }

   if (catchup)
   {
      bct->op1 = currentOp1;
      bct->op2 = currentOp2;
   }
   if ( saveNew )
   {
      bct->commonSave(bct);		// make sure contact is correct
   }
   addToContestList(bct);
   if ( saveNew )
   {
      commonSave( false );
   }

   return bct;
}
QSharedPointer<BaseContact> LoggerContestLog::addContactBetween(QSharedPointer<BaseContact> prior, QSharedPointer<BaseContact> next, dtg ctTime )
{
   // add the contact number as an new empty contact, with disk block and log_seq

   if (!next)
   {
      MinosParameters::getMinosParameters() ->mshowMessage(tr("Attempt to insert after last contact - not allowed. Please report a bug!"));
      return QSharedPointer<BaseContact>();
   }
   bool timenow = false;

   QSharedPointer<BaseContact> bct;
   makeContact( timenow, bct );

   bct->timeOn = ctTime;
   bct->timeOff = ctTime;
   bct->serials.setValue( "" );
   bct->mode.setValue(prior->mode.getValue());
   bct->mgmSubmode.setValue(prior->mgmSubmode.getValue());

   unsigned long pls =  prior?prior->getLogSequence():0;
   unsigned long nls =  next->getLogSequence();

   unsigned long seq = (pls + nls)/2;

   bct->setLogSequence( seq );

   bct->commonSave(bct);		// make sure contact is correct
   addToContestList(bct);
   commonSave( false );

   return bct;
}
//==========================================================================
void LoggerContestLog::clearRunMemory(int memno, const memoryData::memData &mem)
{
    QString band(mem.band);
    if (band.isEmpty())
    {
        band = BandList::getBand(mem.freq);
    }
    if (runMemories[band].size() >= memno + 1)
    {
        memoryData::memData m;
        m.band = band;
        runMemories[band][memno ].setValue(m);
        commonSave(false);
    }

}
void LoggerContestLog::saveRunMemory(int memno, const memoryData::memData &mem)
{
    QString band(mem.band);
    if (band.isEmpty())
    {
        band = BandList::getBand(mem.freq);
    }

    // after "clear" band will be blank...

    if (runMemories[band].size() < memno + 1)
    {
        runMemories[band].resize(memno + 1);
    }
    memoryData::memData m(mem);
    m.band = band;
    runMemories[band][memno ].setValue(m);
    commonSave(false);
}
void LoggerContestLog::saveInitialRunMemory(int memno, const memoryData::memData &mem)
{
    QString band( mem.band);
    if (band.isEmpty())
    {
        band = BandList::getBand(mem.freq);
    }
    if (runMemories[band].size() < memno + 1)
    {
        runMemories[band].resize(memno + 1);
    }
    memoryData::memData m(mem);
    m.band = band;
    runMemories[band][memno].setInitialValue(m);

}
//==========================================================================
void LoggerContestLog::saveRigMemory(int memno, const memoryData::memData &mem)
{
    if (rigMemories.size() < memno + 1)
    {
        rigMemories.resize(memno + 1);
    }
    memoryData::memData m = mem;
    m.memno = memno;
    rigMemories[memno].setValue(m);
    commonSave(false);
}
void LoggerContestLog::saveInitialRigMemory(int memno, const memoryData::memData &mem)
{
    if (rigMemories.size() < memno + 1)
    {
        rigMemories.resize(memno + 1);
    }
    memoryData::memData m = mem;
    m.memno = memno;
    rigMemories[memno].setInitialValue(m);
}
memoryData::memData LoggerContestLog::getRigMemoryData(int memoryNumber)
{
    memoryData::memData m;
    if (rigMemories.size() > memoryNumber)
    {
        m = rigMemories[memoryNumber].getValue();
        if (m.locator.isEmpty())
        {
        }
        else
        {
            Locator loc;
            loc.setLoc(m.locator);

            double lon = 0.0;
            double lat = 0.0;

            if ( lonlat( loc.getLoc(), lon, lat, MinosParameters::getMinosParameters() ->getAllowLoc4() ) == LOC_OK )
            {
                double dist;
                int brg;
                disbeara( lon, lat, dist, brg );
                m.bearing = brg;
            }
        }
    }
    return m;
}

//==========================================================================

void LoggerContestLog::saveClusterFilter(const ClusterClientFilterSettings &ccfs)
{
    clusterFilterSettings.setValue(ccfs);
    commonSave(false);
}
void LoggerContestLog::saveInitialClusterFilter(const ClusterClientFilterSettings &ccfs)
{
    clusterFilterSettings.setInitialValue(ccfs);

}
ClusterClientFilterSettings LoggerContestLog::getClusterFilter()
{

        return clusterFilterSettings.getValue();
}

//==========================================================================

void LoggerContestLog::saveBandmapFilter(const BandmapClientFilterSettings &bcfs)
{
    bandmapFilterSettings.setValue(bcfs);
    commonSave(false);
}
void LoggerContestLog::saveInitialBandmapFilter(const BandmapClientFilterSettings &bcfs)
{
    bandmapFilterSettings.setInitialValue(bcfs);

}
BandmapClientFilterSettings LoggerContestLog::getBandmapFilter()
{

        return bandmapFilterSettings.getValue();
}


//==========================================================================
bool LoggerContestLog::commonSave( bool newfile )
{
   if ( !isReadOnly() )
   {
      if ( GJVFile )
      {
         MinosParameters::getMinosParameters() ->mshowMessage( tr(".GJV should have been opened read only!") );
         return false; // it better be read only!
      }
      else
         if ( minosFile )
         {
            return minosSaveFile( newfile );
         }
   }
   return false;
}
//==========================================================================
bool LoggerContestLog::minosSaveFile( bool newfile )
{
   MinosTestExport mt( this );
   ct_stanzaCount = mt.exportAllDetails( minosContestFile, newfile );
   clearDirty();
   return true;
}
bool LoggerContestLog::minosSaveContestContact( const QSharedPointer<BaseContact> lct )
{
   MinosTestExport mt( this );
   ct_stanzaCount = mt.exportQSO( minosContestFile, lct );
   return true;
}
//==========================================================================
bool LoggerContestLog::GJVsave( GJVParams &gp )
{
   const QString nulc;

   // save the LoggerContestLog details to file_desc
   clearBuffer();
   buffpt = 0;

   strtobuf( "0" );    // block number for LoggerContestLog block
   strtobuf( GJVVERSION );
   strtobuf( contestBands );
   currentBand = contestBands;
   strtobuf( name );
   strtobuf( mycall.getFullCall() );
   strtobuf( myloc.getLoc() );
   strtobuf( location );
   
   opyn( exchangeRequired );
   opyn( false /*Country_mult && County_mult*/ );	//CC_mult
   opyn( locMult );

   QString temp = QString::number( gp.count ); // leave off the dummy entry!
   strtobuf( temp );

   strtobuf( power );

   strtobuf( nulc );
   //   strtobuf( mainOpNow );
   strtobuf( nulc );
   //   strtobuf( secondOpNow );
   strtobuf( currentMode );
   opyn( true );						// RADIAL rings are DEAD
   opyn( false );                // was post entry
   opyn( scoreMode.getValue() == PPQSO );
   opyn( countryMult );
   opyn( false /*County_mult*/ );

   opyn( districtMult );

   opyn( powerWatts );
   opyn( false /*scoreMode == GSPECIAL*/ );
   opyn( allowLoc8 );

   opyn( !RSTMandatoryField.getValue() );
   opyn( !serialMandatoryField.getValue() );
   opyn( !locatorMandatoryField.getValue() );

   strtobuf(); // clear tail

   QSharedPointer<QFile> fd = gp.fd;

   writeBlock( fd, 0 );

   // note that each contact will be saved as entered
   return true;
}
bool LoggerContestLog::GJVload( )
{
   QString temp;
   logCount = 0;

   // load the LoggerContestLog details from file_desc
   clearBuffer();
   readBlock( 0 );
   buffpt = 0;
   buftostr( temp );
   if ( temp.toInt() != 0 )
   {
      MinosParameters::getMinosParameters() ->mshowMessage( tr("Invalid block 0 in .GJV Contest file") );
      return false;
   }

   buftostr( temp );
   if ( strnicmp( temp, GJVVERSION, VERLENGTH ) != 0 )
   {
      MinosParameters::getMinosParameters() ->mshowMessage( tr( "Invalid Contest file format (%1, %2 expected)" ).arg(temp, GJVVERSION) );
      return false;
   }

   buftostr( contestBands );
   buftostr( name );
   buftostr( temp );
   mycall.setFullCall( temp );
   buftostr( temp );
   myloc.setLoc(temp);
   buftostr( location );
   
   exchangeRequired.setValue( inyn() );
   inyn();
   locMult.setValue( inyn() );

   buftostr( temp );
   logCount = temp.toInt();

   buftostr( power );

   buftostr( temp );
   //   buftostr( mainOpNow );

   buftostr( temp );
   //   buftostr( secondOpNow );

   buftostr( currentMode );
   inyn();
   scoreMode.setValue( PPKM );			// don't take any notice of radial flag!

   inyn();        // was post entry
   if ( inyn() )
   {
      scoreMode.setValue( PPQSO );
   }
   countryMult.setValue( inyn() );
   inyn();
   districtMult.setValue( inyn() );

   powerWatts.setValue( inyn() );

   if ( inyn() )
   {
      //      scoreMode = GSPECIAL;
   }

   allowLoc8.setValue( inyn() );
   RSTMandatoryField.setValue( !inyn() );
   serialMandatoryField.setValue( !inyn() );
   locatorMandatoryField.setValue( !inyn() );

   return true;

}
bool LoggerContestLog::GJVloadContacts( )
{
   nextBlock = 1;

   for ( int i = 0; i < logCount; i++ )
   {
      QSharedPointer<BaseContact>bct;
      makeContact( false, bct );
      if ( bct->GJVload( static_cast< int >( nextBlock) ) )
      {
         nextBlock++;
         addToContestList(bct);
         int maxct = bct->serials.getValue().toInt();
         if ( maxct > maxSerial )
            maxSerial = maxct;

      }
   }
   return true;
}
bool LoggerContestLog::export_contest(QSharedPointer<QFile> expfd, ExportType exptype, bool noSerials )
{
   bool ret = false;
   commonSave( false );
   setDirty();                // We always need to export everything
   switch ( exptype )
   {
         //enum ExportType {EREG1TEST, EADIF, EG0GJV, EMINOS };

      case EG0GJV:
         ret = exportGJV( expfd );
         break;

      case EMINOS:
         ret = exportMinos( expfd );
         break;

      case EADIF:
         ret = exportADIF( expfd );
         break;

      case EKML:
         ret = exportKML( expfd );
         break;

      case EREG1TEST:
         ret = exportREG1TEST( expfd, noSerials );
         break;

      case ECABRILLO:
         ret = exportCabrillo(expfd);
         break;

      case EPRINTFILE:
         ret = exportPrintFile(expfd);
         break;
   }
   clearDirty();    // BUT don't leave it dirty!!
   return ret;
}

bool LoggerContestLog::exportGJV(QSharedPointer<QFile>fd )
{
   //	straight copy to disk

   int mind = 1;
   int maxd = maxSerial;
   if ( !enquireDialog(   /*Owner*/nullptr,tr( "Please give first serial to be written"), mind ) )
      return false;
   if ( !enquireDialog(   /*Owner*/nullptr, tr("Please give last serial to be written"), maxd ) )
      return false;

   int mindump = qMin( mind, maxd );
   int maxdump = qMax( mind, maxd );

   QString temp = tr( "Writing all contacts between serials %1 and %2 inclusive" ).arg(mindump).arg(maxdump );
   if ( !MinosParameters::getMinosParameters() ->yesNoMessage( nullptr, temp ) )
      return false;

   GJVParams gp( fd );

   GJVsave( gp );

   bool inDump = false;

   for ( auto const &i: QASCONST(ctList ))
   {
      QSharedPointer<BaseContact> lct = i.wt;
      // we need to test for "in dump"

      int serials = lct->serials.getValue().toInt();
      // dump the contact, until serial seen

      if ( ( serials == mindump ) || ( mindump == 0 ) )
         inDump = true;

      if ( inDump && ( serials <= maxdump ) )
      {
         if ( !lct->GJVsave( gp ) )
            break;
         gp.count++;
      }

      if ( serials >= maxdump )
      {
         break;	// as all dumped
      }
   }

   GJVsave( gp );

   return true;
}
bool LoggerContestLog::exportADIF(QSharedPointer<QFile> expfd )
{
   // ADIF format file entry
   // OP header
   // and EOH
   QString header = tr("Exported by Minos VHF logging system Version %1 %2").arg(STRINGVERSION, PRERELEASETYPE) + "\n\n";

   header += tr( "From file %1" ).arg(cfileName) + "\n\n";

   header += "<EOH>\n";

   QByteArray bh = header.toUtf8();

   qint64 ret = expfd->write(bh);
   if (  ret != bh.size() )
   {
      MinosParameters::getMinosParameters() ->mshowMessage( tr("bad reply from write!") );
   }

   for ( auto const &i: QASCONST(ctList ))
   {
      QSharedPointer<BaseContact> lct = i.wt;
      QString l = lct ->getADIFLine();
      if ( l.size() )
      {
         QByteArray bl = l.toUtf8();
         qint64 ret = expfd->write(bl);
         if (  ret != bl.size() )
         {
            MinosParameters::getMinosParameters() ->mshowMessage( tr("bad reply from write!") );
         }
         const char *EOR = "<EOR>\n";
         ret = expfd->write(EOR, strlen( EOR ));
         if ( ret != static_cast< int >(strlen( EOR )) )
         {
            MinosParameters::getMinosParameters() ->mshowMessage( tr("bad reply from write!") );
         }
      }
   }

   return true;
}
bool LoggerContestLog::exportREG1TEST(QSharedPointer<QFile>expfd, bool noSerials )
{
   // First test validity. Reg1test dictates in particular

    /*
   // band

   QString cb = currentBand.getValue().trimmed();
   BandList &blist = BandList::getBandList();
   BandInfo bi;
   bool bandOK = blist.findBand(cb, bi);
   if (bandOK)
   {
      cb = bi.reg1test;
   }

   if ( !bandOK )
   {
      // put up a band chooser dialog
      TMinosBandChooser mshowMessage( LogContainer );

      BandList &blist = BandList::getBandList();
      for (unsigned int i = 0; i < blist.bandList.size(); i++)
      {
         if (blist.bandList[ i ].reg1test.size())  // only put up real reg1test options
         {
            mshowMessage.BandCombo->Items->Add( blist.bandList[ i ].reg1test.c_str() );
         }
      }

      mshowMessage.BandCombo->ItemIndex = 0;

      QString capt = ( boost::format( "The band description chosen (%s) is invalid for Reg1Test"
                                          " (.EDI) entry. Please choose a valid band description." )
                           % band.getValue() ).str();
      mshowMessage.ScreedLabel->Caption = capt.c_str();

      mshowMessage.ShowModal();

      band.setValue( mshowMessage.BandCombo->Text.c_str() );
   }
      */


   reg1test rtest ( this );
   int rep = rtest.exportTest( expfd, noSerials );
   return rep;
}
bool LoggerContestLog::exportCabrillo(QSharedPointer<QFile> expfd)
{
   Cabrillo rtest( this );
   int rep = rtest.exportTest( expfd );
   return rep;
}
bool LoggerContestLog::exportPrintFile(QSharedPointer<QFile> expfd )
{
   PrintFile rtest( this );
   int rep = rtest.exportTest( expfd );
   return rep;
}
bool LoggerContestLog::exportMinos( QSharedPointer<QFile> expfd )
{
   int mind = 1;
   int maxd = maxSerial;
   if ( !enquireDialog(   /*Owner*/nullptr,tr( "Please give first serial to be written"), mind ) )
      return false;
   if ( !enquireDialog(   /*Owner*/nullptr, tr("Please give last serial to be written"), maxd ) )
      return false;

   int mindump = qMin( mind, maxd );
   int maxdump = qMax( mind, maxd );

   QString temp = tr( "Writing all contacts between serials %1 and %2 inclusive" ).arg(mindump).arg(maxdump );
   if ( !MinosParameters::getMinosParameters() ->yesNoMessage( nullptr, temp ) )
      return false;

   MinosTestExport mtest( this );
   int rep = mtest.exportTest( expfd, mindump, maxdump );
   return ( rep > 0 );
}

static QString kmloutput ( Location *outgrid )
{
   QString qkml = QString("%1,%2").arg(raddeg ( outgrid->easting),3,'f',8 ).arg(raddeg ( outgrid->northing),3,'f',8 );
   return qkml;

}

bool LoggerContestLog::exportKML(QSharedPointer<QFile> expfd )
{
   typedef QMap <QString, QSharedPointer<BaseContact>> cmap; // map by call
   typedef QMap <QString, cmap> smap;       // map by prefix
   smap countries;

   for ( auto const &i: QASCONST(ctList ))
   {
      QSharedPointer<BaseContact> ct = i.wt;
      if ( ct->ctryMult )
      {
         ( countries[ ct->ctryMult->getBasePrefix() ] ) [ ct->cs.getFullCall() ] = ct;
      }
      else
         if ( ct->QSOValid )
         {
            ( countries[ "unknown" ] ) [ ct->cs.getFullCall() ] = ct;
         }
         else
         {
            // contact is invalid - don't display it
         }
   }

   QStringList kml;

   kml.append( "<kml xmlns=\"http://earth.google.com/kml/2.0\">" );
   kml.append( "<Document><visibility>0</visibility><open>1</open>" );
   kml.append( "<Folder><name><![CDATA[" + name.getValue() + " " + mycall.getFullCall() + "]]></name><visibility>0</visibility><open>1</open>" );



   kml.append( "<Style id=\"normalState\">");
   kml.append( "<IconStyle>");
   kml.append( "<scale>1.0</scale>");
   kml.append( "<Icon>");
   kml.append( "<href>http://maps.google.com/mapfiles/kml/paddle/ylw-blank.png</href>");
   kml.append( "</Icon>");
   kml.append( "<scale>0.75</scale>");
   kml.append( "</IconStyle>");
   kml.append( "<LabelStyle>");
   kml.append( "<scale>0</scale>");
   kml.append( "</LabelStyle>");
   kml.append( "</Style>");
   kml.append( "<Style id=\"highlightState\">");
   kml.append( "<IconStyle>");
   kml.append( "<Icon>");
   kml.append( "<href>http://maps.google.com/mapfiles/kml/paddle/ylw-stars.png</href>");
   kml.append( "</Icon>");
   kml.append( "<scale>1.0</scale>");
   kml.append( "</IconStyle>");
   kml.append( "<LabelStyle>");
   kml.append( "<scale>1.0</scale>");
   kml.append( "</LabelStyle>");
   kml.append( "</Style>");
   kml.append( "<StyleMap id=\"styleMapGJV\">");
   kml.append( "<Pair>");
   kml.append( "<key>normal</key>");
   kml.append( "<styleUrl>#normalState</styleUrl>");
   kml.append( "</Pair>");
   kml.append( "<Pair>");
   kml.append( "<key>highlight</key>");
   kml.append( "<styleUrl>#highlightState</styleUrl>");
   kml.append( "</Pair>");
   kml.append( "</StyleMap>");



   for ( smap::iterator s = countries.begin(); s != countries.end(); s++ )
   {
      kml.append( "<Folder><name><![CDATA[" + s.key() + "]]></name><open>0</open><visibility>0</visibility>"  );
      for ( cmap::iterator e = s.value().begin(); e != s.value().end(); e++ )
      {
         Location l1;
         Location l2;
         QSharedPointer<BaseContact> ct = e.value();


         char inputbuff[ 101 ];
         strncpy( inputbuff, ct->loc.getLoc().toUtf8().data(), 100 );
         l1.gridstyle = LOC;
         l1.datastring = inputbuff;

         char outputbuff[ 100 ];
         l2.datastring = outputbuff;
         l2.gridstyle = GEO;

         if ( transform( &l1, &l2 ) == GRIDOK )
         {
            kml.append( "<Placemark><visibility>0</visibility>" );
            kml.append("<styleUrl>#styleMapGJV</styleUrl>");
            kml.append( "<description><![CDATA[" + ct->cs.getFullCall() + " " + ct->loc.getLoc() + "]]></description>"  );
            kml.append( "<name><![CDATA[" + ct->cs.getFullCall() + "]]></name>"  );
            kml.append( "<Point><coordinates>" + kmloutput( &l2 ) + ",0</coordinates></Point>"  );
            kml.append( "</Placemark>" );
         }
      }
      kml.append( "</Folder>" );
   }

   kml.append( "</Folder>" );
   kml.append( "</Document>" );
   kml.append( "</kml>" );


   QTextStream ts( expfd.data() );
   ts << kml.join( "\n" );
   ts << "\n";

   return true;

}
//============================================================
bool LoggerContestLog::importLOG(QSharedPointer<QFile> hLogFile )
{
    trace(QString("Importing LOG log %1").arg(hLogFile->fileName()));

    QStringList ls;
    QTextStream in( hLogFile.data() );

    while ( !in.atEnd() )
    {
        QString line = in.readLine(255);
        ls.append(line);
    }

   // Import from LOG format
   // Needs modification for "new" log format

   locValid = false;
   //   save();
   // read remainder of lines; write each line as we go

   long next_block = 1;

   bool started = false;

   //QSharedPointer<BaseContact> ct;
   int lineNo = -1;
   while ( ++lineNo < ls.count() )
   {
      // skip new format header information, until we can cope
      // properly with it
      QString stemp = trimr( ls[ lineNo ] );
      if ( stemp.length() == 0 || ( !started && !stemp[0].isDigit() ) )
      {
         // GJV.LOG

         /*
         Contest Title                        : VHFNFD
         Band in MHz                          : 432
         Section Entered                      : Multi Operator
         Name of Entrant/Club/Contest Group   : Bracknell Amateur Radio Club
         Band in MHz                          : %band%
         Section Entered                      : %section%
         Callsign Used                        : %call%
         QRH Locator Sent                     : %locator%
         Location as sent(District or QTH)    : %QTH%
         Brief Details of Transmitter Used    : %Transmitter%
         Brief Details of Receiver Used       : %Receiver%
         Power Output (Watts)                 : %power%
         Antenna Type                         : %Antenna%
         Antenna Height AGL(metres)           : %AGL%
         QTH Height ASL(metres)               : %ASL%

         Difficult ones:
         Name and Callsign of all Operators   :
         G0GJV  G4AUC  G0FCT  G0HVS
         Name, Callsign, Telephone Number, Address for Correspondence :
                                     : M. J. Goodey
                                     : G0GJV
                                     : 62, Rose Hill
                                     : Binfield
                                     : Berkshire
                                     : RG42 5LG
                               Phone : 01344 428614
                               Email : mjgoodey@compuserve.com

         */
         int spos = stemp.indexOf( ":" );
         if ( spos != -1 )
         {
            QString text = stemp.mid( spos + 1, stemp.size() ).trimmed();

            if ( stemp.toUpper().indexOf( "CONTEST TITLE" ) == 0 )
            {
               name.setValue( text );
            }
            else
               if (  stemp.toUpper().indexOf( "BAND IN MHZ" ) == 0 )
               {
                  contestBands.setValue( text );
                  currentBand.setValue(text);
               }
               else
                  if ( stemp.toUpper().indexOf( "SECTION ENTERED" ) == 0 )
                  {
                     entSect.setValue( text );
                  }
                  else
                     if ( stemp.toUpper().indexOf( "NAME OF ENTRANT/CLUB/CONTEST GROUP" ) == 0 )
                     {
                        entrant.setValue( text );
                     }
                     else
                        if ( stemp.toUpper().indexOf( "CALLSIGN USED" ) == 0 )
                        {
                           // we need to tail clip the callsign - or we end up with things like
                           // M1DUD/QRP        CLASS : SO  /   50MHZ FIXED
                           int spos = text.indexOf( " " );
                           if ( spos != -1 )
                           {
                              text = text.left(spos );
                           }
                           mycall.setFullCall( text);

                        }
                        else
                           if ( stemp.toUpper().indexOf( "QRH LOCATOR SENT" ) == 0 ||
                                stemp.toUpper().indexOf( "QTH LOCATOR SENT" ) == 0 )
                           {
                              // yes, contestx DOES say QRH!
                              myloc.setLoc( text );
                           }
                           else
                              if ( stemp.toUpper().indexOf( "POWER OUTPUT" ) == 0 )
                              {
                                 power.setValue( text );
                              }
                              else
                                 if ( stemp.toUpper().indexOf( "LOCATION AS SENT" ) == 0 )
                                 {
                                    location.setValue( text );
                                 }
                                 else
                                    if ( stemp.toUpper().indexOf( "ANTENNA TYPE" ) == 0 )
                                    {
                                       entAnt.setValue( text );
                                    }
                                    else
                                       if ( stemp.toUpper().indexOf( "ANTENNA HEIGHT" ) == 0 )
                                       {
                                          entAGL.setValue( text );
                                       }
                                       else
                                          if ( stemp.toUpper().indexOf( "QTH HEIGHT" ) == 0 )
                                          {
											 entASL.setValue( text );
                                          }
                                          else
                                             if ( stemp.toUpper().indexOf( "BRIEF DETAILS OF TRANSMITTER USED" ) == 0 )
                                             {
                                                entTx.setValue( text );
                                             }
                                             else
                                                if ( stemp.toUpper().indexOf( "BRIEF DETAILS OF RECEIVER USED" ) == 0 )
                                                {
                                                   entRx.setValue( text );
                                                }
         }
         continue;
      }
      started = true;

      QSharedPointer<BaseContact> bct;
      makeContact( false, bct );
      bct->setLogSequence( 0 );

      stemp += QString( 200, ' ' );   // make sure there is plenty more...

      QString &lbuff = stemp;

      // parse contact line in

      QString temp;
      strcpysp( temp, lbuff, 6 );
      bct->timeOff.setDate( temp, DTGLOG );
      strcpysp( temp, lbuff.mid(7), 4 );
      bct->timeOff.setTime( temp, DTGLOG );
      bct->timeOn = bct->timeOff;
      strcpysp( temp, lbuff.mid(21), 15 );
      bct->cs.setFullCall( temp );
      strcpysp( temp, lbuff.mid( 37 ), 3 );
      bct->reps.setValue( temp );
      strcpysp( temp, lbuff.mid( 41 ), 4 );
      bct->serials.setValue( temp );

      int maxct = bct->serials.getValue().toInt();
      if ( maxct > maxSerial )
         maxSerial = maxct;

      strcpysp( temp, lbuff.mid( 46 ), 3 );
      bct->repr.setValue( temp );
      strcpysp( temp, lbuff.mid( 51 ), 4 );
      bct->serialr.setValue( temp );

      // here is the score field
      strcpysp( temp, lbuff.mid( 59 ), 5 );
      if ( toInt( temp ) == 0 )
         bct->contactFlags.setValue( NON_SCORING );

      strcpysp( temp, lbuff.mid( 65 ), 6 );
      bct->op1.setValue( temp );

      strcpysp( temp, lbuff.mid( 72 ), 6 );
      bct->loc.setLoc( temp );

      //ct->comments = "";

      QString extra;
      QString comments;

      strcpysp( extra, lbuff.mid( 81 ), 2 );          // 81 is district code
      if ( extra.length() >= 2 && ( extra[ 0 ] != ' ' && extra[ 1 ] != ' ' ) )
      {
         // we always attempt to import the district mult field
         strcpysp( comments, lbuff.mid( 93 ), COMMENTLENGTH );
         bct->comments.setValue( comments );

      }
      else
      {
         strcpysp( extra, lbuff.mid( 93 ), EXTRALENGTH );
      }
      bct->extraText.setValue( extra );

      // save contact

      // we have to have logSequence set before we insert - or it will cause
      // duplicates

      next_block++ ;
      bct->setLogSequence( static_cast<unsigned long>(next_block) << 16 );

      addToContestList(bct);
   }
   return true;
}
//====================================================================
bool LoggerContestLog::importAdif(QSharedPointer<QFile> adifContestFile )
{
    trace(QString("Importing ADIF log %1").arg(adifContestFile->fileName()));

   return ADIFImport::doImportADIFLog( this, adifContestFile );
}
//====================================================================
bool LoggerContestLog::importReg1Test(QSharedPointer<QFile> r1ContestFile )
{
    trace(QString("Importing Reg1Test log %1").arg(r1ContestFile->fileName()));
   // in AdjSQL we do this as a semi-colon separated format, not as something special
   return reg1test::doImportReg1test( this, r1ContestFile );
}
//====================================================================
void LoggerContestLog::processMinosStanza( const QString &methodName, MinosTestImport * const mt )
{
   BaseContestLog::processMinosStanza( methodName, mt );

   if ( methodName == "MinosLogContest" )
   {
      mt->getStructArgMemberValue( "section", entSect );
      mt->getStructArgMemberValue( "sectionList", sectionList );
      mt->getStructArgMemberValue( "ScreenLayout", screenLayout);
      mt->getStructArgMemberValue( "currentFKeySet", currentFKeySet);
      mt->getStructArgMemberValue( "WatchADIFFile", watchedADIFFile);
      mt->getStructArgMemberValue( "WatchADIFLo", watchedADIFLastOffset);
   }
   else
      if ( methodName == "MinosLogMode" )
      {}
      else
         if ( methodName == "MinosLogQTH" )
         {
            mt->getStructArgMemberValue( "stationQTH1", sqth1 );
            mt->getStructArgMemberValue( "stationQTH2", sqth2 );
			mt->getStructArgMemberValue( "ASL", entASL );
		 }
		 else
			if ( methodName == "MinosLogEntry" )
			{
			   mt->getStructArgMemberValue( "entrant", entrant );
			   mt->getStructArgMemberValue( "myName", entName );
			   mt->getStructArgMemberValue( "myCall", entCall );
			   mt->getStructArgMemberValue( "myAddress1", entAddr1 );
			   mt->getStructArgMemberValue( "myAddress2", entAddr2 );
			   mt->getStructArgMemberValue( "myCity", entCity );
			   mt->getStructArgMemberValue( "myCountry", entCountry );
			   mt->getStructArgMemberValue( "myPostCode", entPostCode );
			   mt->getStructArgMemberValue( "myPhone", entPhone );
			   mt->getStructArgMemberValue( "myEmail", entEMail );
			}
			else
			   if ( methodName == "MinosLogStation" )
			   {
				  mt->getStructArgMemberValue( "transmitter", entTx );
				  mt->getStructArgMemberValue( "receiver", entRx );
				  mt->getStructArgMemberValue( "antenna", entAnt );
				  mt->getStructArgMemberValue( "AGL", entAGL );
				  mt->getStructArgMemberValue( "offset", bearingOffset);
                  QString s;
                  if (mt->getStructArgMemberValue("radioName", s))
                    radioName.setValue(PubSubName(s));
                  if (mt->getStructArgMemberValue("rotatorName", s))
                    antennaName.setValue(PubSubName(s));
               }
			   else
				  if ( methodName == "MinosLogOperators" )
				  {
					 mt->getStructArgMemberValue( "ops1", ops1 );
					 mt->getStructArgMemberValue( "ops2", ops2 );
					 mt->getStructArgMemberValue( "currentOp1", currentOp1 );
                     oplist.insert(currentOp1.getValue(), currentOp1.getValue());
                     mt->getStructArgMemberValue( "currentOp2", currentOp2 );
                     oplist.insert(currentOp2.getValue(), currentOp2.getValue());
                  }
                  else
                     if ( methodName == "MinosLogCurrent" )
                     {
                        mt->getStructArgMemberValue( "condx1", entCondx1 );
                        mt->getStructArgMemberValue( "condx2", entCondx2 );
                        mt->getStructArgMemberValue( "condx3", entCondx3 );
                        mt->getStructArgMemberValue( "condx4", entCondx4 );
                     }
                     else
                        if ( methodName == "MinosLogBundles" )
                        {
                           mt->getStructArgMemberValue( "VHFContestName", VHFContestName );
                           mt->getStructArgMemberValue( "entryBundle", entryBundleName );
                           mt->getStructArgMemberValue( "QTHBundle", QTHBundleName );
                           mt->getStructArgMemberValue( "stationBundle", stationBundleName );
                        }
                        else

                           if (methodName == "MinosRigMemory")
                           {
                               memoryData::memData mem;
                               int memno;
                               mt->getStructArgMemberValue( "memno", memno);
                               mt->getStructArgMemberValue( "callsign", mem.callsign);
                               QString temp;
                               mt->getStructArgMemberValue( "freq", temp);
                               mem.freq = Frequency(temp);


                               mt->getStructArgMemberValue( "mode", mem.mode);
                               mt->getStructArgMemberValue( "locator", mem.locator);
                               mt->getStructArgMemberValue( "bearing", mem.bearing);
                               mt->getStructArgMemberValue( "time", mem.time);
                               mt->getStructArgMemberValue( "worked", mem.worked);
                               saveInitialRigMemory(memno, mem);
                           }
                           else
                               if (methodName == "MinosRunMemory")
                               {
                                   memoryData::memData mem;
                                   int memno;
                                   mt->getStructArgMemberValue( "memno", memno);
                                   QString temp;
                                   mt->getStructArgMemberValue( "freq", temp);
                                   mem.freq = Frequency(temp);
                                   QString band;
                                   mt->getStructArgMemberValue( "band", band);
                                   if (band.isEmpty())
                                   {
                                       band = BandList::getBand(mem.freq);
                                   }

                                   mt->getStructArgMemberValue( "mode", mem.mode);

                                   if (!band.isEmpty())
                                   {
                                       mem.band = band;
                                       saveInitialRunMemory(memno, mem);
                                   }
                               }
                               else if (methodName == "MinosClusterFilter")
                               {
                                       clusterFilterSettingsExist = true;
                                       ClusterClientFilterSettings ccfs;
                                       QVector<QSharedPointer<BandInfo> > bands;
                                       BandList::getBandList().loadAllBands(bands, false);
                                       ccfs.initFilterSettings(bands);
                                       mt->getStructArgMemberValue("callsignList", ccfs.callsignFilterList);
                                       mt->getStructArgMemberValue("locatorList", ccfs.locatorFilterList);
                                       bool filterFlag = false;
                                       int distance = 0;

                                       for(const auto &b: QASCONST(bands))
                                       {
                                            QString bandIni = "bandFilter" + b->normalisedName();
                                            QString distIni = "distanceFilter" + b->normalisedName();
                                            QString igdistIni = "ignoreDistanceFlag" + b->normalisedName();
                                            QString igedistIni = "ignoreEmptyDistanceFlag" + b->normalisedName();

                                            mt->getStructArgMemberValue(bandIni, filterFlag);
                                            ccfs.setBandFilter(b->name(), filterFlag);
                                            mt->getStructArgMemberValue(distIni, distance);
                                            ccfs.setDistanceFilter( b->name(), distance);
                                            mt->getStructArgMemberValue(igdistIni, filterFlag);
                                            ccfs.setIgnoreDistanceFlag( b->name(), filterFlag);
                                            mt->getStructArgMemberValue(igedistIni, filterFlag);
                                            ccfs.setIgnoreEmptyDistanceFlag(b->name(), filterFlag );
                                       }

                                       mt->getStructArgMemberValue("modeFilterCW", filterFlag);
                                       ccfs.setModeFilter(hamlibData::CW, filterFlag);

                                       mt->getStructArgMemberValue("modeFilterLSBMODE", filterFlag);
                                       ccfs.setModeFilter(hamlibData::LSB, filterFlag);

                                       mt->getStructArgMemberValue("modeFilterUSBMODE", filterFlag);
                                       ccfs.setModeFilter(hamlibData::USB, filterFlag);

                                       mt->getStructArgMemberValue("modeFilterFMMODE", filterFlag);
                                       ccfs.setModeFilter(hamlibData::FM, filterFlag);

                                       mt->getStructArgMemberValue("modeFilterRTTYMODE", filterFlag);
                                       ccfs.setModeFilter(hamlibData::RTTY, filterFlag);

                                       mt->getStructArgMemberValue("modeFilterPSK31MODE", filterFlag);
                                       ccfs.setModeFilter("PSK31", filterFlag);

                                       mt->getStructArgMemberValue("modeFilterFT4MODE", filterFlag);
                                       ccfs.setModeFilter("FT4", filterFlag);

                                       mt->getStructArgMemberValue("modeFilterFT8MODE", filterFlag);
                                       ccfs.setModeFilter("FT8", filterFlag);

                                       mt->getStructArgMemberValue("modeFilterMSK144MODE", filterFlag);
                                       ccfs.setModeFilter("MSK144", filterFlag);

                                       mt->getStructArgMemberValue("modeFilterJT65MODE", filterFlag);
                                       ccfs.setModeFilter("JT65", filterFlag);

                                       mt->getStructArgMemberValue("modeFilterNONEMODE", filterFlag);
                                       ccfs.setModeFilter("NONE", filterFlag);

                                       saveInitialClusterFilter(ccfs);

                               }
                               else if (methodName == "MinosBandmapFilter")
                               {

                                       bandmapFilterSettingsExist = true;
                                       BandmapClientFilterSettings bcfs;
                                       bool filterFlag = false;
                                       bool filterFlag1 = false;


                                       mt->getStructArgMemberValue("modeFilterCW", filterFlag);
                                       bcfs.setModeFilter("CW", filterFlag);
                                       mt->getStructArgMemberValue("modeFilterLSBMODE", filterFlag);
                                       bcfs.setModeFilter("LSB", filterFlag);
                                       mt->getStructArgMemberValue("modeFilterUSBMODE", filterFlag);
                                       bcfs.setModeFilter("USB", filterFlag);
                                       mt->getStructArgMemberValue("modeFilterFMMODE", filterFlag);
                                       bcfs.setModeFilter("FM", filterFlag);
                                       mt->getStructArgMemberValue("modeFilterRTTYMODE", filterFlag);
                                       bcfs.setModeFilter("RTTY", filterFlag);
                                       mt->getStructArgMemberValue("modeFilterPSK31MODE", filterFlag);
                                       bcfs.setModeFilter("PSK31", filterFlag);
                                       mt->getStructArgMemberValue("modeFilterFT4MODE", filterFlag);
                                       bcfs.setModeFilter("FT4", filterFlag);
                                       mt->getStructArgMemberValue("modeFilterFT8MODE", filterFlag);
                                       bcfs.setModeFilter("FT8", filterFlag);
                                       mt->getStructArgMemberValue("modeFilterMSK144MODE", filterFlag);
                                       bcfs.setModeFilter("MSK144", filterFlag);
                                       mt->getStructArgMemberValue("modeFilterJT65MODE", filterFlag);
                                       bcfs.setModeFilter("JT65", filterFlag);
                                       mt->getStructArgMemberValue("modeFilterNONEMODE", filterFlag);
                                       bcfs.setModeFilter("NONE", filterFlag);
                                       int distance = 0;
                                       mt->getStructArgMemberValue("distanceFilter", distance);
                                       bcfs.setDistanceFilter(distance);
                                       mt->getStructArgMemberValue("ignoreDistanceFlag", filterFlag);
                                       bcfs.setIgnoreDistanceFlag(filterFlag);
                                       mt->getStructArgMemberValue("ignoreEmptyDistanceFlag", filterFlag1);
                                       bcfs.setIgnoreEmptyDistanceFlag(filterFlag1);


                                       saveInitialBandmapFilter(bcfs);



                               }
                               else
                                   if ( methodName == "MinosLogComment" )
                                   {
                                      // should have been dealt with in BaseContest
                                   }
                                   else
                                      if ( methodName == "MinosLogQSO" )
                                      {
                                         // should have been dealt with in BaseContest
                                      }
                                      else
                                         if (methodName == "MinosStackParams")
                                         {
                                             mt->getStructArgMemberValue( "sp1", statsPeriod1);
                                             mt->getStructArgMemberValue( "sp2", statsPeriod2);
                                             mt->getStructArgMemberValue( "eu", showContinentEU);
                                             mt->getStructArgMemberValue( "as", showContinentAS);
                                             mt->getStructArgMemberValue( "af", showContinentAF);
                                             mt->getStructArgMemberValue( "oc", showContinentOC);
                                             mt->getStructArgMemberValue( "sa", showContinentSA);
                                             mt->getStructArgMemberValue( "na", showContinentNA);
                                             mt->getStructArgMemberValue( "sw", showWorkedCountries);
                                             mt->getStructArgMemberValue( "su", showUnworkedCountries);
                                             mt->getStructArgMemberValue( "dw", showWorkedDistricts);
                                             mt->getStructArgMemberValue( "du", showUnworkedDistricts);
                                             for (int i = 0; i < STACKITEMS; i++)
                                             {
                                                  mt->getStructArgMemberValue( "sitem" + QString::number(i), currentStackItems[i]);
                                             }
                                             currentStackItemsValid = true;
                                         }
                                        else
                                           if (methodName == "MinosQSOMap")
                                           {
                                               mt->getStructArgMemberValue( "zoom", zoomLevel);
                                               mt->getStructArgMemberValue( "lat", centreLat);
                                               mt->getStructArgMemberValue( "lon", centreLon);
                                           }
}
//====================================================================
void LoggerContestLog::setStanza(unsigned int stanza, int stanzaStart, int stanzaEnd )
{
   StanzaPos s;
   s.stanza = stanza;
   s.stanzaStart = stanzaStart;
   s.stanzaEnd = stanzaEnd;
   stanzaLocations.push_back( s );
}
//====================================================================
bool LoggerContestLog::getStanza( unsigned int stanza, QString &stanzaData )
{
   if ( static_cast<int>(stanza) > stanzaLocations.size() )
   {
      return false;
   }
   StanzaPos *s = &stanzaLocations[ static_cast<int>(stanza) - 1 ];
   if ( s->stanza != stanza )
   {
      return false;
   }
   // read the stanza from the log file - which we will have to open for read

   QFile contestFile(cfileName);
   bool ret = contestFile.open(QIODevice::ReadOnly);
   if ( !ret )
   {
      QString emess = tr("Failed to open Contest file for monitoring %1 : %2").arg(cfileName, contestFile.errorString());
      MinosParameters::getMinosParameters() ->mshowMessage( emess );
      return false;
   }
   ret = contestFile.seek( s->stanzaStart );
   if ( !ret )
   {
      MinosParameters::getMinosParameters() ->mshowMessage( tr("(write) seek failed!") );
   }
   QByteArray buffer = contestFile.read( s->stanzaEnd - s->stanzaStart );

   bool readOK = true;
   if ( buffer.size() > 0 )
    {
        stanzaData = QString(buffer);
        int epos = stanzaData.indexOf( "</iq>" );     // trim the excess - if there is any
        if ( epos != -1 )
        {
            stanzaData = stanzaData.left(epos + 5 );
        }
    }
    else
        readOK = false;

   contestFile.close();
   return readOK;
}
//====================================================================

