#include "base_pch.h"
#include "MinosLoggerEvents.h"
#include "contest.h"
#include "cutils.h"
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


   ui->clockLabel ->setText("<b><center><nobr><p><big><h1>"
                            + colour + disp + "</p></h1>");
}
