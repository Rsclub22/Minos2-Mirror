/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------

#include "mqtUtils_pch.h"
#include "fileutils.h"

QMutex CsGuard::m_mutex(QMutex::Recursive);
//---------------------------------------------------------------------------
static QTextStream &getLogFile( QString name )
{
    static QTextStream logFile;
    static bool opened = false;
    if (!opened && !name.isEmpty())
    {
        static QFile qfn(name);
        if (qfn.open(QIODevice::WriteOnly|QIODevice::Text))
        {
            logFile.setDevice(&qfn);
        }
        opened = true;
    }
    return logFile;
}

//---------------------------------------------------------------------------
void MLogFile::createLogFile(const QString &path, const QString filePrefix, int keepDays )
{
    StaticDirectoryCreate ( path );

    QString TidyPrefix = path + "/" + filePrefix + "*";
    tidyFiles ( TidyPrefix, keepDays );
    QString fLogFileName = generateLogFileName ( path + "/" + filePrefix  );

    QString dtg = QDateTime::currentDateTime().toString( "yyyy MMM dd HH:mm:ss.zzz" );

	getLogFile( fLogFileName );
    QTextStream &l = log();
    l << dtg << "\n";
    l.flush();
}
//---------------------------------------------------------------------------
QTextStream & MLogFile::log( )
{
	return getLogFile( "" );
}
//---------------------------------------------------------------------------
QTextStream &MLogFile::logT( )
{
    CsGuard lock;
    QDateTime dt = QDateTime::currentDateTime();
    QString time = dt.toString( "HH:mm:ss.zzz" );
    return log() << time;
}
//---------------------------------------------------------------------------
void MLogFile::close( )
{
    QTextStream &l = log();
    l.flush();
    l.device()->close();
    l.setDevice(nullptr);
}
//---------------------------------------------------------------------------
QTextStream & MLogFile::log(const QString &s )
{
   CsGuard scoped_lock;
   QTextStream &l = logT();
   l << " " << s << "\n";
   l.flush();
   return l;
}

//---------------------------------------------------------------------------
/*static */QString MLogFile::generateLogFileName(const QString &prefix )
{
   QDateTime dt = QDateTime::currentDateTime();
   QString s = prefix;
   s += dt.toString( "yyyyMMdd_HHmmss" ) + ".log";
   return s;
}
//---------------------------------------------------------------------------
/*static */
void MLogFile::tidyFiles (const QString &Prefix, int KeepDays )
{
    QString s = Prefix;
    QString sDir = ExtractFileDir ( s );
    if ( sDir != "" )
    {
        if ( sDir[ sDir.size() - 1 ] != '/' )
            sDir += '/';
    }
    QDateTime kdt = QDateTime::currentDateTime().addDays( -KeepDays );

    QDirIterator files( sDir, QDir::Files | QDir::NoSymLinks , QDirIterator::NoIteratorFlags );
    while ( files.hasNext() )
    {
        files.next();
        QFileInfo fi(files.filePath());
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
        QDateTime fd(fi.created());
#else
        QDateTime fd(fi.birthTime());
#endif

        if (fd < kdt)
        {
            if (!QFile::remove(files.filePath()))
            {
                trace("Failed to remove " + files.filePath());
            }
        }
    }
}
