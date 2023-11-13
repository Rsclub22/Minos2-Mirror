#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <QString>

QString GetCurrentDir();
void SetCurrentDir( const QString &dir );

QString ExtractFileDir( const QString &fname );
QString ExtractFileName(const QString &fname );
QString ExtractFileExt(const QString &fname );

bool FileExists( const QString& Name );
bool FileAccessible(const QString &fname);
bool FileWriteable(const QString &fname);
bool FileExecutable(const QString &fname);
bool DirectoryExists ( const QString & Name );

bool CreateDir( const QString &s );

qint64 FileLength(const QString &fname);

QString GetCleanPath(QString);

enum DirectoryLocation {dlBinaries
                         ,dlTranslations
                         ,dlConfiguration
                         ,dlLists
                         ,dlLogs
                         ,dlHelp
                         ,dlDocs
                         ,dlTraceLog
                         ,dlQRZDB
};

QString getDirectoryLocation(DirectoryLocation, QString runDir = ".");
void setDefLogDir(QString l);
void setDefListDir(QString l);
#endif // FILEUTILS_H
