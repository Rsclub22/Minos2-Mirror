#include <cstdint>
#include <QDir>
#include <QFileInfo>
#include <QFile>

#include "fileutils.h"

static QString deflog;
static QString deflist;

void setDefLogDir(QString l)
{
    deflog = l;
}
void setDefListDir(QString l)
{
    deflist = l;
}
QString getDirectoryLocation(DirectoryLocation dl, QString runDir)
{
    // At the moment, these are all Windows, and relative
    // to the current working directory

    QString dirLoc;
    switch (dl)
        {
#ifdef Q_OS_MACOS
    case dlBinaries:
        dirLoc = runDir + "/Bin";
        break;

    case dlTranslations:
        dirLoc = runDir + "/Bin/translations";
        break;

    case dlConfiguration:
        dirLoc = runDir + "/Configuration";
        break;

    case dlLists:
        dirLoc = deflist;
        break;

    case dlLogs:
        dirLoc = deflog;
        break;

    case dlDocs:
        dirLoc = runDir + "/Docs";
        break;

    case dlTraceLog:
        dirLoc = runDir + "/TraceLog";
        break;

    case dlQRZDB:
        dirLoc = runDir;
        break;
#else
        case dlBinaries:
            dirLoc = runDir + "/Bin";
            break;

        case dlTranslations:
            dirLoc = runDir + "/Bin/translations";
            break;

        case dlConfiguration:
            dirLoc = runDir + "/Configuration";
            break;

        case dlLists:
            dirLoc = deflist;
            break;

        case dlLogs:
            dirLoc = deflog;
            break;

        case dlDocs:
            dirLoc = runDir + "/Docs";
            break;

        case dlTraceLog:
            dirLoc = runDir + "/TraceLog";
            break;

        case dlQRZDB:
            dirLoc = runDir;
            break;
#endif

        }
    QDir d(dirLoc);
    dirLoc = d.absolutePath();
    return dirLoc;
}

bool FileExists(const QString &path )
{
    QFileInfo checkFile( path );
    // check if file exists and if yes: Is it really a file and no directory?
    if ( checkFile.exists() && checkFile.isFile() )
    {
        return true;
    }
    else
    {
        return false;
    }
}
bool FileExecutable(const QString &path)
{
    QFileInfo checkFile( path );
    // check if file exists and if yes: Is it really a file and no directory?
    if ( checkFile.exists() && checkFile.isExecutable() )
    {
        return true;
    }
    else
    {
        return false;
    }
}
bool DirectoryExists (const QString &Name )
{
    QFileInfo qinf( Name );
    bool dir = qinf.isDir();
    return dir;
}

QString GetCurrentDir()
{
    QDir dir( "." );
    return dir.absolutePath();
}
QString ExtractFileDir(const QString &fname )
{
    QFileInfo finf( fname );
    QString p = finf.dir().absolutePath();
    return p;
}
QString ExtractFileName(const QString &fname )
{
    QFileInfo finf( fname );
    return finf.fileName();
}
QString ExtractFileExt( const QString &fname )
{
    QFileInfo finf( fname );
    return ( "." + finf.suffix() );
}
#ifdef Q_OS_WIN
extern Q_CORE_EXPORT int qt_ntfs_permission_lookup;
#else
int qt_ntfs_permission_lookup = 0;
#endif

bool FileAccessible(const QString &fname)
{
    qt_ntfs_permission_lookup++;
    bool exists = FileExists(fname);
    bool readable = QFileInfo(fname).isReadable();

    qt_ntfs_permission_lookup--;
    return exists && readable;
}
bool FileWriteable(const QString &fname)
{
    qt_ntfs_permission_lookup++;
    bool exists = FileExists(fname);
    bool writeable = QFileInfo(fname).isWritable();

    qt_ntfs_permission_lookup--;
    return exists && writeable;
}
const QString ExcludeTrailingBackslash( const QString &s )
{
    if ( s.length() && ( s[ s.length() - 1 ] == '\\' || s[ s.length() - 1 ] == '/' ) )
    {
        QString s1 = s.mid(s.length() - 1);
        return s1;
    }
    return s;
}

//---------------------------------------------------------------------------
bool CreateDir(const QString &s )
{
    QString s1 = ExcludeTrailingBackslash(s);
    QDir dir( s1 );
    if ( !dir.exists() )
    {
        dir.mkpath( "." );
    }
    return dir.exists();
}
void SetCurrentDir( const QString &dir )
{
    QDir::setCurrent( dir );
}
qint64 FileLength(const QString &fname)
{
    QFileInfo qinf( fname );
    qint64 len = qinf.size();
    return len;
}
QString GetCleanPath(QString fn)
{
    QDir cdir(GetCurrentDir());
    fn = cdir.cleanPath(fn);
    fn = cdir.absoluteFilePath(fn);
    if (fn.indexOf(GetCurrentDir()) == 0)
    {
        fn = cdir.relativeFilePath(fn);
    }
    return fn;
}
