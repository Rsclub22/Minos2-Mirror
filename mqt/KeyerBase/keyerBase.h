#ifndef KEYERBASE_H
#define KEYERBASE_H

#include <deque>
#include <QString>

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
class my_deque : public std::deque < qe >
{
   public:
      my_deque()
      {}
      ~my_deque()
      {}

      void freeAll()
      {
          for (auto const &i: *this)
            delete i;
          my_deque::clear();
      }

      qe find( const QString &s )
      {
          for (auto const &i: *this)
            if ( i ->pName.compare(s, Qt::CaseInsensitive ) == 0 )
               return i;
         return 0;
      }

      void clear_after ( qe e )
      {
         typename my_deque::iterator i = std::find( this->begin(), this->end(), e );
         if ( i == this->end() )
            return ;
         i++;
         if ( i == this->end() )
            return ;
         for ( typename my_deque::iterator i2 = i; i2 != this->end(); i2++ )
         {
            delete ( *i2 );
         }
         my_deque::erase( i, this->end() );
      }
      void free_element ( qe e )
      {
         if ( e )
         {
            typename my_deque::iterator i = std::find( this->begin(), this->end(), e );
            if ( i != this->end() )
            {
               my_deque::erase( i );
            }
            delete e;
         }
      }

      qe next_element( qe q )
      {
         typename my_deque::iterator i = std::find( this->begin(), this->end(), q );
         if ( i != this->end() )
         {
            i++;
            if ( i != this->end() )
               return * i;
         }
         return 0;
      }
};
//=============================================================================

class RecBuffer
{
   public:
      int RecBlock = -1;
      int WriteBlock = -1;

      char buff[ 40000 ]; // should be same size as sound buffers
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
      static my_deque < KeyerAction *> currentAction;
      static KeyerAction *getCurrentAction();
      KeyerAction *getNextAction();
      virtual char statusLetter() = 0;
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
