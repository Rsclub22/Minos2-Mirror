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
//---------------------------------------------------------------------------
Exception::~Exception()
{}
void makeXMPPEvent( XStanza *xs )
{
   // analyse the stanza and produce the correct event
   if ( RPCRequest * rq = dynamic_cast<RPCRequest *>( xs ) )
   {
      // external RPC request - decode and action
        QSharedPointer<MinosRPCObj> mro = MinosRPCObj::makeRouterObj( rq->methodName );

        if (mro)
        {
            mro->setCallArgs( *rq );
            mro->id = xs->getId();

            if ( mro->callback )
            {
                mro->callback->call( false, mro, rq->getFrom() );
            }
        }
   }
}

