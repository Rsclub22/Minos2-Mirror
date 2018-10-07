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

static QSharedMemory ServerEvent;

void makeServerEvent( bool create )
{
    ServerEvent.setKey( "MinosQtServer" );
    if (create)
    {
        ServerEvent.create( 1 );

    }
    else
    {
        ServerEvent.detach();
    }
}

bool checkServerReady()
{
    QSharedMemory mem( "MinosQtServer" );
    if ( mem.attach() )
    {
        mem.detach();
        return true;
    }
    return false;
}

