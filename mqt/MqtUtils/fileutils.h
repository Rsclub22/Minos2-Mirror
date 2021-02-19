#ifndef FILEUTILS_H
#define FILEUTILS_H
#include <cstdint>
#include <QDir>
#include <QFileInfo>
#include <QFile>

QString GetCurrentDir();
void SetCurrentDir( const QString &dir );

QString ExtractFileDir( const QString &fname );
QString ExtractFilePath(const QString &fname );
QString ExtractFileName(const QString &fname );
QString ExtractFileExt(const QString &fname );

bool FileExists( const QString& Name );
bool FileAccessible(const QString &fname);
bool FileWriteable(const QString &fname);
bool DirectoryExists ( const QString & Name );

bool CreateDir( const QString &s );

qint64 FileLength(const QString &fname);

#endif // FILEUTILS_H
