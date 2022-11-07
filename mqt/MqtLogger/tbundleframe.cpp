#include "contestdetails.h"
#include "tsettingseditdlg.h"
#include "profiles.h"
#include "tbundleframe.h"
#include "ui_tbundleframe.h"

TBundleFrame::TBundleFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::TBundleFrame)
  , cd(nullptr)
  , suppressChange(false)
  , bundle( nullptr )
  , bname( nullptr )
{
    ui->setupUi(this);
}

TBundleFrame::~TBundleFrame()
{
    delete ui;
}

void TBundleFrame::initialise()
{
    suppressChange = true;
   QString csect = bundle->getSection();
   QStringList slist = bundle->getSections( );
   ui->BundleSection->clear();

   int offset = -1;
   for ( int i = 0; i < slist.count(); i++ )
   {
      if ( csect == slist[ i ] )
      {
         offset = i;
      }
      ui->BundleSection->addItem( slist[ i ] );
   }
   if ( offset >= 0 )
   {
      ui->BundleSection->setCurrentIndex( offset);
   }
   else
   {
      ui->BundleSection->setCurrentIndex( 0);
   }
   suppressChange = false;
}
void TBundleFrame::initialise(ContestDetails *cdp, const QString &cap, SettingsBundle *b, MinosStringItem<QString> *name )
{
    cd = cdp;
   if ( b )
   {
      bundle = b;
      bname = name;
      // here we need to read in the histories
      ui->BundleNameLabel->setText(cap);
      initialise();
   }
}
//---------------------------------------------------------------------------
bool TBundleFrame::doEdit( )
{
   TSettingsEditDlg ed ( this, bundle );
   if ( ed.exec() == QDialog::Accepted )
   {
      initialise();
      return true;
   }
   return false;
}
//---------------------------------------------------------------------------
void TBundleFrame::enableBundle( bool state )
{
   ui->BundleNameLabel->setEnabled(state);
   ui->BundleSection->setEnabled(state);
   ui->BundleEdit->setEnabled(state);
}
//---------------------------------------------------------------------------

void TBundleFrame::on_BundleEdit_clicked()
{
    bool ok = doEdit();

    // And we need to call back to the contest detail form

    if (ok && cd)
    {
        cd->bundleChanged();
    }
}

void TBundleFrame::on_BundleSection_currentIndexChanged(const QString &/*arg1*/)
{
    if ( bundle )
    {
       bundle->openSection( ui->BundleSection->currentText() );
       bname->setValue( bundle->getSection() );
    }
    if (cd && !suppressChange)
    {
        cd->bundleChanged();
    }

}
