/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
#include "XMPP_pch.h"
#include <dirent.h>
#include <fstream>

#ifdef Q_OS_WIN
    static QSharedMemory ServerEvent;
#endif
void makeServerEvent( bool create )
{
#ifdef Q_OS_WIN
    ServerEvent.setKey( "MinosQtServer" );
    if (create)
    {
        ServerEvent.create( 1 );

    }
    else
    {
        ServerEvent.detach();
    }
#endif
}
#ifndef Q_OS_WIN

static int getProcIdByName(QString procName)
{
    int pid = -1;

    // Open the /proc directory
    DIR *dp = opendir("/proc");
    if (dp != nullptr)
    {
        // Enumerate all entries in directory until process found
        struct dirent *dirp;
        while (pid < 0 && (dirp = readdir(dp)))
        {
            // Skip non-numeric entries
            int id = atoi(dirp->d_name);
            if (id > 0)
            {
                // Read contents of virtual /proc/{pid}/cmdline file
                QString cmdPath = QString("/proc/") + dirp->d_name + "/cmdline";

                QFile data(cmdPath);
                if (data.open(QFile::ReadOnly))
                {
                    QTextStream in(&data);
                    QString cmdLine = in.readAll();

                    if (!cmdLine.isEmpty())
                    {
                        // Keep first cmdline item which contains the program path
                        int pos = cmdLine.indexOf('\0');
                        if (pos != -1)
                            cmdLine = cmdLine.left(pos);
                        // Keep program name only, removing the path
                        pos = cmdLine.lastIndexOf('/');
                        if (pos != -1)
                            cmdLine = cmdLine.mid(pos + 1);
                        // Compare against requested process name
                        if (procName == cmdLine)
                            pid = id;
                    }
                }
            }
        }
    }

    closedir(dp);

    return pid;
}
#endif

bool checkServerReady()
{
#ifdef Q_OS_WIN
    QSharedMemory mem( "MinosQtServer" );
    if ( mem.attach() )
    {
        mem.detach();
        return true;
    }
#else
    // on Linux, the shared memory can stick around when the process crashes
    int pid = getProcIdByName("MqtServer");
    if (pid > 0)
    {
        int err = kill(pid, 0);
        if (err == 0 || err == EPERM)
        {
            return 0;
        }
    }
#endif
    return false;
}

