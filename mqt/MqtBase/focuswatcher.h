#ifndef FOCUSWATCHER
#define FOCUSWATCHER

#include <QObject>
#include <QEvent>
#include <QFocusEvent>

class FocusWatcher : public QObject
{
   Q_OBJECT
public:
   explicit FocusWatcher(QObject* parent = nullptr) : QObject(parent)
   {
      if (parent)
         parent->installEventFilter(this);
   }
    virtual ~FocusWatcher() override
    {}
   virtual bool eventFilter(QObject *obj, QEvent *event) override
   {
      Q_UNUSED(obj)

      QFocusEvent *fEvent = dynamic_cast<QFocusEvent *>(event);

      if (event->type() == QEvent::FocusIn)
         emit focusChanged(obj, true, fEvent);
      else if (event->type() == QEvent::FocusOut)
         emit focusChanged(obj, false, fEvent);

      return false;
   }

Q_SIGNALS:
   void focusChanged(QObject *obj, bool in, QFocusEvent *event);
};
#endif // FOCUSWATCHER

