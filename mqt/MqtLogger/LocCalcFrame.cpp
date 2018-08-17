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

    S1LocFW = new FocusWatcher(ui->S1Loc);
    ui->S1Loc->setValidator(new UpperCaseValidator(true));
    ui->S1Loc->installEventFilter(this);
    connect(S1LocFW, SIGNAL(focusChanged(QObject *, bool, QFocusEvent * )), this, SLOT(focusChange(QObject *, bool, QFocusEvent *)));

    S1LatLongFW = new FocusWatcher(ui->S1LatLong);
    ui->S1LatLong->installEventFilter(this);
    connect(S1LatLongFW, SIGNAL(focusChanged(QObject *, bool, QFocusEvent * )), this, SLOT(focusChange(QObject *, bool, QFocusEvent *)));

    S1NGRFW = new FocusWatcher(ui->S1NGR);
    ui->S1NGR->installEventFilter(this);
    connect(S1NGRFW, SIGNAL(focusChanged(QObject *, bool, QFocusEvent * )), this, SLOT(focusChange(QObject *, bool, QFocusEvent *)));

    S2LocFW = new FocusWatcher(ui->S2Loc);
    ui->S2Loc->setValidator(new UpperCaseValidator(true));
    ui->S2Loc->installEventFilter(this);
    connect(S2LocFW, SIGNAL(focusChanged(QObject *, bool, QFocusEvent * )), this, SLOT(focusChange(QObject *, bool, QFocusEvent *)));

    S2LatLongFW = new FocusWatcher(ui->S2LatLong);
    ui->S2LatLong->installEventFilter(this);
    connect(S2LatLongFW, SIGNAL(focusChanged(QObject *, bool, QFocusEvent * )), this, SLOT(focusChange(QObject *, bool, QFocusEvent *)));

    S2NGRFW = new FocusWatcher(ui->S2NGR);
    ui->S2NGR->installEventFilter(this);
    connect(S2NGRFW, SIGNAL(focusChanged(QObject *, bool, QFocusEvent * )), this, SLOT(focusChange(QObject *, bool, QFocusEvent *)));

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
    ui->S1Loc->setText(S1Loc);
    ui->S2Loc->setText(S2Loc);
    on_CalcButton_clicked();
}

void LocCalcFrame::on_CalcButton_clicked()
{
    // Now calculate distance to the current contest locator

    double dist = 0.0;
    int brg;
    double longitude = 0.0;
    double latitude = 0.0;

    if (handleExit( ui->S1Loc ) && handleExit( ui->S2Loc ) )
    {
       BaseContestLog cnt;
       cnt.myloc.loc.setValue( ui->S1Loc->text().toUpper() );
       cnt.validateLoc();

       if ( lonlat( ui->S2Loc->text().toUpper(), longitude, latitude ) == LOC_OK )
       {
          cnt.disbeara( longitude, latitude, dist, brg );
          int idist = static_cast<int>(dist);
          Distance = QString::number( idist );
          ui->Distance->setText(QString( "Dist " ) + Distance + " km " + QString::number(brg) + " degrees");
       }
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

void LocCalcFrame::focusChange(QObject *obj, bool fIn, QFocusEvent * /*event*/)
{
    if (!fIn)
    {
        QLineEdit *Edit = dynamic_cast<QLineEdit *>(obj);
        if (Edit)
        {
            handleExit(Edit);
        }
    }
}

bool LocCalcFrame::handleExit( QLineEdit *Edit )
{
   Location l1, l2;
   gridstyle gstyle = NUL;

   QLineEdit *locIl;
   QLineEdit *geoIl;
   QLineEdit *ngrIl;

   if (Edit == ui->S1Loc)
   {
         gstyle = LOC;
         locIl = ui->S1Loc;
         geoIl = ui->S1LatLong;
         ngrIl = ui->S1NGR;
   }
   else if (Edit == ui->S1LatLong)
   {
         gstyle = GEO;
         locIl = ui->S1Loc;
         geoIl = ui->S1LatLong;
         ngrIl = ui->S1NGR;
   }
   else if (Edit == ui->S1NGR)
   {
         gstyle = NGR;
         l1.centremeridian = degrad( -2.0 );
         locIl = ui->S1Loc;
         geoIl = ui->S1LatLong;
         ngrIl = ui->S1NGR;
   }
   else if (Edit == ui->S2Loc)
   {
         gstyle = LOC;
         locIl = ui->S2Loc;
         geoIl = ui->S2LatLong;
         ngrIl = ui->S2NGR;
   }
   else if (Edit == ui->S2LatLong)
   {
         gstyle = GEO;
         locIl = ui->S2Loc;
         geoIl = ui->S2LatLong;
         ngrIl = ui->S2NGR;
   }
   else if (Edit == ui->S2NGR)
   {
         gstyle = NGR;
         locIl = ui->S2Loc;
         geoIl = ui->S2LatLong;
         ngrIl = ui->S2NGR;
         l1.centremeridian = degrad( -2.0 );
   }
   else
   {
       return false;
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
