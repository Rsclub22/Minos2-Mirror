/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2006 - 2008
//
/////////////////////////////////////////////////////////////////////////////
#ifndef ControlPortH
#define ControlPortH 
//==============================================================================
#include <QString>
#include <QVector>
#include <QWidget>
class commonPort;
class LineConfig;
class PortConfig;
class commonController;

class PiGPIO;
class commonLineControl
{
      bool state;
      bool lastState;
   public:
      QString lineName;
      QString portLineName;
      commonPort *port;
      bool lineIn;
      commonLineControl( QString name, bool lineIn, QString portLine, commonPort *port );
      virtual ~commonLineControl();

      void checkLine();

      void setState( bool state );
      bool getState()
      {
         return state;
      }
};
class commonPort
{
   private:

   public:
      QString pName;
      QString portName;
      int openCount;

      commonController *monitor;

      // We will also need a structure to accept the individual lines for this port
      // Then we will have a common line change routine.
      // checkControls will have to report on all relevant line changes

      QVector<commonLineControl *> lines;
      virtual void addLine( const LineConfig &line );
      commonLineControl *findLine( const QString &name, bool lineIn );

      commonPort( const PortConfig &port );
      virtual ~commonPort();

      virtual bool initialise( const PortConfig &port, commonController &monitor );
      virtual bool initialisePort() = 0;

      virtual bool openPort() = 0;
      virtual bool closePort() = 0;

      void checkControls( );

      virtual void setLine( commonLineControl * ) = 0;
      virtual bool getLine( commonLineControl * ) = 0;
      virtual void getLineState() = 0;
      void reportLineChange( commonLineControl * );

};


//==============================================================================

// this is a particular implementation, created by factory and not generally seen

class WindowsMonitorPort: public commonPort
{
   private:
    QWidget *parent;
   public:

      WindowsMonitorPort(QWidget *p, const PortConfig &port );
      virtual ~WindowsMonitorPort() override;

      virtual bool initialisePort() override;

      virtual bool openPort() override;
      virtual bool closePort() override;

      void setLine( commonLineControl * ) override;
      bool getLine( commonLineControl * ) override;
      void getLineState() override;
};
class PiGPIOPort: public commonPort
{
   private:
      PiGPIO *pigpio;
   public:

      PiGPIOPort(const PortConfig &port );
      virtual ~PiGPIOPort() override;

      virtual bool initialisePort() override;

      virtual bool openPort() override;
      virtual bool closePort() override;
      virtual void addLine( const LineConfig &line ) override;

      void setLine( commonLineControl * ) override;
      bool getLine( commonLineControl * ) override;
      void getLineState() override;
};
#ifdef K8055PORT
class K8055Port: public commonPort
{
   private:

      int addr;
      long dig;
   public:
      K8055Port( const PortConfig &port );
      virtual ~K8055Port();

      virtual bool initialisePort();

      virtual bool openPort();
      virtual bool closePort();

      void setLine( commonLineControl * );
      bool getLine( commonLineControl * );
      void getLineState();
};
#endif
#endif
