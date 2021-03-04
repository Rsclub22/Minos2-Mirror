#include "AnalysePubSubNotify.h"
#include "ConfigFile.h"

AnalysePubSubNotify::AnalysePubSubNotify(bool err, QSharedPointer<MinosRPCObj> mro ) :
      OK( false )
{
    qRegisterMetaType< AnalysePubSubNotify > ( "AnalysePubSubNotify" );

   if ( !err )
   {
      QSharedPointer<RPCParam> psPublisher;
      QSharedPointer<RPCParam> psRouter;
      QSharedPointer<RPCParam> psCategory;
      QSharedPointer<RPCParam> psKey;
      QSharedPointer<RPCParam> psValue;
      QSharedPointer<RPCParam> psState;
      RPCArgs *args = mro->getCallArgs();
      if (
              // NB publisher is initiated by the "from" from the publisher
              // and persists with the publihe entity
         args->getStructArgMember( 0, "Publisher", psPublisher )
         && args->getStructArgMember( 0, "Server", psRouter )
         && args->getStructArgMember( 0, "Category", psCategory )
         && args->getStructArgMember( 0, "Key", psKey )
         && args->getStructArgMember( 0, "Value", psValue )
         && args->getStructArgMember( 0, "State", psState )
      )
      {
         int stemp;
         QString pub;
         if (
            psPublisher->getString(pub) &&
            psRouter->getString( router ) &&
            psCategory->getString( category ) &&
            psKey->getString( key ) &&
            psValue->getString( value ) &&
            psState->getInt( stemp )
         )
         {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
            QStringList p = pub.split(QChar('@'), Qt::KeepEmptyParts);
#else
             QStringList p = pub.split(QChar('@'), QString::KeepEmptyParts);
#endif
            if (p.size() > 1)
            {
                publisherRouter = p[1];
            }
            if (publisherRouter == "localhost")
            {
                publisherRouter = MinosConfig::getMinosConfig()->getThisRouterName();
            }
            publisherProgram = p[0];
            state = static_cast<PublishState>(stemp);
            OK = true;
         }
      }
   }
}
AnalysePubSubNotify::AnalysePubSubNotify()
{}
