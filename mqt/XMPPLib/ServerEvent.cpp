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
#include <QUuid>

static QSharedMemory ServerEvent;

QString makeUuid()
{
    QUuid uuid = QUuid::createUuid();
    return uuid.toString();
}
QString getServerId()
{
   const QString key = "ServerUUID";

   QSettings settings("Minos2Qt", "MinosQtServerUUID");    // override the app settings

   QString uuid = settings.value(key, "").toString();
   if (uuid.size() == 0)
   {
       uuid = makeUuid();
       settings.setValue(key, uuid);
   }
   return uuid;

}

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

