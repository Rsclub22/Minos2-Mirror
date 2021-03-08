#ifndef ANALYSEPUBSUBNOTIFY_H
#define ANALYSEPUBSUBNOTIFY_H
#include <QString>
#include <QSharedPointer>
#include "PublishState.h"
#include "XMPPRPCObj.h"

class MinosRPCObj;
class AnalysePubSubNotify
{
      QString publisherProgram;
      QString publisherRouter;
      QString router;
      QString category;
      QString key;
      QString value;
      PublishState state;

      bool OK = false;
   public:
      AnalysePubSubNotify( bool err, QSharedPointer<MinosRPCObj>mro );
      AnalysePubSubNotify();
      QString getPublisherProgram() const
      {
         return publisherProgram;
      }
      QString getPublisherRouter() const
      {
         return publisherRouter;
      }
      QString getRouter() const
      {
         return router;
      }
      QString getCategory() const
      {
         return category;
      }
      QString getKey() const
      {
         return key;
      }
      QString getValue() const
      {
         return value;
      }
      PublishState getState() const
      {
         return state;
      }
      bool getOK() const
      {
         return OK;
      }
};
#endif // ANALYSEPUBSUBNOTIFY_H
