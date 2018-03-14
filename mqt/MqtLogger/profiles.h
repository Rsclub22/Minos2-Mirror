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
//----------------------------------------------------------------------------
class SettingsBundle;
class ProfileEntry
{
      enum peType{petString, petInteger, petBool} pt;
   public:
      int id;
      QString name;
      QString dispname;

      QString sdefaultval;
      int idefaultval;
      bool bdefaultval;

      QString hint;
      bool RO;

      void createEntry( SettingsBundle * );
      ProfileEntry(int id, const  char *name, const char *def, const char *dispname, const char *hint, bool RO );
      ProfileEntry(int id, const  char *name, int def, const  char *dispname, const  char *hint, bool RO );
      ProfileEntry(int id, const char *name, bool def, const char *dispname, const char *hint, bool RO );
};
class INIFile;

class BundleFile
{
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
   protected:
      QString currsection;
   public:
      QSharedPointer<BundleFile> bundleFile;
      QString noneBundle;

      SettingsBundle( );
      ~SettingsBundle();
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

      void closeProfile( void );

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
    AppSettingsBundle():SettingsBundle()
    {
        noneBundle = "<DefaultApps>";
    }
    virtual bool populateDefaultSection()override {return true;}
};
#endif
