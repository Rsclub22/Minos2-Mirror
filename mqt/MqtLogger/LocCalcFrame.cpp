#include "base_pch.h"
#include "contest.h"
#include "latlong.h"
#include "cutils.h"
#include "LocCalcFrame.h"
#include "ui_LocCalcFrame.h"

LocCalcFrame::LocCalcFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::LocCalcFrame),
    contest(nullptr)
{
    ui->setupUi(this);

    ui->S1Grid->setValidator(&ucValidator);
    ui->S2Grid->setValidator(&ucValidator);
}

LocCalcFrame::~LocCalcFrame()
{
    delete ui;
}
void LocCalcFrame::setContest(BaseContestLog *ct)
{
    contest = ct;
}

void LocCalcFrame::doExec()
{
    ui->S1Grid->setText(S1Loc);
    ui->S2Grid->setText(S2Loc);

    ui->S1LocRb->setChecked(true);
    ui->S2LocRb->setChecked(true);

    on_CalcButton_clicked();
}
void LocCalcFrame::on_S1Calc_clicked()
{
    handleExit(ui->S1Grid);
}

void LocCalcFrame::on_S2Calc_clicked()
{
    handleExit(ui->S2Grid);
}

void LocCalcFrame::on_CalcButton_clicked()
{
    on_S1Calc_clicked();
    on_S2Calc_clicked();
    // Now calculate distance to the current contest locator

    double dist = 0.0;
    int brg;
    double longitude = 0.0;
    double latitude = 0.0;

    BaseContestLog cnt;
    cnt.myloc = Locator( ui->S1Loc->text().toUpper() );

    if ( lonlat( ui->S2Loc->text().toUpper(), longitude, latitude, MinosParameters::getMinosParameters() ->getAllowLoc4() ) == LOC_OK )
    {
        cnt.disbeara( longitude, latitude, dist, brg );
        int idist = static_cast<int>(dist);
        Distance = QString::number( idist );
        ui->Distance->setText(tr( "Dist %1 km %2 degrees" ).arg(idist).arg(brg));
    }

}

void LocCalcFrame::on_ExitButton_clicked()
{
    if ( ui->Distance->text().trimmed().length() == 0 )
    {
       on_CalcButton_clicked();
    }
    dynamic_cast<QDialog *>(parent())->accept();
}

void LocCalcFrame::on_CancelButton_clicked()
{
    dynamic_cast<QDialog *>(parent())->reject();
}

bool LocCalcFrame::handleExit( QLineEdit *Edit )
{
   Location l1, l2;
   gridstyle gstyle = NUL;

   QLineEdit *locIl = nullptr;
   QLineEdit *geoIl = nullptr;
   QLineEdit *ngrIl = nullptr;

   if (Edit == ui->S1Grid)
   {
       if (ui->S1LocRb->isChecked())
       {
             gstyle = LOC;
             locIl = ui->S1Loc;
             geoIl = ui->S1LatLong;
             ngrIl = ui->S1NGR;
             ui->S1Loc->setText(ui->S1Grid->text());
       }
       else if (ui->S1LatLongRb->isChecked())
       {
             gstyle = GEO;
             locIl = ui->S1Loc;
             geoIl = ui->S1LatLong;
             ngrIl = ui->S1NGR;
             ui->S1LatLong->setText(ui->S1Grid->text());
       }
       else if (ui->S1NGRRb->isChecked())
       {
             gstyle = NGR;
             l1.centremeridian = degrad( -2.0 );
             locIl = ui->S1Loc;
             geoIl = ui->S1LatLong;
             ngrIl = ui->S1NGR;
             ui->S1NGR->setText(ui->S1Grid->text());
       }
   }
   else if (Edit == ui->S2Grid)
   {
       if (ui->S2LocRb->isChecked())
       {
             gstyle = LOC;
             locIl = ui->S2Loc;
             geoIl = ui->S2LatLong;
             ngrIl = ui->S2NGR;
             ui->S2Loc->setText(ui->S2Grid->text());
       }
       else if (ui->S2LatLongRb->isChecked())
       {
             gstyle = GEO;
             locIl = ui->S2Loc;
             geoIl = ui->S2LatLong;
             ngrIl = ui->S2NGR;
             ui->S2LatLong->setText(ui->S2Grid->text());
       }
       else if (ui->S2NGRRb->isChecked())
       {
             gstyle = NGR;
             locIl = ui->S2Loc;
             geoIl = ui->S2LatLong;
             ngrIl = ui->S2NGR;
             l1.centremeridian = degrad( -2.0 );
             ui->S2NGR->setText(ui->S2Grid->text());
       }
   }


   l1.gridstyle = gstyle;
   l1.datastring = Edit->text().trimmed().toUpper();

   if ( gstyle != NGR )
   {
      l2.gridstyle = NGR;
      l2.centremeridian = degrad( -2.0 );

      int tok = transform( &l1, &l2 );

      ngrIl->setText( (tok == GRIDOK ) ? l2.datastring : "" );
   }

   if ( gstyle != LOC )
   {
      l2.gridstyle = LOC;

      int tok = transform( &l1, &l2 );

      locIl->setText( (tok == GRIDOK ) ? l2.datastring : "" );
   }

   if ( gstyle != GEO )
   {
      l2.gridstyle = GEO;
      int tok = transform( &l1, &l2 );

      geoIl->setText( (tok == GRIDOK ) ? l2.datastring : "" );
   }
   return true;
}

