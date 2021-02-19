#include "fileutils.h"

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
    QString p = finf.dir().canonicalPath();
    return p;
}
QString ExtractFilePath( const QString &fname )
{
    return ExtractFileDir( fname );
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
    QDir dir( s );
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

