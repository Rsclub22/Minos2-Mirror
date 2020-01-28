/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifndef ProfilesH
#define ProfilesH 
#include "base_pch.h"
#include "ProfileEnums.h"
//----------------------------------------------------------------------------
extern const QString defaultLayoutName;

class SettingsBundle;
class ProfileEntry
{
      enum peType{petString, petInteger, petBool} pt;
   public:
      int id;
      QString name;
      const char * dispname;

      QString sdefaultval;
      int idefaultval;
      bool bdefaultval;

      const char * hint;
      bool RO;

      void createEntry( SettingsBundle * );
      ProfileEntry(int id, const  char *name, const char * def, const char * dispname, const char * hint, bool RO );
      ProfileEntry(int id, const  char *name, int def, const char * dispname, const char * hint, bool RO );
      ProfileEntry(int id, const char *name, bool def, const char * dispname, const char * hint, bool RO );
};
class INIFile;

class BundleFile
{
    Q_DECLARE_TR_FUNCTIONS(BundleFile)

   private:
      QString bundleName;
   public:
      QSharedPointer<INIFile> iniFile;
      QVector<QSharedPointer<ProfileEntry> > entries;

      static QSharedPointer<BundleFile>bundleFiles[ epMAXPROFILE ];
      BundleFile( PROFILES p );
      ~BundleFile();
      static QSharedPointer<BundleFile> getBundleFile( PROFILES p );
      bool openProfile( const QString &pname, const QString &bname );
      QString getBundle()
      {
         return bundleName;
      }
      QSharedPointer<ProfileEntry> &GetKey( int p );
};
class SettingsBundle
{
    Q_DECLARE_TR_FUNCTIONS(SettingsBundle)
   protected:
      QString currsection;
   public:
      QSharedPointer<BundleFile> bundleFile;
      QString noneBundle;

      SettingsBundle( );
      virtual ~SettingsBundle();
      QString getBundle();

      void checkLoaded();

      void setProfile( QSharedPointer<BundleFile> b );

      void openSection( const QString &psect );
      QString getSection();
      bool dupSection( const QString &sname );
      bool newSection( const QString &sname );
      bool renameSection( const QString &sname );
      bool isSectionPresent(QString sname);
      bool isCurrSectionPresent();

      void closeProfile( );

      bool checkKeyExists( const QString &key );
      QString displayNameOf( int enumKey );
      bool isReadOnly( int enumKey );
      void getBoolProfile( const QString &key, bool &value, bool def );
      void getBoolProfile( const QString &key, MinosItem<bool> &value, bool def );
      void setBoolProfile(const QString &key, bool value );
      void getBoolProfile( int enumkey, bool &value );
      void getBoolProfile( int enumkey, MinosItem<bool> &value );
      void setBoolProfile( int enumkey, bool value );

      void getStringProfile( const QString &key, QString &value, const QString &def );
      void getStringProfile( const QString &key, MinosStringItem<QString> &value, const QString &def );
      void setStringProfile(const QString &key, const QString &value );
      void getStringProfile( int enumkey, QString &value );
      void getStringProfile( int enumkey, MinosStringItem<QString> &value );
      void setStringProfile( int enumkey, const QString &value );

      void getIntProfile(const QString &key, int &value, int def );
      void getIntProfile( const QString &key, MinosItem<int> &value, int def );
      void setIntProfile( const QString &key, int value );
      void getIntProfile( int enumkey, int &value );
      void getIntProfile( int enumkey, MinosItem<int> &value );
      void setIntProfile( int enumkey, int value );

      void flushProfile( );
      void clearProfileSection(bool clearCurr );

      QStringList getProfileEntries( );
      QVector<int> getBundleEntries( );
      QStringList getBundleHints( );
      QStringList getSections( );

      void startGroup();
      void endGroup();

      virtual bool populateDefaultSection(){return false;}
};
class AppSettingsBundle:public SettingsBundle
{
public:
    AppSettingsBundle();
    virtual bool populateDefaultSection()override;
};
#endif
