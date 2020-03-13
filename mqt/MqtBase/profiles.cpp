/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
#include "base_pch.h"

#include "MLogFile.h"
#include "INIFile.h"
#include "profiles.h"
//#include "ScreenConfigFile.h"
QString defaultLayoutName()
{
    return QCoreApplication::translate("Default Screen Config", "default");
}
QString defaultSessionName()
{
    return QCoreApplication::translate("Default Log Session", "Default Session");
}

AppSettingsBundle::AppSettingsBundle():
    SettingsBundle()
{}
bool AppSettingsBundle::populateDefaultSection()
{
    return true;
}
//=============================================================================
BundleFile::BundleFile( PROFILES p )
{
    switch ( p )
    {
    //       ProfileEntry(int id, const  char *name, const char *def, const char *dispname, const char *hint, bool RO );
    case epLOGGERPROFILE:

        entries.push_back( QSharedPointer<ProfileEntry> (new ProfileEntry( elpListDirectory, "List Directory", "./Lists", "", QT_TR_NOOP("Default archive list directory"), false ) ) );
        entries.push_back( QSharedPointer<ProfileEntry> (new ProfileEntry( elpLogDirectory, "Log Directory", "./Logs", "", QT_TR_NOOP("Default logs directory"), false ) ) );

        entries.push_back( QSharedPointer<ProfileEntry> (new ProfileEntry( elpEntryFile, "EntryFile", "./Configuration/Entry.ini", QT_TR_NOOP("Entry settings file"), QT_TR_NOOP("File containing entry settings"), false ) ) );
        entries.push_back( QSharedPointer<ProfileEntry> (new ProfileEntry( elpStationFile, "StationFile", "./Configuration/Station.ini", QT_TR_NOOP("Station settings file"), QT_TR_NOOP("File containing station settings"), false ) ) );
        entries.push_back( QSharedPointer<ProfileEntry> (new ProfileEntry( elpQTHFile, "QTHFile", "./Configuration/QTH.ini", QT_TR_NOOP("QTH settings file"), QT_TR_NOOP("File containing QTH settings"), false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( elpLocsFile, "LocsFile", "./Configuration/LocSquares.ini", QT_TR_NOOP("Country locators file"), QT_TR_NOOP("File containing valid locators for countries"), false ) ));

        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( elpDisplayFile, "DisplayFile", "./Configuration/Display.ini", QT_TR_NOOP("Display settings file"), QT_TR_NOOP("File containing saved display settings"), false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( elpDisplaySection, "DisplaySection", "Default", QT_TR_NOOP("Display file section"), QT_TR_NOOP("Section to use in display file"), false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( elpOperatorFile, "OperatorFile", "./Configuration/Operator.ini", QT_TR_NOOP("Operators file"), QT_TR_NOOP("File containing operators"), false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( elpOperatorSection, "OperatorSection", "Default", QT_TR_NOOP("Operators file section"), QT_TR_NOOP("section to use in operators file"), false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( elpPreloadFile, "PreloadFile", "./Configuration/LogsPreload.ini", QT_TR_NOOP("Log preload file"), QT_TR_NOOP("File containing log pre-loads"), false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( elpPreloadSection, "PreloadSection", "Default", QT_TR_NOOP("Preload contests default section"), QT_TR_NOOP("Section to use in preload file"), false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( elpListsFile, "ListsFile", "./Configuration/ListPreload.ini", QT_TR_NOOP("List preload file"), QT_TR_NOOP("File containing list pre-loads"), false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( elpListsSection, "ListsSection", "Default", QT_TR_NOOP("Preload Lists file section"), QT_TR_NOOP("Section to use in lists preload file"), false ) ) );

        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( elpWSJTXEnabled, "WSJTXEnabled", true, QT_TR_NOOP("WSJTX Enabled"), QT_TR_NOOP("WSJTX Enabled"), false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( elpWSJTXPort, "WSJTXPort", 2237, QT_TR_NOOP("WSJTX Port"), QT_TR_NOOP("WSJTX Port"), false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( elpWSJTXGroupAddress, "WSJTXGroupAddress", "", QT_TR_NOOP("WSJTX Group Address"), QT_TR_NOOP("WSJTX Group Address"), false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( elpWSJTXAutoEnabled, "WSJTXAutoEnabled", false, QT_TR_NOOP("WSJTX Auto Enabled"), QT_TR_NOOP("WSJTX Auto Enabled"), false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( elpWSJTXTestEnabled, "WSJTXtestEnabled", false, QT_TR_NOOP("WSJTX Test Enabled"), QT_TR_NOOP("WSJTX Test Enabled"), false ) ) );


        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( elpcontactsSelect, "UDPContactsEnabled", false, QT_TR_NOOP("UDP Contacts Enabled"), QT_TR_NOOP("UDP Contacts Enabled"), false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( elpextCSSelect, "UDPextCSEnabled", false, QT_TR_NOOP("UDP External Callsign Lookup Enabled"), QT_TR_NOOP("UDP External Callsign Lookup Enabled"), false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( elpwsjtxRbSelect, "WSJTXRbEnabled", false, QT_TR_NOOP("WSJTX re-broadcast Enabled"), QT_TR_NOOP("WSJTX re-broadcast Enabled"), false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( elpADIFSelect, "ADIFEnabled", false, QT_TR_NOOP("UDP ADIF Enabled"), QT_TR_NOOP("UDP ADIF Enabled"), false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( elpcontactsAddr, "UDPContactsAddr", "", QT_TR_NOOP("UDP Contacts Address"), QT_TR_NOOP("UDP Contacts Address"), false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( elpcontactsPort, "UDPContactsPort", 12060, QT_TR_NOOP("UDP Contacts Port"), QT_TR_NOOP("UDP Contacts Port"), false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( elpextCSAddr, "UDPExtCSAddress", "", QT_TR_NOOP("UDP External Callsign Lookup Address"), QT_TR_NOOP("UDP External Callsign Lookup  Address"), false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( elpADIFAddr, "UDPADIFAddr", "", QT_TR_NOOP("UDP ADIF Address"), QT_TR_NOOP("UDP ADIF Address"), false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( elpADIFPort, "UDPADIFPort", 12060, QT_TR_NOOP("UDP ADIF Port"), QT_TR_NOOP("UDP ADIF Port"), false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( elpextCSPort, "UDPExtCSPort", 12060, QT_TR_NOOP("UDP External Callsign Lookup  Port"), QT_TR_NOOP("UDP External Callsign Lookup  Port"), false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( elpwsjtxRbAddr,"WSJTXRbAddress", "", QT_TR_NOOP("WSJTX re-broadcast Address"), QT_TR_NOOP("WSJTX re-broadcast Address"), false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( elpwsjtxRbPort, "WSJTXRbPort", 12060, QT_TR_NOOP("WSJTX re-broadcast Port"), QT_TR_NOOP("WSJTX re-broadcast Port"), false ) ) );


        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( elpHelpFile, "HelpFile", "./helpfiles/MinosHelp.qch", QT_TR_NOOP("Help file"), QT_TR_NOOP("Help file"), false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( elpHelpEntryURL, "HelpEntryPoint", "qthelp://Minos/Minos.html", QT_TR_NOOP("Help entry point"), QT_TR_NOOP("Help entry point"), false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( elpPDFFile, "PDFFile", "./Docs/Minos.pdf", QT_TR_NOOP("PDF documentation file"), QT_TR_NOOP("PDF documentation file"), false ) ) );

        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( elpAutoFill, "AutoFill", false, QT_TR_NOOP("Auto Fill signal report"), QT_TR_NOOP("Auto Fill signal report on return"), false ) ) );

        break;
    case epPRELOADPROFILE:
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( eppCurrent, "CurrentLog", 0, nullptr, QT_TR_NOOP("hint"), false ) ) );
        {
            QByteArray temp = defaultSessionName().toUtf8();

            entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( eppDefSession, "DefaultSessionName", temp.constData(), temp, QT_TR_NOOP("hint"), false ) ) );
            entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( eppSession, "CurrentSession", temp.constData(), temp, QT_TR_NOOP("hint"), false ) ) );
        }
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

        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( edpShowOperators, "ShowOperators", true, "", "hint", false ) ) );

        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( edpEditor, "Editor", "Notepad.exe", "", QT_TR_NOOP("Default editor"), false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( edpStatisticsPeriod1, "Statistics Period 1", 10, "", QT_TR_NOOP("Statistics Period 1"), false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( edpStatisticsPeriod2, "Statistics Period 2", 60, "", QT_TR_NOOP("Statistics Period 2"), false ) ) );

        {
            QByteArray temp = defaultLayoutName().toUtf8();
            entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( edpCurrentLayout, "CurrentLayout", temp.constData(), temp, "hint", false ) ) );
        }
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( edpListCompression, "List Compression Factor", 100, "", "hint", false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( edpQSOFieldFont, "QSO Field Expansion Factor", 100, "", "hint", false ) ) );

        break;
    case epENTRYPROFILE:
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( eepCall, "Call", "", QT_TR_NOOP("Call Used"), QT_TR_NOOP("Call sign used"), false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( eepEntrant, "Entrant", "", QT_TR_NOOP("On Behalf Of (Club)"), QT_TR_NOOP("Name of club/group"), false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( eepMyName, "MyName", "", QT_TR_NOOP("My Name"), QT_TR_NOOP("Name of responsible operator"), false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( eepMyCall, "MyCall", "", QT_TR_NOOP("My Call"), QT_TR_NOOP("Callsign of responsible operator"), false ) ) );

        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( eepMyAddress1, "MyAddress1", "", QT_TR_NOOP("My Address Line 1"), QT_TR_NOOP("Address line 1 of responsible operator"), false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( eepMyAddress2, "MyAddress2", "", QT_TR_NOOP("My Address Line 2"), QT_TR_NOOP("Address line 2 of responsible operator"), false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( eepMyCity, "MyCity", "", QT_TR_NOOP("My City"), QT_TR_NOOP("City of responsible operator"), false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( eepMyCountry, "MyCountry", "",QT_TR_NOOP("My Country"), QT_TR_NOOP("Country of responsible operator"), false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( eepMyPostCode, "MyPostCode", "", QT_TR_NOOP("My Postcode"), QT_TR_NOOP("Post Code of responsible operator"), false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( eepMyPhone, "MyPhone", "", QT_TR_NOOP("My Phone"), QT_TR_NOOP("Phone no. of responsible operator"), false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( eepMyEmail, "MyEmail", "", QT_TR_NOOP("My Email"), QT_TR_NOOP("eMail address of responsible operator"), false ) ) );
        break;
    case epQTHPROFILE:
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( eqpLocator, "Locator", "", QT_TR_NOOP("Locator"), QT_TR_NOOP("Locator"), false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( eqpDistrict, "District", "", QT_TR_NOOP("District Exchange"), QT_TR_NOOP("District Exchange"), false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( eqpLocation, "Location", "", QT_TR_NOOP("Location Exchange"), QT_TR_NOOP("Descriptive Location Exchange"), false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( eqpStationQTH1, "StationQTH1", "", QT_TR_NOOP("Station QTH Line 1"), QT_TR_NOOP("Address line 1/2 of station"), false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( eqpStationQTH2, "StationQTH2", "", QT_TR_NOOP("Station QTH Line 2"), QT_TR_NOOP("Address line 2/2 of station"), false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( eqpASL, "ASL", 0, QT_TR_NOOP("QTH Height ASL (metres)"), QT_TR_NOOP("QTH height ASL (metres)"), false ) ) );
        break;
    case epSTATIONPROFILE:
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( espPower, "Power", 0, QT_TR_NOOP("Transmitter Power"), QT_TR_NOOP("Transmit Power (Watts)"), false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( espTransmitter, "Transmitter", "", QT_TR_NOOP("Transmit Equipment"), QT_TR_NOOP("Transmit Equipment"), false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( espReceiver, "Receiver", "", QT_TR_NOOP("Receive Equipment"), QT_TR_NOOP("Receive Equipment"), false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( espAntenna, "Antenna", "", QT_TR_NOOP("Antenna details"), QT_TR_NOOP("Antenna details"), false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( espAGL, "AGL", 0, QT_TR_NOOP("Antenna Height AGL (metres)"), QT_TR_NOOP("Antenna Height AGL (metres)"), false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( espOffset, "Bearing Offset", 0, QT_TR_NOOP("Antenna Bearing Offset"), QT_TR_NOOP("Amount to offset antenna bearings"), false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( espRadioName, "Radio", "", QT_TR_NOOP("Radio in Rig Control"), QT_TR_NOOP("Radio in Rig Control"), false ) ) );
        entries.push_back(  QSharedPointer<ProfileEntry> (new ProfileEntry( espRotatorName, "rotator", "", QT_TR_NOOP("Rotator in Rotator Control"), QT_TR_NOOP("Rotator in Rotator Control"), false ) ) );
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
SettingsBundle::SettingsBundle():noneBundle(tr("<None>"))
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
            if (strlen(( *i )->dispname))
            {
               return BundleFile::tr(( *i )->dispname);
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
         e.append( BundleFile::tr(( *i )->hint) );
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
void SettingsBundle::flushProfile( )
{
   if ( !bundleFile || !bundleFile->iniFile )
   {
      return ;
   }
   bundleFile->iniFile->writePrivateProfileString( "", "", "" );
}
//=============================================================================
ProfileEntry::ProfileEntry(int id, const char *name, const char *def, const char * dispname, const char * hint, bool RO )
      :
        pt( petString ),
        id( id ),
        name( name ),
        dispname(dispname),
        sdefaultval( def ),
        hint( hint ),
        RO(RO)
{}
/*
ProfileEntry::ProfileEntry(int id, const QString &n, const QString &d, const QString &dname, const QString &h, bool RO )
      : id( id ), name( n ), sdefaultval( d ), hint( h ), pt( petString ), dispname(dname), RO(RO)
{}
*/
ProfileEntry::ProfileEntry(int id, const char *name, int def, const char * dispname, const char * hint, bool RO )
      :
        pt( petInteger ),
        id( id ),
        name( name ),
        dispname(dispname),
        sdefaultval( QString::number( def ) ),
        idefaultval( def ),
        hint( hint ),
        RO(RO)
{}
ProfileEntry::ProfileEntry(int id, const char *name, bool def, const char * dispname, const char * hint, bool RO )
      :
        pt( petBool ),
        id( id ),
        name( name ),
        dispname(dispname),
        sdefaultval( def?"1":"0" ),
        bdefaultval( def ),
        hint( hint ),
        RO(RO)
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

