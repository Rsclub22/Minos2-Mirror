/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifndef LoggerContestH
#define LoggerContestH 
#include "base_pch.h"
#include "cutils.h"
//----------------------------------------------------------------------------
#include "profiles.h"
#include "rigmemcommondata.h"
#include "clustercommon.h"
#include "PubSubValue.h"
#include "contest.h"

class DisplayContestContact;
struct StanzaPos
{
   unsigned int stanza;
   long stanzaStart;
};
class LoggerContestLog : public BaseContestLog
{
    Q_OBJECT
      friend class MinosTestImport;
      friend class ContestDetailsTransferObject;
      // a LoggerContestLog is a sorted collection of ContestContact records, sorted by
      // the logSequence field
   private:

      // These are Logger specific, as they relate to storage
      bool minosFile;
      bool GJVFile;
      int logCount;  // Only used loading GJV files
      bool logFile;
      bool adifFile;
      bool ediFile;
      bool needExport;
      QSharedPointer<QFile> minosContestFile;
      QSharedPointer<QFile> GJVcontestFile;
      QSharedPointer<QFile> logContestFile;
      QSharedPointer<QFile> adifContestFile;
      QSharedPointer<QFile> ediContestFile;
      bool importLOG( QSharedPointer<QFile>  );
      bool importAdif( QSharedPointer<QFile>  );
      bool importReg1Test( QSharedPointer<QFile> adifContestFile );
      QVector<StanzaPos> stanzaLocations;

protected:
      // minos file
      virtual bool minosSaveFile( bool newfile ) override;
   public:
      bool isMinosFile()
      {
         return minosFile;
      }
      bool isGJVFile()
      {
         return GJVFile;
      }
      bool isLogFile()
      {
         return logFile;
      }
      bool needsExport()
      {
         return needExport;
      }

      virtual void processMinosStanza( const QString &methodName, MinosTestImport * const mt ) override;
      virtual void setStanza( unsigned int stanza, int stanzaStart ) override;
      virtual bool getStanza(unsigned int Stanza, QString &stanzaData ) override;

      // The contest details

      // Bundles are specific to the off disk logs, not the monitored logs

      SettingsBundle entryBundle;      // individual LoggerContestLog
      MinosStringItem<QString> entryBundleName;
      SettingsBundle QTHBundle;        // individual QTH
      MinosStringItem<QString> QTHBundleName;
      SettingsBundle stationBundle;    // individual station
      MinosStringItem<QString> stationBundleName;

      MinosItem<PubSubName> radioName;
      MinosItem<PubSubName> antennaName;

      MinosStringItem<QString> VHFContestName;

      // Most of this info is bundle related, and not relevant when monitored (?)

      MinosStringItem<QString> entrant;
      MinosStringItem<QString> entSect;
      MinosStringItem<QString> sqth1;
      MinosStringItem<QString> sqth2;
      MinosStringItem<QString> entTx;
      MinosStringItem<QString> entRx;
      MinosStringItem<QString> entAnt;
      MinosStringItem<QString> entAGL;
      MinosStringItem<QString> entASL;
      MinosStringItem<QString> ops1;        // list of ops for contest
      MinosStringItem<QString> ops2;        // list of ops for contest
      MinosStringItem<QString> entCondx1;
      MinosStringItem<QString> entCondx2;
      MinosStringItem<QString> entCondx3;
      MinosStringItem<QString> entCondx4;
      MinosStringItem<QString> entName;
      MinosStringItem<QString> entCall;
      MinosStringItem<QString> entAddr1;
      MinosStringItem<QString> entAddr2;
      MinosStringItem<QString> entPostCode;
      MinosStringItem<QString> entCity;
      MinosStringItem<QString> entCountry;
      MinosStringItem<QString> entPhone;
      MinosStringItem<QString> entEMail;
      MinosStringItem<QString> sectionList; // needed to be carried forward

      QVector<MinosItem<memoryData::memData> > runMemories;
      QVector<MinosItem<memoryData::memData> > rigMemories;

      MinosItem<ClusterClientFilterSettings> clusterFilterSettings;
      bool clusterFilterSettingsExist;

      MinosItem<BandmapClientFilterSettings> bandmapFilterSettings;
      bool bandmapFilterSettingsExist;

      QDateTime lastSpotTabTime;
      QDateTime lastCallsignTabTime;
      QDateTime lastLocatorTabTime;

      MinosStringItem<QString> screenLayout;

      MinosItem<int> statsPeriod1;
      MinosItem<int> statsPeriod2;
      MinosItem<bool> showContinentEU;
      MinosItem<bool> showContinentAS;
      MinosItem<bool> showContinentAF;
      MinosItem<bool> showContinentOC;
      MinosItem<bool> showContinentSA;
      MinosItem<bool> showContinentNA;
      MinosItem<bool> showWorked;
      MinosItem<bool> showUnworked;
      MinosStringItem<QString> currentStackItems[STACKITEMS];

      void saveRunMemory(int memno, const memoryData::memData &mem);
      void saveRigMemory(int memno, const memoryData::memData &mem);
      void saveInitialRunMemory(int memno, const memoryData::memData &mem);
      void saveInitialRigMemory(int memno, const memoryData::memData &mem);
      memoryData::memData getRigMemoryData(int memoryNumber);

      void saveClusterFilter(const ClusterClientFilterSettings &ccfs);
      void saveInitialClusterFilter(const ClusterClientFilterSettings &ccfs);
      ClusterClientFilterSettings getClusterFilter();

      void saveBandmapFilter(const BandmapClientFilterSettings &bcfs);
      void saveInitialBandmapFilter(const BandmapClientFilterSettings &bcfs);
      BandmapClientFilterSettings getBandmapFilter();

      // dirty info is only relevant when it is being editted
      // but needs to stay with the data

      virtual void clearDirty() override;
      virtual void setDirty() override;
      // end of contest details

      void setINIDetails();

      // startup/closedown

      LoggerContestLog( );
      void initialiseINI();
      bool initialise( int slotno );
      bool initialise(const QString &, bool, int slotno );
      ~LoggerContestLog() override;


      // common file stuff
      bool commonSave( bool newfile ) override;
      void closeFile( ) override;

      // minos save
      virtual bool minosSaveContestContact(const QSharedPointer<BaseContact> lct ) override;

      // GJV file manipulation
      qint64 readBlock( int bno );
      qint64 writeBlock(QSharedPointer<QFile> fd, int bno );
      bool GJVsave( GJVParams & );
      virtual bool GJVload( ) override;
      bool GJVloadContacts( );

      // Import/export etc

      bool export_contest( QSharedPointer<QFile>, ExportType, bool noSerials );
      bool exportGJV(QSharedPointer<QFile> );
      void procUnknown( QSharedPointer<BaseContact> cct, writer &wr );
      bool exportADIF( QSharedPointer<QFile> expfd );
      bool exportKML( QSharedPointer<QFile> expfd );
      bool exportREG1TEST(QSharedPointer<QFile> expfd, bool noSerials );
      bool exportPrintFile( QSharedPointer<QFile> expfd );
      bool exportMinos(QSharedPointer<QFile> expfd );

      virtual void makeContact( bool time_now, QSharedPointer<BaseContact>& ) override;
      QSharedPointer<BaseContact> addContact(int newctno, unsigned short extra_flags, bool save_new, bool catchup , QString mode, QString mgmSubmode, dtg ctTime);
      QSharedPointer<BaseContact> addContactBetween(QSharedPointer<BaseContact> prior, QSharedPointer<BaseContact> next , dtg ctTime);
      void removeContact(QSharedPointer<BaseContact> );



};

#endif
