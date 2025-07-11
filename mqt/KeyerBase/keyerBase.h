#ifndef KEYERBASE_H
#define KEYERBASE_H

#include <QString>
#include <QSharedPointer>
#include <deque>
#include <iterator>



extern QMap <int, QString> MORSECODE;    // . is 0x40, - is 0x80
class MORSEMSG
{
   public:
      MORSEMSG();
      ~MORSEMSG();
      void setMessage( const QString &msg );
      QString msg;
      bool repeat;
};
extern QMap <int, MORSEMSG> MORSEMSGS;

extern int tuneTime;
extern double tuneLevel;
extern int CWTone;
extern int CWSpeed;

//================================================================================
template < class qe >
class my_deque : public std::deque < QSharedPointer<qe> >
{
   public:
      my_deque()
      {}
      ~my_deque()
      {}

      void freeAll()
      {
          for (auto &i: *this)
          {
              i.reset();
          }
          my_deque::clear();
      }

      QSharedPointer<qe> find( const QString &s )
      {
          for (auto const &i: *this)
            if ( i ->pName.compare(s, Qt::CaseInsensitive ) == 0 )
               return i;
         return 0;
      }

      void clear_after ( QSharedPointer<qe> e )
      {
         typename my_deque::iterator i = std::find( this->begin(), this->end(), e );
         if ( i == this->end() )
            return ;
         i++;
         if ( i == this->end() )
            return ;
         for ( typename my_deque::iterator i2 = i; i2 != this->end(); i2++ )
         {
            QSharedPointer<qe> mdp = *i2;
            mdp.reset( );
         }
         my_deque::erase( i, this->end() );
      }
      void free_element ( QSharedPointer<qe> e )
      {
         if ( e )
         {
            typename my_deque::iterator i = std::find( this->begin(), this->end(), e );
            if ( i != this->end() )
            {
               my_deque::erase( i );
            }
            e.reset();
         }
      }

      QSharedPointer<qe> next_element( qe &q )
      {
         typename my_deque::iterator i;
         for (i = this->begin(); i != this->end(); i++)
         {
            QSharedPointer<qe> x = *i;
            if (x.data() == &q)
            {
               std::advance(i, 1);
               if (i == this->end())
               {
                   return QSharedPointer<qe>();
               }
               else
               {
                   return *i;
               }
            }
         }

         return QSharedPointer<qe>();
      }
};
//=============================================================================

class RecBuffer
{
   public:
      int RecBlock = -1;
      int WriteBlock = -1;

      char buff[40000] = {}; // should be same size as sound buffers
      int Size = 0;
      bool filled = false;
      RecBuffer()
      {}
      void reset()
      {
         RecBlock = -1;
         WriteBlock = -1;
         Size = 0;
         filled = false;
      }
};
//=============================================================================
class KeyerAction
{
   protected:
//      qint64 startTick;
      qint64 lastTick;
   public:
      unsigned int pipStartDelaySamples = 0;
      bool tailWithPip = false;

      long actionTime = -1;
      bool deleteAtTick = false;
      void checkTimer();
      virtual void getActionState( QString &s ) = 0;

      virtual void LxChanged( int line, bool state ) = 0;
      virtual void pttChanged( bool state ) = 0;
      virtual void linesModeChanged(int lmode) = 0;
      virtual void queueFinished() = 0;
      virtual void timeOut() = 0;
      virtual void stopTransmit();

      KeyerAction();
      virtual ~KeyerAction();
      static my_deque < KeyerAction > currentAction;
      static QSharedPointer<KeyerAction> getCurrentAction();
      QSharedPointer<KeyerAction> getNextAction();
      virtual QString statusLetter() = 0;
      virtual bool playingFile( const QString & );
      virtual void activateVox( );
      virtual void interruptOK( ) = 0;

      virtual RecBuffer *getSourceBuffer(){return nullptr;}
      virtual void doSinkBuffer(RecBuffer *){}

      long getActionTime()
      {
         return actionTime;
      }
};

#endif // KEYERBASE_H
