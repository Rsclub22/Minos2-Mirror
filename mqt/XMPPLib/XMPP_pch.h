/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#ifndef XMPP_pchH
#define XMPP_pchH

#include "mqtUtils_pch.h"

#include "PortIds.h"

#include "ServerEvent.h"
#include "XMPPEvents.h"

#include "Dispatcher.h"

#include "XMPPRPCParams.h"
#include "XMPPStanzas.h"
#include "XMPPRPCObj.h"
#include "RPCPubSub.h"

#include "RPCCommandConstants.h"
#include "MinosRPC.h"
#include "PubSubClient.h"

#include "MinosConnection.h"

template <class itemtype>
class MapWrapper
{
public:
    QSharedPointer<itemtype> wt;

    MapWrapper(){}
    MapWrapper(itemtype *mp):wt(mp){}
    MapWrapper(QSharedPointer<itemtype> mp):wt(mp){}
    MapWrapper(const MapWrapper &m)
    {
        wt = m.wt;
    }
    MapWrapper &operator= (const MapWrapper &m)
    {
        wt = m.wt;
        return *this;
    }
    operator bool()
    {
        return !wt.isNull();
    }
    bool operator!() const
    {
        return wt.isNull();
    }

    bool operator==(const MapWrapper &rhs) const
    {
        return *wt.data() == *rhs.wt.data();
    }
    bool operator!=(const MapWrapper &rhs) const
    {
        return *wt.data() != *rhs.wt.data();
    }
    bool operator<(const MapWrapper &rhs) const
    {
        return *wt.data() < *rhs.wt.data();
    }

};
template <class itemtype>
uint qHash(const MapWrapper<itemtype> &m)
{
    return m.wt->qHash();
}
#endif
