#include "base_pch.h"
#include "MinosLoggerEvents.h"
#include "contest.h"
#include "cutils.h"
#include "tlogcontainer.h"
#include "TClockFrame.h"
#include "ui_TClockFrame.h"

TClockFrame::TClockFrame(QWidget *parent) :
    QFrame(parent)
   ,  ui(new Ui::TClockFrame)
   , contest(nullptr)
{
    ui->setupUi(this);
    connect(&MinosLoggerEvents::mle, SIGNAL(TimerDistribution()), this, SLOT(RecheckTimerTimer()));
}

TClockFrame::~TClockFrame()
{
    delete ui;
}
void TClockFrame::setContest(BaseContestLog *c)
{
    contest = c;
}

void TClockFrame::RecheckTimerTimer(  )
{
   if ( !isVisible() )
      return ;

   QDateTime t = QDateTime::currentDateTimeUtc().addSecs( MinosParameters::getMinosParameters() ->getBigClockCorrection());
   QString disp = t.toString( "HH:mm:ss" );
   QString t1 = contest->DTGStart.getValue();
   QDateTime start = CanonicalToTDT( t1 );
   t1 = contest->DTGEnd.getValue();
   QDateTime end = CanonicalToTDT( t1 );

   QString sStart = start.toString("dd/MM/yyyy HH:mm");
   QString sEnd = end.toString("dd/MM/yyyy HH:mm");

   bool timeOK = false;
   if (contest)
   {
        timeOK = contest->checkTime(t);
   }
   QString colour;

   if (!timeOK)
       colour = HtmlFontColour(Qt::red);
   else
       colour = HtmlFontColour(Qt::blue);

   QString sGray = HtmlFontColour(Qt::gray);


   QString mess = "<b>" + sGray + "<center><nobr>" + sStart + "<b><nobr><big><h1>"
           + colour + disp + "</h1></big></b/<nobr>" + sGray + sEnd;

   //mShowMessage(mess, this);
   ui->clockLabel ->setText(mess);
}
