#ifndef MINOSLINES_H
#define MINOSLINES_H

#include <QTimer>
#include <QMap>

class LineFileMapper;
class LineSet: public QObject
{
    Q_OBJECT
   private:
      static LineSet *myLineSet;

      QMap < QString, bool > LineMap;
      LineFileMapper *FileMap;

      QTimer checkTimer;

      bool readSet();
      void writeSet();

   public:
      static void ( *lsLog ) ( const QString & );
      static LineSet *GetLineSet();

      void publish( const QString &, bool );

      void readLines();
      bool getState( const QString name );

      LineSet();
      virtual ~LineSet();

      bool initialise();

signals:
      void linesChanged();

private slots:
      void checkTimerTimeout();
};
#endif // MINOSLINES_H
