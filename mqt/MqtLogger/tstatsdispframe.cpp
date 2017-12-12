#include "logger_pch.h"

#include "tstatsdispframe.h"
#include "ui_tstatsdispframe.h"

const int checkInterval = 10;
TStatsDispFrame::TStatsDispFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::TStatsDispFrame),
    ct(0), lastCheckTime(QDateTime::currentDateTime().addSecs(-1000)),
    suppressPeriodChange(false)

{
    ui->setupUi(this);
    connect(&MinosLoggerEvents::mle, SIGNAL(TimerDistribution()), this, SLOT(RecheckTimerTimer()));
}

TStatsDispFrame::~TStatsDispFrame()
{
    delete ui;
    ct = nullptr;
}
void TStatsDispFrame::setContest( LoggerContestLog *pct )
{
   ct = pct;
}
void TStatsDispFrame::RecheckTimerTimer(  )
{
   if ( !isVisible() || !ct )
      return ;

   QDateTime now = QDateTime::currentDateTime();

   if (lastCheckTime.secsTo(now) >= checkInterval)
   {
       lastCheckTime = now;
       reInitialiseStats();
   }
}

void TStatsDispFrame::on_P1Edit_valueChanged(int arg1)
{
   if (!suppressPeriodChange && ct)
   {
      ct->statsPeriod1.setValue( arg1 );
      ct->commonSave(false);
      reInitialiseStats();
   }
}

void TStatsDispFrame::on_P2Edit_valueChanged(int arg1)
{
   if (!suppressPeriodChange && ct)
   {
       ct->statsPeriod2.setValue( arg1 );
       ct->commonSave(false);
      reInitialiseStats();
   }
}
//---------------------------------------------------------------------------
void TStatsDispFrame::reInitialiseStats()
{
   // we need to set the operator bundle to the current contest ops
    ui->SLabel1->clear();

    if ( !ct )
    {
       // not there...
       return ;
    }

   if ( !ui->P1Edit->hasFocus() && !ui->P2Edit->hasFocus() )
   {
      suppressPeriodChange = true;
      ui->P1Edit->setValue( ct->statsPeriod1.getValue() );
      ui->P2Edit->setValue( ct->statsPeriod2.getValue() );
      suppressPeriodChange = false;
   }


   QSharedPointer<BaseContact> bestdx = ct->getBestDX();

   if ( !bestdx || ct->maxSerial <= 0 )
      return ;

   int ltot = 0;
   if ( ct->countryMult.getValue() )
      ltot += ct->nctry ;
   if ( ct->districtMult.getValue() )
      ltot += ct->ndistrict;
   if ( ct->locMult.getValue() )
      ltot += ct->nlocs;

   ct->updateStats(ct->statsPeriod1.getValue() * 60, ct->statsPeriod2.getValue() * 60);
   int nvalid = ct->getValidQSOs();
   if (nvalid)
   {
      double avpts = static_cast< double>(ct->contestScore) / nvalid;
      QString tempcs;
      tempcs = bestdx->cs.fullCall.getValue().trimmed() ;

      QString buff = QString( "Best DX %1 Sn %2 %3(%4 kms). Average QSO is %5 Points" )
                           //#define MAX_DISPLAY_TEST 1
   #ifndef MAX_DISPLAY_TEST
                           .arg( tempcs) .arg(bestdx->serials.getValue())
                           .arg(bestdx->loc.loc.getValue()) .arg( bestdx->contactScore.getValue())
                           .arg(avpts, 0, 'f', 1);
   #else
                           .arg("PE0/LX5ABC/P") .arg("9999") .arg( "XX99XX") .arg(29999) .arg(9999.9);
   #endif

      int sp1 = ct->statsPeriod1.getValue();
      int sp2 = ct->statsPeriod2.getValue();
      QDateTime  contestStart = CanonicalToTDT(ct->DTGStart.getValue());
      QDateTime now = QDateTime::currentDateTimeUtc().addSecs( MinosParameters::getMinosParameters() ->getBigClockCorrection());
      int fromContestStart = contestStart.secsTo(now);

      // if period is less than half the time from the start
      // then we want to split the time from start

      if (sp1 > fromContestStart/120) // fromContestStart is seconds
      {
         sp1 = fromContestStart/120; // so half that, in minutes
      }
      if (sp2 > fromContestStart/120)
      {
         sp2 = fromContestStart/120;
      }
      if ( ltot )
      {
         double qmult = static_cast<double> (nvalid) / ltot;
         QString temp;

         long Qs = ct->QSO1 ? ct->QSO1 : ct->QSO2;
         if ( Qs )
         {
            // Why the selection of period in this way?  kms1 is the distance for the first period; use that unless it is zero
            int period = ct->kms1 ? sp1 : sp2;
            double qmins = ( qmult * period ) / Qs;
            temp = QString(" (%1 Mins)" ) .arg(qmins, 0, 'f', 1) ;
         }

         QString lbuff = QString( "%1\nOne New Multiplier is equivalent to %2 Average QSOs%3" ) .arg(buff)
                               .arg(qmult, 0, 'f', 1) .arg(temp);
         buff = lbuff ;
      }

      QString lbuff;

      if (ct->usesBonus.getValue())
      {
          lbuff = QString( "Last %1 Mins: %2 QSO %3 pts %4 bonus\r\n(Previous %5; %6; %7)"
                               "\nLast %8 Mins: %9 QSO %10 pts %11 bonus\r\n(Previous %12; %13; %14)" )
                              .arg(sp1)  .arg(ct->QSO1) .arg(ct->kms1) .arg(ct->bonus1)
                              .arg(ct->QSO1p) .arg(ct->kms1p) .arg(ct->bonus1p)
                              .arg(sp2) .arg(ct->QSO2) .arg(ct->kms2) .arg(ct->bonus2)
                              .arg(ct->QSO2p) .arg(ct->kms2p) .arg(ct->bonus2p);
      }
      else
      {
        lbuff = QString( "Last %1 Mins: %2 QSO %3 pts %4 mults\r\n(Previous %5; %6; %7)"
                             "\nLast %8 Mins: %9 QSO %10 pts %11 mults\r\n(Previous %12; %13; %14)" )
                            .arg(sp1)  .arg(ct->QSO1) .arg(ct->kms1) .arg(ct->mults1)
                            .arg(ct->QSO1p) .arg(ct->kms1p) .arg(ct->mults1p)
                            .arg(sp2) .arg(ct->QSO2) .arg(ct->kms2) .arg(ct->mults2)
                            .arg(ct->QSO2p) .arg(ct->kms2p) .arg(ct->mults2p);
      }
      ui->SLabel1->setText( buff + "\r\n\r\n" + lbuff );
   }
}
//---------------------------------------------------------------------------

