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

#ifndef XMPPRPCObjH
#define XMPPRPCObjH 
//---------------------------------------------------------------------------
#include <string>
#include <QString>
#include <QSharedPointer>
#include "PublishState.h"
#include "XMPPRPCParams.h"
#include "PubSubValue.h"
#include "PublishState.h"
//---------------------------------------------------------------------------
class XStanza;
typedef void ( *sendActionCall ) ( XStanza *a  );
extern sendActionCall sendAction;
void setSendAction(sendActionCall sa);

// We need a factory for MinosRPCObj descendents, so that we can register those we
// are interested in, and when a call is received those we know are manufactured
// but those we don't know are generically rejected

class MinosRPCObj;

// abstract base class
class TRPCFunctor
{
   public:

        virtual ~TRPCFunctor();

      // two possible functions to call member function. virtual cause derived
      // classes will use a pointer to an object and a pointer to a member function
      // to make the function call
      virtual void call( bool err, QSharedPointer<MinosRPCObj>mro, const QString &from ) = 0;        // call using function
};


// derived template class
template <class TClass>
class TRPCCallback : public TRPCFunctor
{
   private:
      void ( TClass::*fpt ) ( bool err, QSharedPointer<MinosRPCObj>mro, const QString &from );   // pointer to member function
      TClass* pt2Object = nullptr;                  // pointer to object

   public:

      // constructor - takes pointer to an object and pointer to a member and stores
      // them in two private variables
      TRPCCallback( TClass* _pt2Object, void( TClass::*_fpt ) ( bool err, QSharedPointer<MinosRPCObj>mro, const QString &from ) )
      {
         pt2Object = _pt2Object;
         fpt = _fpt;
      }

      // override function "Call"
      virtual void call( bool err, QSharedPointer<MinosRPCObj>mro, const QString &from ) override
      {
         ( *pt2Object.*fpt ) ( err, mro, from );
      } // execute member function
};


class MinosRPCClient;
class MinosRPCServer;
class MinosRPCObj
{
      // Base object for Minos RPC calls. It encapsulates both ends of
      // the call
   private:
      bool general;

   protected:
      QString methodName;
      RPCArgs callArgs;

   public:
      RPCArgs *getCallArgs()
      {
         return & callArgs;
      }
      void setCallArgs( RPCArgs &r )
      {
         callArgs = r;
      }
      void clearCallArgs();

      virtual bool isGeneralObject()
      {
          return general;
      }

      virtual QString getMethodName()
      {
          return methodName;
      }

      QString id;
      TRPCFunctor *callback;

      MinosRPCObj( const QString &methodName, TRPCFunctor *cb, bool gen = false );
      virtual ~MinosRPCObj();
      static void clearRPCObjects();

      virtual QSharedPointer<MinosRPCObj>makeObj() = 0;

      static QSharedPointer<MinosRPCObj> makeServerObj( QString call );

      static void addServerObj( QSharedPointer<MinosRPCObj> mro );

      virtual void queueCall( QString /* to*/ )
      {}

};


// I think we need master and slave ends...  how do we do this?
// Do we go for mixins, one for each type?

class MinosRPCClient: public MinosRPCObj
{
   private:
      MinosRPCClient();            // don't allow default constructor
   public:
      MinosRPCClient( const QString &mName, TRPCFunctor *cb, bool gen = false ) : MinosRPCObj( mName, cb, gen )
      {}
      virtual ~MinosRPCClient() override
      {}

      virtual QSharedPointer<MinosRPCObj>makeObj() override = 0;

      // call queueCall to make initial call;
      // This will construct a BlockingRequest to allow the eventual callback
      // actually, NOT a Blocking Request, but something similar
      // we don't wait on it, but we do queue it, and on response arriving
      // we queue the desired response

      virtual void queueCall( QString to ) override;
      void queueCall(const PubSubName &psn);

};
class MinosRPCServer: public MinosRPCObj
{
   private:
      MinosRPCServer();            // don't allow default constructor
   public:
      MinosRPCServer( const QString &mName, TRPCFunctor *cb, bool gen = false ) : MinosRPCObj( mName, cb, gen )
      {}
      virtual ~MinosRPCServer() override;

      virtual QSharedPointer<MinosRPCObj>makeObj() override = 0;
};

#endif
