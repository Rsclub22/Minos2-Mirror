#include "AppStartup.h"
#include "MShowMessageDlg.h"
#include "MTrace.h"
#include "MinosRPC.h"
#include "RPCCommandConstants.h"
#include "RPCPubSub.h"

#include "kprpcserver.h"

KPRPCServer::KPRPCServer()
{
    MinosRPC *rpc = MinosRPC::getMinosRPC(getAppStartupName());

    rpc->findProviders(rpcConstants::KeyerCategory, {});

    connect(rpc, &MinosRPC::routerCall, this, &KPRPCServer::on_routerCall);
    connect(rpc, &MinosRPC::notify, this, &KPRPCServer::on_notify);
    connect(rpc, &MinosRPC::provider, this, &KPRPCServer::on_provider);


}

KPRPCServer::~KPRPCServer()
{

}
void KPRPCServer::on_provider(Provider /*provider*/, QString /*cat*/)
{
    // We should now be subscribed
}
//---------------------------------------------------------------------------
void KPRPCServer::on_routerCall(bool err, QSharedPointer<MinosRPCObj>mro, const QString from )
{
    //trace( "Keyer callback from " + from + ( err ? ":Error" : ":Normal" ) );

//    if ( !err )
//    {
//        QSharedPointer<RPCParam> psName;
//        QSharedPointer<RPCParam>p1Value;
//        RPCArgs *args = mro->getCallArgs();
//        if ( args->getStructArgMember( 0, rpcConstants::paramName, psName ) && args->getStructArgMember( 0, rpcConstants::paramValue, p1Value ) )
//        {
//            QString commandName;
//            int Value;
//            bool nameOk = psName->getString( commandName );
//        }
//    }
}
//---------------------------------------------------------------------------
void KPRPCServer::on_notify(AnalysePubSubNotify an, const QString /*from*/ )
{
   //trace( "Notify callback from " + from + ( !an.getOK() ? ":Error" : ":Normal" ) );

   if ( an.getOK() )
   {
      if ( an.getState() == psPublished)
      {
          trace(QString("Category %1 key %2").arg(an.getCategory(), an.getKey()));
          if (an.getCategory() == rpcConstants::KeyerCategory)
          {
              if (an.getKey() == rpcConstants::keyerListen)
              {
                    QString ip = an.getPublisherIP();
                    QString port = an.getValue();
                    mShowMessage(ip + ":" + port, 0);
              }
          }

      }
      else
      {
      }
   }
}
//---------------------------------------------------------------------------
