/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
#include <QSharedMemory>

#include "SecondInstall.h"
#include "singleapplication.h"

#ifdef Q_OS_WIN
    static QSharedMemory *RouterEvent = nullptr;
#endif
void makeRouterEvent( bool create )
{
    // we keep this so that we appear to old systems
#ifdef Q_OS_WIN
    if (!RouterEvent)
    {
        RouterEvent = new QSharedMemory();
    }
    RouterEvent->setKey( SecondInstall::getRouterEventName() );
    if (create)
    {
        RouterEvent->create( 1 );

    }
    else
    {
        RouterEvent->detach();
    }
#else
    Q_UNUSED(create)
#endif
}

bool checkRouterReady()
{
    QString pid;
    return SingleApplication::testRunning(SecondInstall::getSingleAppRouterName(), -1, pid);

}

