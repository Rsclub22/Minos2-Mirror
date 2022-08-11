/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------

#ifndef CommonMonitorH
#define CommonMonitorH 
//---------------------------------------------------------------------------

#include <QTimer>
#include <QVector>

class commonController;
class commonPort;
class commonLineControl;
class LineConfig;
class PortConfig;

void setLines(bool PTTOut, bool PTTIn, bool L1, bool L2 , bool L3, bool L4, bool L5, bool L6, bool T1, bool T2);

class timerTicker: public QObject
{
    Q_OBJECT
      QTimer b;
   public:
      timerTicker();
      ~timerTicker() override;
      virtual void tickEvent() = 0;       // this will often be an interrupt routine

private slots:
      void tick()
      {
          tickEvent();
      }
};

class commonController: public timerTicker
{
    QWidget *parent;
   public:
      commonController(QWidget *p);
      ~commonController() override;
      QVector < commonPort *> portChain;
      commonPort *createPort( const PortConfig &port );
      commonLineControl *findLine(const QString &name, bool lineIn );

      bool initialise( );
      void closeDown();

      virtual void tickEvent() override;       // this will often be an interrupt routine
      void checkControls( );

      void lineChange( commonLineControl *line );
};
#endif
