#ifndef INIFILE_H
#define INIFILE_H
/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
#include "mqtUtils_pch.h"

#include "MLogFile.h"

// Here we define all the possible profile entries in a section
// we use them for new sections in a bundle, and also to get
// values from a bundle.

// What do we do about variable entries? Do we allow them?
// What about about entries for other Minos programs
// where we won't know what is is relevant?

class INISection;
typedef INISection *IniSectionPtr;
class INIFile
{
    friend class INISection;
private:
    QVector <IniSectionPtr> sections;

    bool fileDirty;
    bool fileLoaded;

    bool isDirty();
    void setClean();

    QString loadedFileName;

    QFileInfo statbuf;

    bool inGroup = false;

public:
    void loadINIFile( );
    bool writeINIFile( );
    bool checkStat();
    bool checkKeyExists(const QString &Section,
                        const QString &Entry
                        );
    int getPrivateProfileString(const QString &Section,
                                const QString &Entry,
                                const QString &DefaultValue,
                                QString &Buffer);
    int getPrivateProfileList(const QString &Section,
                              const QString &Entry,
                              QStringList &Buffer);

    int getPrivateProfileInt( const QString &Section,
                              const QString &Entry,
                              int DefaultValue );

    bool getPrivateProfileBool(const QString &Section,
                                const QString &Entry,
                                bool DefaultValue );

    bool writePrivateProfileString( const QString &Section,
                                    const QString &Entry,
                                    const QString &Buffer );

    bool writePrivateProfileBool( const QString &Section,
                                    const QString &Entry,
                                    bool val );

    bool dupSection( const QString &oldname, const QString &newname );
    bool isSectionPresent(QString sname);
    QStringList getSections( );

    void startGroup();
    void endGroup();

    INIFile(const QString &name );
    ~INIFile();
};

#endif // INIFILE_H
