/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
#include "logger_pch.h"

#include "MLogFile.h"
#include "INIFile.h"

//=============================================================================
BundleFile::BundleFile( PROFILES p )
{
    switch ( p )
    {
    //       ProfileEntry(int id, const  char *name, const char *def, const char *dispname, const char *hint, bool RO );
    case epLOGGERPROFILE:

        entries.push_back( QSharedPointer<ProfileEntry> (new ProfileEntry( elpListDirectory, "List Directory", "./Lists", "", "Default archive list directory", false ) ) );
        entries.push_back( QSharedPointer<ProfileEntry> (new ProfileEntry( elpLogDirectory, "Log Directory", "./Logs", "", "Default logs directory", false ) ) );

        entries.push_back( QSharedPointer<ProfileEntry> (new ProfileEntry( elpEntryFile, "EntryFile", "./Configuration/Entry.ini", "Entry settings file", "File containing entry settings", false ) ) );
        entries.push_back( QSharedPointer<ProfileEntry> (new ProfileEntry( elpStationFile, "StationFile", "./Configuration/Station.ini", "Station settings file", "File containing station settings", false ) ) );
        entries.push_back( QSharedPointer<ProfileEntry> (new ProfileEntry( elpQTHFile, "QTHFile", "./Configuration/QTH.ini", "QTH settings file", "File containing QTH settings", false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( elpLocsFile, "LocsFile", "./Configuration/LocSquares.ini", "Country locators file", "File containing valid locators for countries", false ) ));

        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( elpDisplayFile, "DisplayFile", "./Configuration/Display.ini", "Display settings file", "File containing saved display settings", false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( elpDisplaySection, "DisplaySection", "Default", "Display file section", "Section to use in display file", false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( elpOperatorFile, "OperatorFile", "./Configuration/Operator.ini", "Operators file", "File containing operators", false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( elpOperatorSection, "OperatorSection", "Default", "Operators file section", "section to use in operators file", false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( elpPreloadFile, "PreloadFile", "./Configuration/LogsPreload.ini", "Log preload file", "File containing log pre-loads", false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( elpPreloadSection, "PreloadSection", "Default", "Preload contests default section", "Section to use in preload file", false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( elpListsFile, "ListsFile", "./Configuration/ListPreload.ini", "List preload file", "File containing list pre-loads", false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( elpListsSection, "ListsSection", "Default", "Preload Lists file section", "Section to use in lists preload file", false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( elpAppFile, "AppsFile", "./Configuration/Apps.ini", "Apps File", "File containing Apps settings", false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( elpAutoFill, "AutoFill", false, "Auto Fill signal report", "Auto Fill signal report on return", false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( elpAuxWindows, "Auxiliary Windows", 1, "", "Number of Auxiliary Windows", false ) ) );

        break;
    case epPRELOADPROFILE:
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( eppCurrent, "CurrentLog", 0, 0, "hint", false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( eppDefSession, "DefaultSessionName", "Default Session", "Default Session", "hint", false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( eppSession, "CurrentSession", "Default Session", "Default Session", "hint", false ) ) );
        break;

    case epLISTSPROFILE:
        break;

    case epDISPLAYPROFILE:
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( edpShowContinentEU, "ShowContinentEU", true, "", "hint", false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( edpShowContinentAS, "ShowContinentAS", false, "", "hint", false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( edpShowContinentAF, "ShowContinentAF", false, "", "hint", false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( edpShowContinentOC, "ShowContinentOC", false, "", "hint", false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( edpShowContinentSA, "ShowContinentSA", false, "", "hint", false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( edpShowContinentNA, "ShowContinentNA", false, "", "hint", false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( edpShowWorked, "ShowWorked", true, "", "hint", false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( edpShowUnworked, "ShowUnworked", false, "", "hint", false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( edpStackFrame, "StackFrame", "Clock", "", "hint", false ) ) );

        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( edpNextContactDetailsOnLeft, "ShowNextContactDetailsOnLeft", false, "", "hint", false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( edpShowOperators, "ShowOperators", true, "", "hint", false ) ) );

        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( edpEditor, "Editor", "Notepad.exe", "", "Default editor", false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( edpStatisticsPeriod1, "Statistics Period 1", 10, "", "Statistics Period 1", false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( edpStatisticsPeriod2, "Statistics Period 2", 60, "", "Statistics Period 2", false ) ) );
        break;
    case epENTRYPROFILE:
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( eepCall, "Call", "", "Call Used", "Call sign used", false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( eepEntrant, "Entrant", "", "On Behalf Of (Club)", "Name of club/group", false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( eepMyName, "MyName", "", "My Name", "Name of responsible operator", false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( eepMyCall, "MyCall", "", "My Call", "Callsign of responsible operator", false ) ) );

        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( eepMyAddress1, "MyAddress1", "", "My Address Line 1", "Address line 1 of responsible operator", false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( eepMyAddress2, "MyAddress2", "", "My Address Line 2", "Address line 2 of responsible operator", false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( eepMyCity, "MyCity", "", "My City", "City of responsible operator", false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( eepMyCountry, "MyCountry", "", "My Country", "Country of responsible operator", false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( eepMyPostCode, "MyPostCode", "", "My Postcode", "Post Code of responsible operator", false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( eepMyPhone, "MyPhone", "", "My Phone", "Phone no. of responsible operator", false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( eepMyEmail, "MyEmail", "", "My Email", "eMail address of responsible operator", false ) ) );
        break;
    case epQTHPROFILE:
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( eqpLocator, "Locator", "", "Locator", "Locator", false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( eqpDistrict, "District", "", "District Exchange", "District Exchange", false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( eqpLocation, "Location", "", "Location Exchange", "Descriptive Location Exchange", false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( eqpStationQTH1, "StationQTH1", "", "Station QTH Line 1", "Address line 1/2 of station", false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( eqpStationQTH2, "StationQTH2", "", "Station QTH Line 2", "Address line 2/2 of station", false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( eqpASL, "ASL", 0, "QTH Height ASL (metres)", "QTH height ASL (metres)", false ) ) );
        break;
    case epSTATIONPROFILE:
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( espPower, "Power", 0, "Transmitter Power", "Transmit Power (Watts)", false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( espTransmitter, "Transmitter", "", "Transmit Equipment", "Transmit Equipment", false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( espReceiver, "Receiver", "", "Receive Equipment", "Receive Equipment", false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( espAntenna, "Antenna", "", "Antenna details", "Antenna details", false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( espAGL, "AGL", 0, "Antenna Height AGL (metres)", "Antenna Height AGL (metres)", false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( espOffset, "Bearing Offset", 0, "Antenna Bearing Offset", "Amount to offset antenna bearings", false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( espRadioName, "Radio", "", "Radio in Rig Control", "Radio in Rig Control", false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( espRotatorName, "rotator", "", "Rotator in Rotator Control", "Rotator in Rotator Control", false ) ) );
        break;

    case epAPPPFROFILE:
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( eapBandMap, "Band Map", "BandMap", "Band Map App", "Band Map App", false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( eapRigControl, "Rig Control", "RigControl", "Rig Control App", "Rig Control App", false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( eapRotator, "Rotator", "Rotator", "Rotator App", "Rotator App", false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( eapVoiceKeyer, "Voice Keyer", "Keyer", "Voice Keyer App", "Voice Keyer App", false ) ) );
        break;

    case epLOCSQUARESPROFILE:
    default:
        break;
    }
}
bool BundleFile::openProfile( const QString &fname, const QString &bname )
{
   bundleName = bname;
   iniFile = QSharedPointer<INIFile>( new INIFile( fname ) );

   iniFile->loadINIFile();
   return true;
}
BundleFile::~BundleFile()
{
}
/*static*/
QSharedPointer<BundleFile>BundleFile::bundleFiles[ epMAXPROFILE ]; //  =  {0};
/*static*/
QSharedPointer<BundleFile>BundleFile::getBundleFile( PROFILES p )
{
   if ( !bool( bundleFiles[ p ] ) )
   {
      bundleFiles[ p ] = QSharedPointer<BundleFile>( new BundleFile( p ) );
   }
   return bundleFiles[ p ];
}
QSharedPointer <ProfileEntry> &BundleFile::GetKey( int p )
{
   static QSharedPointer <ProfileEntry> NullEntry(new ProfileEntry( -1, "", 0,"",  "", false ));
   for ( int i = 0; i < entries.size(); i++ )
   {
      if ( entries[ i ]->id == p )
         return entries[ i ];
   }
   return NullEntry;
}
//=============================================================================
SettingsBundle::SettingsBundle():noneBundle("<None>")
{}

SettingsBundle::~SettingsBundle()
{
   try
   {
      closeProfile();
   }
   catch ( ... )
   {}
}
QString SettingsBundle::getBundle()
{
   return bundleFile->getBundle();
}
void SettingsBundle::checkLoaded()
{
    if ( !bundleFile )
    {
       return ;
    }
    // force a reload if necessary
    bundleFile->iniFile->getPrivateProfileInt( "dummy", "dummy",  true );
}
void SettingsBundle::setProfile( QSharedPointer<BundleFile> b )
{
   bundleFile = b;
}
void SettingsBundle::openSection( const QString &psect )
{
   if ( psect.size() )
      currsection = psect;
   else
      currsection = noneBundle;
}

bool SettingsBundle::isSectionPresent(QString sname)
{
    return bundleFile->iniFile->isSectionPresent(sname);
}
bool SettingsBundle::isCurrSectionPresent()
{
    return isSectionPresent(currsection);
}
void SettingsBundle::closeProfile()
{
   flushProfile();
}
QString SettingsBundle::getSection()
{
   return currsection;
}
bool SettingsBundle::newSection( const QString &newname )
{
   // Create a new section with no match
   if (!isSectionPresent(newname))
   {
      currsection = newname;
      for ( QVector<QSharedPointer<ProfileEntry> >::iterator i = bundleFile->entries.begin(); i != bundleFile->entries.end(); i++ )
      {
         (*i)->createEntry( this );
      }
      return true;
   }
   return false;
}

bool SettingsBundle::dupSection( const QString &newname )
{
   if ( currsection == noneBundle )
   {
      return false;
   }
   // Create a new section that matches the old
   return bundleFile->iniFile->dupSection( currsection, newname );
}

bool SettingsBundle::renameSection( const QString &newname )
{
   if ( currsection == noneBundle )
   {
      return false;
   }
   // Create a new section that matches the old
   if (bundleFile->iniFile->dupSection( currsection, newname ))
   {
        clearProfileSection(true);
        currsection = newname;
   }
   return true;
}

bool SettingsBundle::checkKeyExists(const QString &key )
{
   if ( !bundleFile || currsection == noneBundle )
   {
      return false;
   }
   return bundleFile->iniFile->checkKeyExists( currsection, key );
}
QString SettingsBundle::displayNameOf( int enumKey )
{
   if ( bool( bundleFile ) /*&& currsection != noneBundle*/ )
   {
      for ( QVector<QSharedPointer <ProfileEntry> >::iterator i = bundleFile->entries.begin(); i != bundleFile->entries.end(); i++ )
      {
         if ( ( *i )->id == enumKey )
         {
            if (( *i )->dispname.size())
            {
               return ( *i )->dispname;
            }
            return ( *i )->name;
         }
      }
   }
   return "";
}
bool SettingsBundle::isReadOnly( int enumKey )
{
   if ( bool( bundleFile ) && currsection != noneBundle )
   {
      for ( QVector<QSharedPointer <ProfileEntry> >::iterator i = bundleFile->entries.begin(); i != bundleFile->entries.end(); i++ )
      {
         if ( ( *i )->id == enumKey )
         {
            return ( *i )->RO;
         }
      }
   }
   return true;
}
void SettingsBundle::startGroup()
{
    if (bundleFile && bundleFile->iniFile)
        bundleFile->iniFile->startGroup();
}
void SettingsBundle::endGroup()
{
    if (bundleFile && bundleFile->iniFile)
        bundleFile->iniFile->endGroup();
}

void SettingsBundle::getBoolProfile(const QString &key, bool &value, bool def )
{
   if ( !bundleFile || currsection == noneBundle )
   {
      value = def;
      return ;
   }
   int intval = bundleFile->iniFile->getPrivateProfileInt( currsection, key,  (def?true:false) );
   if ( intval )
      value = true;
   else
      value = false;
}
void SettingsBundle::getBoolProfile( int enumkey, bool &value )
{
   QSharedPointer <ProfileEntry>  p = bundleFile->GetKey( enumkey );
   getBoolProfile( p->name, value, p->bdefaultval );
}
void SettingsBundle::getBoolProfile( int enumkey, MinosItem<bool> &value )
{
   bool temp;
   getBoolProfile( enumkey, temp );
   value.setValue( temp );
}
void SettingsBundle::setBoolProfile( const QString &key, bool value )
{
   if ( !bundleFile || currsection == noneBundle )
   {
      return ;
   }
   bundleFile->iniFile->writePrivateProfileString( currsection, key, value ? "1" : "0" );
}
void SettingsBundle::setBoolProfile( int enumkey, bool value )
{
   QSharedPointer <ProfileEntry> p = bundleFile->GetKey( enumkey );
   setBoolProfile( p->name, value );
}
void SettingsBundle::getStringProfile(const QString &key, QString &value, const QString &def )
{
   if ( !bundleFile || currsection == noneBundle )
   {
      value = def;
      return ;
   }
   QString inbuff;
   bundleFile->iniFile->getPrivateProfileString( currsection, key, def, inbuff );
   value = inbuff;
}
void SettingsBundle::getStringProfile( int enumkey, QString &value )
{
   QSharedPointer <ProfileEntry>  p = bundleFile->GetKey( enumkey );
   getStringProfile( p->name, value, p->sdefaultval );
}

void SettingsBundle::getStringProfile(int enumkey, MinosStringItem<QString> &value )
{
   QString temp;
   getStringProfile( enumkey, temp );
   value.setValue( temp );
}
void SettingsBundle::setStringProfile( const QString &key, const QString &value )
{
   if ( !bundleFile || currsection == noneBundle )
   {
      return ;
   }
   bundleFile->iniFile->writePrivateProfileString( currsection, key, value );
}
void SettingsBundle::setStringProfile(int enumkey, const QString &value )
{
   QSharedPointer <ProfileEntry>  p = bundleFile->GetKey( enumkey );
   setStringProfile( p->name, value );
}

void SettingsBundle::getIntProfile( const QString &key, int &value, int def )
{
   if ( !bundleFile || currsection == noneBundle )
   {
      value = def;
      return ;
   }
   value = bundleFile->iniFile->getPrivateProfileInt( currsection, key, def );
}
void SettingsBundle::getIntProfile( int enumkey, int &value )
{
   QSharedPointer <ProfileEntry>  p = bundleFile->GetKey( enumkey );
   getIntProfile( p->name, value, p->idefaultval );
}
void SettingsBundle::getIntProfile( int enumkey, MinosItem<int> &value )
{
   int temp;
   getIntProfile( enumkey, temp );
   value.setValue( temp );
}
void SettingsBundle::setIntProfile(const QString &key, int value )
{
   if ( !bundleFile || currsection == noneBundle )
   {
      return ;
   }
   QString sval = QString::number( value );
   bundleFile->iniFile->writePrivateProfileString( currsection, key, sval );
}
void SettingsBundle::setIntProfile( int enumkey, int value )
{
   QSharedPointer <ProfileEntry>  p = bundleFile->GetKey( enumkey );
   setIntProfile( p->name, value );
}

QStringList SettingsBundle::getProfileEntries( )
{
   QStringList sectlist;
   if ( !bundleFile || currsection == noneBundle )
   {
      return sectlist;
   }

   /*int mlen =*/ bundleFile->iniFile->getPrivateProfileList( currsection, "", sectlist );

   return sectlist;
}
QVector<int> SettingsBundle::getBundleEntries( )
{
   QVector<int> e;
   if ( bundleFile )
   {
      for ( QVector<QSharedPointer <ProfileEntry> >::iterator i = bundleFile->entries.begin(); i != bundleFile->entries.end(); i++ )
      {
         e.push_back( ( *i )->id );
      }
   }
   return e;

}
QStringList SettingsBundle::getBundleHints( )
{
   QStringList e;
   if ( bundleFile )
   {
      for ( QVector<QSharedPointer <ProfileEntry> >::iterator i = bundleFile->entries.begin(); i != bundleFile->entries.end(); i++ )
      {
         e.append( ( *i )->hint );
      }
   }
   return e;
}
QStringList SettingsBundle::getSections( )
{
   QStringList slist;
   if ( !bundleFile )
   {
      return slist;
   }

   slist.append( noneBundle );

   slist.append(bundleFile->iniFile->getSections());

   if (currsection.size() && std::find(slist.begin(), slist.end(), currsection) == slist.end())
   {
      slist.append(currsection);
   }
   return slist;
}
void SettingsBundle::clearProfileSection( bool clearCurr)
{
   // clear the content AND the section header
   if ( bundleFile )
   {
      bundleFile->iniFile->writePrivateProfileString( currsection, "", "" );
      if (clearCurr)
      {
         currsection = noneBundle;
      }
   }
}
void SettingsBundle::flushProfile( void )
{
   if ( !bundleFile || !bundleFile->iniFile )
   {
      return ;
   }
   bundleFile->iniFile->writePrivateProfileString( "", "", "" );
}
//=============================================================================
ProfileEntry::ProfileEntry(int id, const char *name, const char *def, const char *dispname, const char *hint, bool RO )
      : id( id ), name( name ), sdefaultval( def ), hint( hint ), pt( petString ), dispname(dispname), RO(RO)
{}
/*
ProfileEntry::ProfileEntry(int id, const QString &n, const QString &d, const QString &dname, const QString &h, bool RO )
      : id( id ), name( n ), sdefaultval( d ), hint( h ), pt( petString ), dispname(dname), RO(RO)
{}
*/
ProfileEntry::ProfileEntry(int id, const char *name, int def, const char *dispname, const char *hint, bool RO )
      : id( id ), name( name ), idefaultval( def ), sdefaultval( QString::number( def ) ), hint( hint ), pt( petInteger ), dispname(dispname), RO(RO)
{}
ProfileEntry::ProfileEntry(int id, const char *name, bool def, const char *dispname, const char *hint, bool RO )
      : id( id ), name( name ), bdefaultval( def ), sdefaultval( def?"1":"0" ), hint( hint ), pt( petBool ), dispname(dispname), RO(RO)
{}
void ProfileEntry::createEntry( SettingsBundle *s )
{
   switch ( pt )
   {
      case petString:
         s->setStringProfile( name, sdefaultval ) ;
         break;
      case petInteger:
         s->setIntProfile( name, idefaultval ) ;
         break;
      case petBool:
         s->setBoolProfile( name, bdefaultval ) ;
         break;
   }
}

