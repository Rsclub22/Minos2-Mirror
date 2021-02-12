#include "base_pch.h"

#include "LoggerContest.h"
#include "LoggerContacts.h"

#include "ContestApp.h"
#include "Calendar.h"
#include "CalendarList.h"
#include "BandList.h"
#include "tentryoptionsform.h"
#include "tminosbshelpform.h"
#include "tcalendarform.h"
#include "tlogcontainer.h"
#include "SendRPCDM.h"
#include "ScreenConfigFile.h"

#include "contestdetails.h"
#include "ui_contestdetails.h"

ContestDetails::ContestDetails(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ContestDetails),
    inputcontest(nullptr),
    saveContestOK(false), suppressProtectedOnClick(false),
    noMultRipple(false)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    ui->setupUi(this);

    QSettings settings;
    QByteArray geometry = settings.value("ContestDetails/geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    ui->ExchangeComboBox->addItem(tr("No Exchange Required"));
    ui->ExchangeComboBox->addItem(tr("PostCode Multipliers"));
    ui->ExchangeComboBox->addItem(tr("Other Exchange Multiplier"));
    ui->ExchangeComboBox->addItem(tr("Optional Exchange Multiplier"));
    ui->ExchangeComboBox->addItem(tr("Exchange Required (no multiplier)"));

    ui->BonusComboBox->addItem(tr("None"));
    ui->BonusComboBox->addItem(tr("UKAC Bonuses (B2)"));
    ui->BonusComboBox->addItem(tr("UKAC Bonuses (B4)"));
    ui->BonusComboBox->addItem(tr("NAC Bonuses"));

    ui->ModeComboBox->addItem(hamlibData::CW);
    ui->ModeComboBox->addItem(hamlibData::USB);
    ui->ModeComboBox->addItem(hamlibData::FM);
    ui->ModeComboBox->addItem(hamlibData::MGM);

    for ( int i = 0; i < 24; i++ )
    {
        QString cbText = QString("%1:").arg(i, 2, 10, QChar('0'));
        QString hour = cbText + "00 UTC";
        QString halfhour = cbText + "30 UTC";
        QString fiftyfive = cbText + "55 UTC";
        ui->StartTimeCombo->addItem( hour );
        ui->StartTimeCombo->addItem ( halfhour );
        ui->EndTimeCombo->addItem( hour );
        ui->EndTimeCombo->addItem ( halfhour );
        ui->EndTimeCombo->addItem ( fiftyfive );
    }

    ui->StartDateEdit->setDate(QDate::currentDate());
    ui->EndDateEdit->setDate(QDate::currentDate());
    ui->CallsignEdit->setValidator(&ucValidator);
    ui->LocatorEdit->setValidator(&ucValidator);
    ui->MainOpComboBox->setValidator(&ucValidator);
    ui->SecondOpComboBox->setValidator(&ucValidator);

    ContestNameEditFW = new FocusWatcher(ui->ContestNameEdit);
    ui->ContestNameEdit->installEventFilter(this);
    BandComboBoxFW = new FocusWatcher(ui->BandComboBox);
    ui->BandComboBox->installEventFilter(this);
    CallsignEditFW = new FocusWatcher(ui->CallsignEdit);
    ui->CallsignEdit->installEventFilter(this);
    LocatorEditFW = new FocusWatcher(ui->LocatorEdit);
    ui->LocatorEdit->installEventFilter(this);
    PowerEditFW = new FocusWatcher(ui->PowerEdit);
    ui->PowerEdit->installEventFilter(this);
    MainOpComboBoxFW = new FocusWatcher(ui->MainOpComboBox);
    ui->MainOpComboBox->installEventFilter(this);

    connect(ContestNameEditFW, SIGNAL(focusChanged(QObject *, bool, QFocusEvent * )), this, SLOT(focusChange(QObject *, bool, QFocusEvent *)));
    connect(BandComboBoxFW, SIGNAL(focusChanged(QObject *, bool, QFocusEvent * )), this, SLOT(focusChange(QObject *, bool, QFocusEvent *)));
    connect(CallsignEditFW, SIGNAL(focusChanged(QObject *, bool, QFocusEvent * )), this, SLOT(focusChange(QObject *, bool, QFocusEvent *)));
    connect(LocatorEditFW, SIGNAL(focusChanged(QObject *, bool, QFocusEvent * )), this, SLOT(focusChange(QObject *, bool, QFocusEvent *)));
    connect(PowerEditFW, SIGNAL(focusChanged(QObject *, bool, QFocusEvent * )), this, SLOT(focusChange(QObject *, bool, QFocusEvent *)));
    connect(MainOpComboBoxFW, SIGNAL(focusChanged(QObject *, bool, QFocusEvent * )), this, SLOT(focusChange(QObject *, bool, QFocusEvent *)));

    connect(LogContainer->sendDM, SIGNAL(setRadioList()), this, SLOT(on_SetRadioList()));
    connect(LogContainer->sendDM, SIGNAL(RotatorList()), this, SLOT(on_RotatorList()));

    ui->NonGCtryMult->setVisible(false);
    ui->GLocMult->setVisible(false);
    ui->M7LocatorMults->setVisible(false);
}
void ContestDetails::doCloseEvent()
{
    QSettings settings;
    settings.setValue("ContestDetails/geometry", saveGeometry());
}
void ContestDetails::reject()
{
    doCloseEvent();
    QDialog::reject();
}
void ContestDetails::accept()
{
    doCloseEvent();
    QDialog::accept();
}

int ContestDetails::exec()
{
    // If someone has created them by hand, make sure we reread them1
    contestTransferObject->QTHBundle.checkLoaded();
    contestTransferObject->stationBundle.checkLoaded();
    contestTransferObject->entryBundle.checkLoaded();

    ui->QTHBundleFrame->initialise( this, tr("QTH"), &contestTransferObject->QTHBundle, &contestTransferObject->QTHBundleName );
    ui->StationBundleFrame->initialise(this,  tr("Station"), &contestTransferObject->stationBundle, &contestTransferObject->stationBundleName );
    ui->EntryBundleFrame->initialise(this,  tr("Entry"), &contestTransferObject->entryBundle, &contestTransferObject->entryBundleName );
    ui->ContestNameSelected->setText(contestTransferObject->VHFContestName.getValue());

    contestTransferObject->initialiseINI();

    focusChange(nullptr, false, nullptr);    // higlight required fields
    QWidget *nextD = getDetails( );
    if ( nextD )
    {
       nextD->setFocus();
    }

    int res = QDialog::exec();

    return res;
}

ContestDetails::~ContestDetails()
{
    delete ui;
}


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
/*
      onShow - needs to produce a copy of the contest structure containing relevant
               fields
      setDetails (both) to populate display from those details

      getDetails to get from screen into copy

      Then OK Button to actually copy the detail over - then cancel
      can just discard the details
*/
void ContestDetails::setDetails( LoggerContestLog * pcont )
{
   if ( !pcont )
      return ;
   inputcontest = pcont;
   contestTransferObject = QSharedPointer<ContestDetailsTransferObject>(new ContestDetailsTransferObject());
   contestTransferObject->getFromContest(pcont);
   sectionList = contestTransferObject->sectionList.getValue(); // the combo will then be properly set up in setDetails()
   setDetails();
}
void ContestDetails::setDetails(  )
{
   setWindowTitle( (tr("Details of Contest Entry - %1").arg(contestTransferObject->cfileName)));

   ui->ContestNameEdit->setText(contestTransferObject->name.getValue());

   ui->BandComboBox->clear();
   // need to get legal bands from ContestLog

   bool allowHF = false;
   TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpAllowHF, allowHF );

   BandList &blist = BandList::getBandList();
   if (allowHF)
   {
       ui->BandComboBox->addItem( tr("All HF") );
   }
   for (auto const &b: blist.bandList)
   {
       if (allowHF || b->getType() != "HF")
       {
           ui->BandComboBox->addItem( b->uk );
       }
   }

   QString cb = contestTransferObject->contestBands.getValue().trimmed();
   if (cb == allHF)
   {
       cb = tr("All HF");
   }
   else
   {
       QSharedPointer<BandInfo>  bi;
       bool bandOK = blist.findBand(cb, bi);
       if (bandOK)
       {
           cb = bi->uk;
       }
   }
   int b = ui->BandComboBox->findText( cb );        // contest

   if ( b >= 0 )
   {
      ui->BandComboBox->setCurrentIndex( b);
   }
   else
   {
      ui->BandComboBox->setCurrentText(contestTransferObject->contestBands.getValue());
   }

   if (!contestTransferObject->currentMode.getValue().isEmpty())
   {
       int m = ui->ModeComboBox->findText( contestTransferObject->currentMode.getValue() );

       if ( m >= 0 )
       {
          ui->ModeComboBox->setCurrentIndex(m);
       }
       else
       {
          ui->ModeComboBox->setCurrentText(contestTransferObject->currentMode.getValue());
       }
    }
   ui->SectionComboBox->clear();
   if ( sectionList.size() )
   {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
       QStringList sl = sectionList.split(",", Qt::SkipEmptyParts);
#else
       QStringList sl = sectionList.split(",", QString::SkipEmptyParts);
#endif
       ui->SectionComboBox->addItems(sl);
   }

   int s = ui->SectionComboBox->findText( contestTransferObject->entSect.getValue() );        // contest

   if ( s >= 0 )
   {
      ui->SectionComboBox->setCurrentIndex(s);
   }
   else
   {
      ui->SectionComboBox->setCurrentText(contestTransferObject->entSect.getValue());
   }

   // start/end of ContestLog
   // if attempt to log QSO BEFORE, complain (but allow) (give set time offset option)
   // if AFTER at load time, set "post entry"
   // if attempt to log QSO AFTER and NOT post entry, complain (give set time offset option, post entry option)
   //      QString contest->DTGStart;  //ccccmmsshhmm
   //      QString contest->DTGEnd;    //ccccmmsshhmm

   if ( contestTransferObject->DTGStart.getValue().size() )
   {
      QString s = contestTransferObject->DTGStart.getValue();
      QDateTime t = QDateTime::fromString(s, "yyyyMMddHHmm");
      ui->StartDateEdit->setDate(t.date());
      QString stc = t.time().toString( "HH:mm UTC" );
      ui->StartTimeCombo->setCurrentText(stc);
   }
   else
   {
      //         StartDateEdit->Date = "";
      ui->StartTimeCombo->setCurrentText("");
   }
   if ( contestTransferObject->DTGEnd.getValue().size() )
   {
       QString s = contestTransferObject->DTGEnd.getValue();
       QDateTime t = QDateTime::fromString(s, "yyyyMMddHHmm");
      ui->EndDateEdit->setDate(t.date()); // short date format, hours:minutes
      QString etc = t.time().toString( "HH:mm UTC" );
      ui->EndTimeCombo->setCurrentText(etc); // short date format, hours:minutes
   }
   else
   {
      //         EndDateEdit->Date = "";
      ui->EndTimeCombo->setCurrentText("");
   }

   QString call = contestTransferObject->mycall.getFullCall();
   if ( !call.size() )                                       // Entry
   {
      contestTransferObject->entryBundle.getStringProfile( eepCall, call );

      // STL version of strupr
      call = call.toUpper();
      contestTransferObject->mycall.setFullCall( call );
   }
   ui->CallsignEdit->setText(call);

   QString mainop = contestTransferObject->currentOp1.getValue();
   if ( !mainop.size() )                                       // Entry
   {
      contestTransferObject->entryBundle.getStringProfile( eepMainOp, mainop );

   }
   if (contestTransferObject->currentOp1.getValue().size()== 0)
   {
       if (mainop.size())
       {
           contestTransferObject->currentOp1.setValue( mainop);
       }
       else
       {
           contestTransferObject->currentOp1.setValue( contestTransferObject->mycall.realCall);
       }
   }

   QString secondop = contestTransferObject->currentOp2.getValue();
   if ( !secondop.size() )                                       // Entry
   {
      contestTransferObject->entryBundle.getStringProfile( eepSecondOp, secondop );

   }
   if (contestTransferObject->currentOp2.getValue().size()== 0)
   {
      contestTransferObject->currentOp2.setValue( secondop);
   }

   contestTransferObject->validateLoc();

   if ( !contestTransferObject->locValid && contestTransferObject->myloc.getLoc().size() == 0 )
   {
      QString temp;
      contestTransferObject->QTHBundle.getStringProfile( eqpLocator, temp );
      contestTransferObject->myloc.setLoc( temp );
   }
   ui->LocatorEdit->setText(contestTransferObject->myloc.getLoc());

   ui->AllowLoc4CB->setChecked(contestTransferObject->allowLoc4.getValue());    // bool               // ?? contest
   ui->AllowLoc8CB->setChecked(contestTransferObject->allowLoc8.getValue());    // bool               // ?? contest

   ui->ExchangeEdit->setText(contestTransferObject->location.getValue()); // QTH/if contest specifies - but disp anyway

   switch (contestTransferObject->scoreMode.getValue())
   {
   case 0:
       ui->commencedKRB->setChecked(true);
       break;
   case 1:
       ui->PPQSORB->setChecked(true);
       break;
   }

   /*
      ExchangeComboBox:

      No Exchange Required
      PostCode Multipliers
      Other Exchange Multiplier
      Optional Exchange Muktiplier
      Exchange Required (no multiplier)
   */

   if ( contestTransferObject->districtMult.getValue() )
   {
      ui->ExchangeComboBox->setCurrentIndex( 1);
   }
   else
      if ( contestTransferObject->otherExchange.getValue() )
      {
          ui->ExchangeComboBox->setCurrentIndex( 4);
      }
      else
          if ( contestTransferObject->otherOptionalExchange.getValue() )
          {
              ui->ExchangeComboBox->setCurrentIndex( 3);
          }
          else
              {
                  ui->ExchangeComboBox->setCurrentIndex( 0);
              }
   ui->DXCCMult->setChecked( contestTransferObject->countryMult.getValue()) ;
   ui->NonGCtryMult->setChecked( contestTransferObject->nonGCountryMult.getValue()) ;

   ui->M7LocatorMults->setChecked(contestTransferObject->M7Mults.getValue());

   bool usesBonus = contestTransferObject->usesBonus.getValue();
   QString bonusType = contestTransferObject->bonusType.getValue();

   if (usesBonus)
   {
       if (bonusType == "B2")
           ui->BonusComboBox->setCurrentIndex(1);
       else if (bonusType == "B4")
           ui->BonusComboBox->setCurrentIndex(2);
       else if (bonusType == "NAC")
           ui->BonusComboBox->setCurrentIndex(3);
       else
           ui->BonusComboBox->setCurrentIndex(0);

   }
   else
   {
       ui->BonusComboBox->setCurrentIndex(0);
   }

   ui->LocatorMult->setChecked(contestTransferObject->locMult.getValue()) ;
   ui->GLocMult->setChecked(contestTransferObject->GLocMult.getValue());

   ui->PowerEdit->setText(contestTransferObject->power.getValue());

   on_SetRadioList();
   on_RotatorList();

   ui->ageProtectedcb->setChecked(contestTransferObject->isAgeProtected());

   if ( contestTransferObject->isMinosFile() )
   {
      suppressProtectedOnClick = true;
      ui->ProtectedOption->setChecked(contestTransferObject->isProtected() && !contestTransferObject->isProtectedSuppressed());
      suppressProtectedOnClick = false;
   }
   else
   {
      ui->ProtectedOption->setEnabled(false);
   }
   ui->RSTField->setChecked(contestTransferObject->RSTMandatoryField.getValue()) ;   // bool                   // contest
   ui->SerialField->setChecked(contestTransferObject->serialMandatoryField.getValue()) ;   // bool             // contest
   ui->LocatorField->setChecked(contestTransferObject->locatorMandatoryField.getValue()) ;   // bool         // contest

   ui->AntOffsetEdit->setText(QString::number(contestTransferObject->bearingOffset.getValue()));	// int

   ui->MGMCheckBox->setChecked(contestTransferObject->MGMContestRules.getValue());
   refreshOps();


   ui->screenLayoutCombo->clear();
   ScreenConfigFile &scf = ScreenConfigFile::getScreenConfigFile(this);

   QString curConfigName = contestTransferObject->screenLayout.getValue();
   if (curConfigName.isEmpty())
       curConfigName = defaultLayoutName();

   int j = 0;
   int crow = 0;

   for(auto const &c: scf.configs )
   {
       if (c.name == curConfigName)
           crow = j;
       ui->screenLayoutCombo->addItem(c.name);
       j++;
   }
   ui->screenLayoutCombo->setCurrentIndex(crow);


   enableControls();
   focusChange(nullptr, false, nullptr);
}
void ContestDetails::refreshOps()
{
   // refill the op combo boxes from the current contest, and select the correct op
   if (contestTransferObject)
   {
      ui->MainOpComboBox->clear();
      ui->SecondOpComboBox->clear();
      //bool addCall = true;
      QStringList ops;
      for ( auto const &i: contestTransferObject->oplist )
      {
          if (!i.isEmpty())
            ops.append(i);
      }
      ops.append(contestTransferObject->currentOp1.getValue());
      ops.append(contestTransferObject->currentOp2.getValue());

      ops.append("");

      ops.sort();
      ops.removeDuplicates();

      ui->MainOpComboBox->addItems(ops);
      ui->SecondOpComboBox->addItems(ops);

      ui->MainOpComboBox->setCurrentText(contestTransferObject->currentOp1.getValue());
      ui->SecondOpComboBox->setCurrentText(contestTransferObject->currentOp2.getValue());
   }
}
void ContestDetails::setDetails( const IndividualContest &ic )
{

   setWindowTitle(tr("Details of Contest Entry - %1").arg(contestTransferObject->cfileName) );

   ui->ContestNameEdit->setText(ic.description);                      // contest
   contestTransferObject->VHFContestName.setValue(ic.description);

   contestTransferObject->RSTMandatoryField.setValue(true);
   contestTransferObject->serialMandatoryField.setValue(true);
   contestTransferObject->locatorMandatoryField.setValue(true);

   // need to get legal bands from ContestLog
   ui->BandComboBox->clear();

   BandList &blist = BandList::getBandList();
   QSharedPointer<BandInfo>  bi;
    bool bandOK = blist.findBand(ic.reg1band, bi);
    if (bandOK)
    {
        ui->BandComboBox->addItem( bi->uk );
    }
   else
    {
        ui->BandComboBox->addItem( ic.reg1band );
    }
    ui->BandComboBox->setCurrentIndex(0);

    ui->SectionComboBox->clear();

   sectionList = ic.sections; // the combo will then be properly set up in setDetails()
   if ( sectionList.size() )
   {
      QStringList sl = sectionList.split(",");
      ui->SectionComboBox->addItems(sl);
   }

   int s = ui->SectionComboBox->findText( contestTransferObject->entSect.getValue() );        // contest

   if ( s >= 0 )
   {
      ui->SectionComboBox->setCurrentIndex(s);
   }
   else
   {
      ui->SectionComboBox->setCurrentText(contestTransferObject->entSect.getValue());
   }

   ui->StartDateEdit->setDate(ic.start.date());
   ui->StartTimeCombo->setCurrentText(ic.start.toString( "HH:mm" ) + " UTC");

   ui->EndDateEdit->setDate(ic.finish.date()); // short date format, hours:minutes
   ui->EndTimeCombo->setCurrentText(ic.finish.toString( "HH:mm" ) + " UTC"); // short date format, hours:minutes

   if ( ic.mults == "M1" )
   {
      // PC, DXCC
       contestTransferObject->usesBonus.setValue(false);
       contestTransferObject->bonusType.setValue("");

      contestTransferObject->districtMult.setValue( true );
      contestTransferObject->countryMult.setValue( true );
      contestTransferObject->locMult.setValue( false );
      contestTransferObject->GLocMult.setValue( false );
      contestTransferObject->nonGCountryMult.setValue( false );

      contestTransferObject->M7Mults.setValue(false);

      contestTransferObject->UKloc_mult = false;
      contestTransferObject->NonUKloc_mult = false;
      contestTransferObject->UKloc_multiplier = 0;
      contestTransferObject->NonUKloc_multiplier = 0;
   }
   else if ( ic.mults == "M2" )
   {
      // Loc
       contestTransferObject->usesBonus.setValue(false);
       contestTransferObject->bonusType.setValue("");

      contestTransferObject->districtMult.setValue( false );
      contestTransferObject->countryMult.setValue( false );
      contestTransferObject->locMult.setValue( true );
      contestTransferObject->GLocMult.setValue( false );
      contestTransferObject->nonGCountryMult.setValue( false );

      contestTransferObject->M7Mults.setValue(false);

      contestTransferObject->UKloc_mult = true;
      contestTransferObject->NonUKloc_mult = true;
      contestTransferObject->UKloc_multiplier = 1;
      contestTransferObject->NonUKloc_multiplier = 1;
   }
   else if ( ic.mults == "M3" )
   {
      // PC, DXCC, LOC
       contestTransferObject->usesBonus.setValue(false);
       contestTransferObject->bonusType.setValue("");

      contestTransferObject->districtMult.setValue( true );
      contestTransferObject->countryMult.setValue( true );
      contestTransferObject->locMult.setValue( true );
      contestTransferObject->GLocMult.setValue( false );
      contestTransferObject->nonGCountryMult.setValue( false );

      contestTransferObject->M7Mults.setValue(false);

      contestTransferObject->UKloc_mult = true;
      contestTransferObject->NonUKloc_mult = true;
      contestTransferObject->UKloc_multiplier = 1;
      contestTransferObject->NonUKloc_multiplier = 1;
   }
   else if ( ic.mults == "M4" )
   {
      // DXCC, LOC
      contestTransferObject->usesBonus.setValue(false);
      contestTransferObject->bonusType.setValue("");

      contestTransferObject->districtMult.setValue( false );
      contestTransferObject->countryMult.setValue( true );
      contestTransferObject->locMult.setValue( true );
      contestTransferObject->GLocMult.setValue( false );
      contestTransferObject->nonGCountryMult.setValue( false );

      contestTransferObject->M7Mults.setValue(false);

      contestTransferObject->UKloc_mult = true;
      contestTransferObject->NonUKloc_mult = true;
      contestTransferObject->UKloc_multiplier = 1;
      contestTransferObject->NonUKloc_multiplier = 1;
   }
   else if ( ic.mults == "M5" )
   {
      // G Locs only
      contestTransferObject->usesBonus.setValue(false);
      contestTransferObject->bonusType.setValue("");

      contestTransferObject->districtMult.setValue( false );
      contestTransferObject->countryMult.setValue( false );
      contestTransferObject->locMult.setValue( true );
      contestTransferObject->GLocMult.setValue( true );
      contestTransferObject->nonGCountryMult.setValue( false );

      contestTransferObject->M7Mults.setValue(false);

      contestTransferObject->UKloc_mult = true;
      contestTransferObject->NonUKloc_mult = false;
      contestTransferObject->UKloc_multiplier = 1;
      contestTransferObject->NonUKloc_multiplier = 0;
   }
   else if ( ic.mults == "M6" )
   {
      // G Locs only  + DXCC
      contestTransferObject->usesBonus.setValue(false);
      contestTransferObject->bonusType.setValue("");

      contestTransferObject->districtMult.setValue( false );
      contestTransferObject->countryMult.setValue( false );
      contestTransferObject->locMult.setValue( true );
      contestTransferObject->GLocMult.setValue( true );
      contestTransferObject->nonGCountryMult.setValue( true );

      contestTransferObject->M7Mults.setValue(false);

      contestTransferObject->UKloc_mult = true;
      contestTransferObject->NonUKloc_mult = false;
      contestTransferObject->UKloc_multiplier = 1;
      contestTransferObject->NonUKloc_multiplier = 0;
   }
   else if ( ic.mults == "M7" )
   {
      // Modified M5; non UK 1 mult, UK 2 mults
      contestTransferObject->usesBonus.setValue(false);
      contestTransferObject->bonusType.setValue("");

      contestTransferObject->districtMult.setValue( false );
      contestTransferObject->countryMult.setValue( false );
      contestTransferObject->locMult.setValue( true );
      contestTransferObject->GLocMult.setValue( true );
      contestTransferObject->nonGCountryMult.setValue( false );

      contestTransferObject->M7Mults.setValue(true);

      contestTransferObject->UKloc_mult = true;
      contestTransferObject->NonUKloc_mult = true;
      contestTransferObject->UKloc_multiplier = 2;
      contestTransferObject->NonUKloc_multiplier = 1;
   }
   else if ( ic.mults == "B2" )
   {
       contestTransferObject->usesBonus.setValue(true);
       contestTransferObject->bonusType.setValue("B2");

       contestTransferObject->districtMult.setValue( false );
       contestTransferObject->countryMult.setValue( false );
       contestTransferObject->locMult.setValue( false );
       contestTransferObject->GLocMult.setValue( false );
       contestTransferObject->nonGCountryMult.setValue( false );

       contestTransferObject->M7Mults.setValue(false);

       contestTransferObject->UKloc_mult = false;
       contestTransferObject->NonUKloc_mult = false;
       contestTransferObject->UKloc_multiplier = 0;
       contestTransferObject->NonUKloc_multiplier = 0;
   }
   else if ( ic.mults == "B4" )
   {
       contestTransferObject->usesBonus.setValue(true);
       contestTransferObject->bonusType.setValue("B4");

       contestTransferObject->districtMult.setValue( false );
       contestTransferObject->countryMult.setValue( false );
       contestTransferObject->locMult.setValue( false );
       contestTransferObject->GLocMult.setValue( false );
       contestTransferObject->nonGCountryMult.setValue( false );

       contestTransferObject->M7Mults.setValue(false);

       contestTransferObject->UKloc_mult = false;
       contestTransferObject->NonUKloc_mult = false;
       contestTransferObject->UKloc_multiplier = 0;
       contestTransferObject->NonUKloc_multiplier = 0;
   }
   else
   {
      contestTransferObject->usesBonus.setValue(false);
      contestTransferObject->bonusType.setValue("");

      contestTransferObject->districtMult.setValue( false );
      contestTransferObject->countryMult.setValue( false );
      contestTransferObject->locMult.setValue( false );
      contestTransferObject->GLocMult.setValue( false );
      contestTransferObject->nonGCountryMult.setValue( false );

      contestTransferObject->M7Mults.setValue(false);

      contestTransferObject->UKloc_mult = false;
      contestTransferObject->NonUKloc_mult = false;
      contestTransferObject->UKloc_multiplier = 0;
      contestTransferObject->NonUKloc_multiplier = 0;
   }
   if (ic.specialRules.indexOf("MGM") >= 0)
   {
       //contest->locMult.setValue( true );
       contestTransferObject->MGMContestRules.setValue(true);
       contestTransferObject->serialMandatoryField.setValue(false);
       contestTransferObject->allowLoc4.setValue(true);
       ui->AllowLoc4CB->setChecked(true);
   }
   else
   {
       contestTransferObject->MGMContestRules.setValue(false);
   }
   ui->MGMCheckBox->setChecked(contestTransferObject->MGMContestRules.getValue());
   /*
      ExchangeComboBox:

      No Exchange Required
      PostCode Multipliers
      Other Exchange Multiplier
      Optional Exchange Multilier
      Exchange Required (no multiplier)
   */

   if ( contestTransferObject->districtMult.getValue() )
   {
      ui->ExchangeComboBox->setCurrentIndex(1);
   }
   else
      if ( contestTransferObject->otherExchange.getValue() )
      {
         ui->ExchangeComboBox->setCurrentIndex(4);
      }
      else
          if ( contestTransferObject->otherOptionalExchange.getValue() )
          {
             ui->ExchangeComboBox->setCurrentIndex(3);
          }
          else
              {
                 ui->ExchangeComboBox->setCurrentIndex(0);
              }
   ui->NonGCtryMult->setChecked(contestTransferObject->nonGCountryMult.getValue()) ;
   ui->DXCCMult->setChecked(contestTransferObject->countryMult.getValue()) ;

   ui->LocatorMult->setChecked(contestTransferObject->locMult.getValue()) ;
   ui->GLocMult->setChecked(contestTransferObject->GLocMult.getValue()) ;
   ui->M7LocatorMults->setChecked(contestTransferObject->M7Mults.getValue()) ;

   bool UKACBonus = contestTransferObject->usesBonus.getValue();
   if (!UKACBonus)
   {
       ui->BonusComboBox->setCurrentIndex(0);
   }
   else
   {
       QString bonusType = contestTransferObject->bonusType.getValue();
       if (bonusType == "B2")
            ui->BonusComboBox->setCurrentIndex(1);
       if (bonusType == "B4")
            ui->BonusComboBox->setCurrentIndex(2);
   }


   QString mode = ic.mode;
   if (mode.isEmpty())
   {
      if (contestTransferObject->MGMContestRules.getValue() || ic.specialRules.contains("S12"))
          mode = hamlibData::MGM;
      else
          mode = hamlibData::USB;
   }
   int m = ui->ModeComboBox->findText( mode );

   if ( m >= 0 )
   {
      ui->ModeComboBox->setCurrentIndex(m);
   }
   else
   {
      ui->ModeComboBox->setCurrentText(mode);
   }
   contestTransferObject->currentMode.setValue(mode);


   ui->RSTField->setChecked(contestTransferObject->RSTMandatoryField.getValue()) ;
   ui->SerialField->setChecked(contestTransferObject->serialMandatoryField.getValue()) ;
   ui->LocatorField->setChecked(contestTransferObject->locatorMandatoryField.getValue()) ;

   contestTransferObject->scoreMode.setValue( static_cast< SCOREMODE> ( ic.ppKmScoring ? 0 : 1 ) );  // combo

   switch (( ic.ppKmScoring ? 0 : 1 ))
   {
   case 0:
       ui->commencedKRB->setChecked(true);
       break;
   case 1:
       ui->PPQSORB->setChecked(true);
       break;
   }
//   setDetails();
}
//---------------------------------------------------------------------------
static QString ssLineEditFrRedBkRed = "QLineEdit { border-style: outset ; border-width: 2px ; border-color: red  }";
static QString ssComboBoxFrRedBkRed = "QComboBox { border-style: outset ; border-width: 2px ; border-color: red  }";

void ContestDetails::focusChange(QObject * /*obj*/, bool in, QFocusEvent * /*event*/)
{
    if (!in)
    {
        // need to test on control exit (e.g. mouse or tab) as well as on OK

        if ( ui->ContestNameEdit->text().trimmed().isEmpty() )
        {
            ui->ContestNameEdit->setStyleSheet(ssLineEditFrRedBkRed);
        }
        else
        {
            ui->ContestNameEdit->setStyleSheet("");
        }

        if ( ui->BandComboBox->currentText().trimmed().isEmpty() )
        {
            ui->BandComboBox->setStyleSheet(ssComboBoxFrRedBkRed);
        }
        else
        {
            ui->BandComboBox->setStyleSheet("");
        }

        contestTransferObject->mycall.setFullCall( ui->CallsignEdit->text() );
        if ( contestTransferObject->mycall.getValRes() != CS_OK )
        {
            ui->CallsignEdit->setStyleSheet(ssLineEditFrRedBkRed);
        }
        else
        {
            ui->CallsignEdit->setStyleSheet("");
        }

        contestTransferObject->myloc.setLoc( ui->LocatorEdit->text() );
        if ( contestTransferObject->myloc.getValRes() != LOC_OK )
        {
            ui->LocatorEdit->setStyleSheet(ssLineEditFrRedBkRed);
        }
        else
        {
            ui->LocatorEdit->setStyleSheet("");
        }

        if ( ui->PowerEdit->text().trimmed().isEmpty() )
        {
            ui->PowerEdit->setStyleSheet(ssLineEditFrRedBkRed);
        }
        else
        {
            ui->PowerEdit->setStyleSheet("");
        }


        QString cop = ui->MainOpComboBox->currentText();
        if ( cop.trimmed().isEmpty() )
        {
            ui->MainOpComboBox->setStyleSheet(ssComboBoxFrRedBkRed);
        }
        else
        {
            ui->MainOpComboBox->setStyleSheet("");
        }
    }
}
QWidget * ContestDetails::getDetails( )
{
    QWidget *nextD = getNextFocus();

    contestTransferObject->name.setValue( ui->ContestNameEdit->text() );
    QString cb = ui->BandComboBox->currentText();
    if (cb == tr("All HF"))
    {
        cb = allHF;
    }
    contestTransferObject->contestBands.setValue( cb );
    contestTransferObject->entSect.setValue( ui->SectionComboBox->currentText() );
    contestTransferObject->sectionList.setValue( sectionList );

    if ( ui->ContestNameEdit->text().trimmed().isEmpty() )
    {
        if (!nextD)
        {
            nextD = ui->ContestNameEdit;
        }
    }

    if ( ui->BandComboBox->currentText().trimmed().isEmpty() )
    {
        if (!nextD)
        {
            nextD = ui->BandComboBox;
        }
    }

    if (ui->StartDateEdit->text().isEmpty())
    {
        ui->StartDateEdit->setDate(QDate::currentDate());
    }
    QString sdate = ui->StartDateEdit->date().toString("dd/MM/yyyy");
    contestTransferObject->DTGStart.setValue(TDTToCanonical( sdate + " " + ui->StartTimeCombo->currentText())) ;

    if (ui->EndDateEdit->text().isEmpty())
    {
        ui->EndDateEdit->setDate(QDate::currentDate());
    }
    contestTransferObject->DTGEnd.setValue(  TDTToCanonical(ui->EndDateEdit->date().toString("dd/MM/yyyy") + " " + ui->EndTimeCombo->currentText())) ;

    contestTransferObject->mycall.setFullCall( ui->CallsignEdit->text() );
    if ( contestTransferObject->mycall.getValRes() != CS_OK )
    {
        if (!nextD)
        {
            nextD = ui->CallsignEdit;
        }
    }
    contestTransferObject->myloc.setLoc( ui->LocatorEdit->text() );
    if ( contestTransferObject->myloc.getValRes() != LOC_OK )
    {
        if (!nextD)
        {
            nextD = ui->LocatorEdit;
        }
    }

    if ( ui->PowerEdit->text().trimmed().isEmpty() )
    {
        if (!nextD)
        {
            nextD = ui->PowerEdit;
        }
    }


    contestTransferObject->currentOp1.setValue(ui->MainOpComboBox->currentText());
    contestTransferObject->currentOp2.setValue(ui->SecondOpComboBox->currentText());
    contestTransferObject->oplist.insert(contestTransferObject->currentOp1.getValue(), contestTransferObject->currentOp1.getValue());
    contestTransferObject->oplist.insert(contestTransferObject->currentOp2.getValue(), contestTransferObject->currentOp2.getValue());

    if ( contestTransferObject->currentOp1.getValue().isEmpty() )
    {
        if (!nextD)
        {
            nextD = ui->MainOpComboBox;
        }
    }
    contestTransferObject->allowLoc4.setValue( ui->AllowLoc4CB->isChecked() );    // bool
    contestTransferObject->allowLoc8.setValue( ui->AllowLoc8CB->isChecked() );    // bool
    contestTransferObject->location.setValue( ui->ExchangeEdit->text() );
    contestTransferObject->scoreMode.setValue( static_cast< SCOREMODE > (ui->PPQSORB->isChecked()?1:0) );  // combo

    if (ui->NonGCtryMult->isChecked())
    {
        ui->DXCCMult->setChecked(true);
    }
    contestTransferObject->countryMult.setValue( ui->DXCCMult->isChecked() );   // bool
    contestTransferObject->nonGCountryMult.setValue( ui->NonGCtryMult->isChecked() );   // bool

    if (ui->GLocMult->isChecked() || ui->M7LocatorMults->isChecked())
    {
        ui->LocatorMult->setChecked(true);
    }

    contestTransferObject->locMult.setValue( ui->LocatorMult->isChecked() ) ;   // bool
    contestTransferObject->GLocMult.setValue( ui->GLocMult->isChecked() ) ;   // bool
    contestTransferObject->M7Mults.setValue( ui->M7LocatorMults->isChecked() ) ;   // bool
    contestTransferObject->usesBonus.setValue(ui->BonusComboBox->currentIndex() >= 1);

    if (contestTransferObject->usesBonus.getValue())
    {
        int bt = ui->BonusComboBox->currentIndex();

        if (bt == 1)
        {
            contestTransferObject->bonusType.setValue("B2");
        }
        else if (bt == 2)
        {
            contestTransferObject->bonusType.setValue("B4");
        }
        else if (bt == 3)
        {
            contestTransferObject->bonusType.setValue("NAC");
        }
        else
        {
            contestTransferObject->usesBonus.setValue(false);
            contestTransferObject->bonusType.setValue("");
        }
    }
    else
    {
        contestTransferObject->bonusType.setValue("");
    }

    if (contestTransferObject->M7Mults.getValue())
    {
        contestTransferObject->UKloc_mult = true;
        contestTransferObject->NonUKloc_mult = true;
        contestTransferObject->UKloc_multiplier = 2;
        contestTransferObject->NonUKloc_multiplier = 1;
    }
    else
    {
        if (contestTransferObject->locMult.getValue())
        {
            contestTransferObject->UKloc_mult = true;
            contestTransferObject->UKloc_multiplier = 1;

            if (contestTransferObject->GLocMult.getValue())
            {
                contestTransferObject->NonUKloc_mult = false;
                contestTransferObject->NonUKloc_multiplier = 0;
            }
            else
            {
                contestTransferObject->NonUKloc_mult = true;
                contestTransferObject->NonUKloc_multiplier = 1;
            }
        }
        else
        {
            contestTransferObject->UKloc_mult = false;
            contestTransferObject->NonUKloc_mult = false;
            contestTransferObject->UKloc_multiplier = 0;
            contestTransferObject->NonUKloc_multiplier = 0;
        }
    }
    contestTransferObject->MGMContestRules.setValue(ui->MGMCheckBox->isChecked());

    if (ui->ProtectedOption->isChecked() && contestTransferObject->isProtected() && contestTransferObject->isProtectedSuppressed())
    {
        contestTransferObject->setProtectedSuppressed(false);
    }
    else
    {
        if (ui->ProtectedOption->isChecked() && !contestTransferObject->isProtected())
        {
            contestTransferObject->setProtected( true ) ;
            saveContestOK  = true;
        }
        else if (!ui->ProtectedOption->isChecked() && contestTransferObject->isProtected())
        {
            contestTransferObject->setProtected( false ) ;
            saveContestOK  = true;
        }
        else if (!contestTransferObject->isReadOnly()) // not protected, not unwriteable, protected but suppressed
        {
            saveContestOK  = true;
        }
    }
    contestTransferObject->setAgeProtected(ui->ageProtectedcb->isChecked());
    /*
      ExchangeComboBox:

      No Exchange Required
      PostCode Multipliers
      Other Exchange Multiplier
      Optional Exchange Multiplier
      Exchange Required (no multiplier)
   */
    switch ( ui->ExchangeComboBox->currentIndex() )
    {
    case 0:
        contestTransferObject->otherExchange.setValue( false );
        contestTransferObject->otherOptionalExchange.setValue( false );
        contestTransferObject->districtMult.setValue( false );
        break;

    case 1:
        contestTransferObject->otherExchange.setValue( true );
        contestTransferObject->otherOptionalExchange.setValue( false );
        contestTransferObject->districtMult.setValue( true );
        break;

    case 2:
        contestTransferObject->otherExchange.setValue( true );
        contestTransferObject->otherOptionalExchange.setValue( false );
        contestTransferObject->districtMult.setValue( false );
        break;

    case 3:
        contestTransferObject->otherExchange.setValue( false );
        contestTransferObject->otherOptionalExchange.setValue( true );
        contestTransferObject->districtMult.setValue( false );
        break;

    case 4:
        contestTransferObject->otherExchange.setValue( true );
        contestTransferObject->otherOptionalExchange.setValue( false );
        contestTransferObject->districtMult.setValue( false );
        break;

    }
    contestTransferObject->RSTMandatoryField.setValue( ui->RSTField->isChecked() ) ;   // bool
    contestTransferObject->serialMandatoryField.setValue( ui->SerialField->isChecked() ) ;   // bool
    contestTransferObject->locatorMandatoryField.setValue( ui->LocatorField->isChecked() ) ;   // bool

    contestTransferObject->power.setValue( ui->PowerEdit->text() );
    contestTransferObject->bearingOffset.setValue(ui->AntOffsetEdit->text().toInt());	// int

    if (LogContainer->sendDM->radioLoaded)
        contestTransferObject->radioName.setValue(PubSubName(ui->radioNameEdit->currentText().trimmed().remove(':')));
    if (LogContainer->sendDM->rotatorLoaded)
        contestTransferObject->antennaName.setValue(PubSubName(ui->antennaNameEdit->currentText()));

    contestTransferObject->currentMode.setValue(ui->ModeComboBox->currentText());

    contestTransferObject->validateLoc();

    contestTransferObject->screenLayout.setValue(ui->screenLayoutCombo->currentText());

    return nextD;
}
//---------------------------------------------------------------------------
QWidget * ContestDetails::getNextFocus()
{

   if ( ui->ContestNameEdit->text().trimmed().isEmpty() )
   {
      return ui->ContestNameEdit;
   }
   if ( ui->BandComboBox->currentText().trimmed().isEmpty() )
   {
      return ui->BandComboBox;
   }
   if ( ui->CallsignEdit->text().trimmed().isEmpty() )
   {
      return ui->CallsignEdit;
   }
   if ( ui->LocatorEdit->text().trimmed().isEmpty() )
   {
      return ui->LocatorEdit;
   }
   if ( ui->PowerEdit->text().trimmed().isEmpty() )
   {
      return ui->PowerEdit;
   }
   return nullptr;
}

//---------------------------------------------------------------------------
void ContestDetails::enableControls()
{
// Should protected be disabled if the contest is unwriteable?
   bool protectedChecked = ui->ProtectedOption->isChecked() || ui->ageProtectedcb->isChecked();
// enable/disable relevant fields based on protected
   ui->ContestNameEdit->setEnabled(!protectedChecked);
   ui->BandComboBox->setEnabled(!protectedChecked);
   ui->CallsignEdit->setEnabled(!protectedChecked);
   ui->LocatorEdit->setEnabled(!protectedChecked);
   ui->ExchangeEdit->setEnabled(!protectedChecked);
   ui->ScoreGroupBox->setEnabled(!protectedChecked);
   ui->MultGroupBox->setEnabled(!protectedChecked);
   ui->DXCCMult->setEnabled(!protectedChecked);
   ui->LocatorMult->setEnabled(!protectedChecked);
   ui->FieldsGroupBox->setEnabled(!protectedChecked);
   ui->RSTField->setEnabled(!protectedChecked);
   ui->SerialField->setEnabled(!protectedChecked);
   ui->LocatorField->setEnabled(!protectedChecked);

   ui->QTHBundleFrame->enableBundle(!protectedChecked);
   ui->StationBundleFrame->enableBundle(!protectedChecked);
   ui->EntryBundleFrame->enableBundle(!protectedChecked);

   ui->SectionComboBox->setEnabled(!protectedChecked);
   ui->StartTimeCombo->setEnabled(!protectedChecked);
   ui->EndTimeCombo->setEnabled(!protectedChecked);
   ui->ExchangeComboBox->setEnabled(!protectedChecked);
   ui->VHFCalendarButton->setEnabled(!protectedChecked);
   ui->ContestNameSelected->setEnabled(!protectedChecked);
   ui->LocatorGroupBox->setEnabled(!protectedChecked);
   ui->AllowLoc8CB->setEnabled(!protectedChecked);
   ui->AllowLoc4CB->setEnabled(!protectedChecked);
   ui->StartDateEdit->setEnabled(!protectedChecked);
   ui->EndDateEdit->setEnabled(!protectedChecked);
   ui->MainOpComboBox->setEnabled(!protectedChecked);
   ui->SecondOpComboBox->setEnabled(!protectedChecked);
   ui->AntOffsetEdit->setEnabled(!protectedChecked);
   ui->radioNameEdit->setEnabled(!protectedChecked);
   ui->antennaNameEdit->setEnabled(!protectedChecked);

   ui->PowerEdit->setEnabled(!protectedChecked);
   ui->ModeComboBox->setEnabled(!protectedChecked);
   ui->BonusComboBox->setEnabled(!protectedChecked);

   ui->MGMCheckBox->setEnabled(!protectedChecked);

//   if (!protectedChecked)
//   {
//       bool mgm = ui->MGMCheckBox->isChecked();
//       ui->ScoreGroupBox->setEnabled(!mgm);
//       ui->BonusComboBox->setEnabled(!mgm);
//       ui->LocatorGroupBox->setEnabled(!mgm);
//       ui->FieldsGroupBox->setEnabled(!mgm);
//       ui->MultGroupBox->setEnabled(!mgm);
//       ui->ModeComboBox->setEnabled(!mgm);
//   }
}
//---------------------------------------------------------------------------

void ContestDetails::on_OKButton_clicked()
{
    // make sure we have the minimum required information

    if (ui->ProtectedOption->isChecked() && ! contestTransferObject->isProtected())
    {
       if (!mShowYesNoMessage(this, tr("This contest will be marked as protected.\r\n"
                                     "This is a permanent change that may be temporarily overridden.\r\n"
                                     "Please confirm this change by pressing \"Yes\"." )))
       {
          return;
       }
    }

    QWidget *nextD = getDetails( );
    if ( nextD )
    {
       nextD->setFocus();
    }
    else
    {
        contestTransferObject->setToContest(inputcontest);

        inputcontest->loadBonusList();

        if (saveContestOK)
        {
            bool temp = inputcontest->isProtectedSuppressed();
            inputcontest->setProtectedSuppressed(true);
            inputcontest->commonSave( false );
            inputcontest->setProtectedSuppressed(temp);
        }

        accept();
    }

}

void ContestDetails::on_EntDetailButton_clicked()
{
    getDetails( );   // override from the window
    TEntryOptionsForm EntryDlg( this, contestTransferObject, nullptr, true );        // don't show the export options
    if ( EntryDlg.exec() == QDialog::Accepted )
       setDetails( );

}

void ContestDetails::on_CancelButton_clicked()
{
    reject();
}


const char * ContestDetails::BSHelpText =
   QT_TR_NOOP("These settings are groups of settings that can "
   "be applied to a contest all in one go."
   "\r\n\r\n"
   "There are four basic groups: - \r\n\r\n"
   "Contest - for the description, bands, multipliers and time of a contest\r\n"
   "Entry - all the extra bits for a real entry - callsign, group, contact details.\r\n"
   "Station - Rig details, antenna, antenna height.\r\n"
   "QTH - where the station is, height above sea level, Locator.\r\n"
   "\r\n"
   "To use them select from the drop down lists, or for Contest, use the "
   "\"VHF Calendar\" button.\r\n"
   "Any group set to \"<none>\" will be ignored.\r\n"
   "\r\n"
   "If the setting you want isn't there, press the \"Edit\" "
   "button for the group.\r\n"
   "\r\n"
   "This brings up a dialog where you can define a new setting, "
   "copy an existing setting, or delete an existing setting\r\n"
   "\r\n"
   "Click on the setting name on the left to select an existing setting "
   "and then its components are shown in the right hand pane, and "
   "can be edited individually.\r\n"
   "\r\n"
   "Move between components of a group using the mouse or up/down arrow keys.\r\n")
   ;

void ContestDetails::on_BSHelpButton_clicked()
{
    // Put up the help text on bundled settings
     TMinosBSHelpForm HelpForm( this );
     HelpForm.setText(tr(BSHelpText));
     HelpForm.exec();}

void ContestDetails::on_VHFCalendarButton_clicked()
{
    TCalendarForm CalendarDlg(this, ectVHF);

    CalendarDlg.setWindowTitle( tr("VHF Calendar"));
    CalendarDlg.description = ui->ContestNameSelected->text();

    QString sdate = ui->StartDateEdit->date().toString("dd/MM/yyyy");
    CalendarDlg.sdate = CanonicalToTDT(TDTToCanonical( sdate + " " + ui->StartTimeCombo->currentText())) ;
    CalendarDlg.band = ui->BandComboBox->currentText();

    if ( CalendarDlg.exec() == QDialog::Accepted )
    {
       // set up all the details that we can from the calendar
       ui->ContestNameSelected->setText(CalendarDlg.ic.description);
       setDetails( CalendarDlg.ic );
    }
    QWidget *next = getNextFocus();
    if (next)
    {
       next->setFocus();
    }
    else
    {
       ui->OKButton->setFocus();
    }
    focusChange(nullptr, false, nullptr);
}

void ContestDetails::on_CallsignEdit_editingFinished()
{
    if (ui->MainOpComboBox->currentText().isEmpty())
    {
       Callsign cs;
       cs.setFullCall(ui->CallsignEdit->text());

       ui->MainOpComboBox->addItem( ( cs.realCall ) );
       ui->MainOpComboBox->setCurrentText( cs.realCall);
    }

}
void ContestDetails::on_DXCCMult_clicked()
{
    if (noMultRipple)
    {
        return;
    }
    noMultRipple = true;
    if (ui->DXCCMult->isChecked())
    {
       ui->NonGCtryMult->setChecked(false);
    }
    ui->BonusComboBox->setCurrentIndex(0);
    noMultRipple = false;
}

void ContestDetails::on_NonGCtryMult_clicked()
{
    if (noMultRipple)
    {
        return;
    }
    noMultRipple = true;
    if (ui->NonGCtryMult->isChecked())
    {
       ui->DXCCMult->setChecked(false);
    }
    ui->BonusComboBox->setCurrentIndex(0);
    noMultRipple = false;
}

void ContestDetails::on_LocatorMult_clicked()
{
    if (noMultRipple)
    {
        return;
    }
    noMultRipple = true;
    if (!ui->LocatorMult->isChecked())
    {

       ui->GLocMult->setChecked(false);
       ui->M7LocatorMults->setChecked(false);

    }
    ui->BonusComboBox->setCurrentIndex(0);
    noMultRipple = false;
}

void ContestDetails::on_GLocMult_clicked()
{
    if (noMultRipple)
    {
        return;
    }
    noMultRipple = true;
    if (ui->GLocMult->isChecked())
    {
       ui->LocatorMult->setChecked(true);
       ui->M7LocatorMults->setChecked(false);
    }
    ui->BonusComboBox->setCurrentIndex(0);
    noMultRipple = false;
}

void ContestDetails::on_M7LocatorMults_clicked()
{
    if (noMultRipple)
    {
        return;
    }
    noMultRipple = true;
    if (ui->M7LocatorMults->isChecked())
    {
       ui->LocatorMult->setChecked(true);
       ui->GLocMult->setChecked(false);
       ui->NonGCtryMult->setChecked(false);
       ui->DXCCMult->setChecked(false);
    }
    ui->BonusComboBox->setCurrentIndex(0);
    noMultRipple = false;
}
void ContestDetails::on_BonusComboBox_currentIndexChanged(int /*index*/)
{
    if (noMultRipple)
    {
        return;
    }
    noMultRipple = true;
    ui->LocatorMult->setChecked(false);
    ui->GLocMult->setChecked(false);
    ui->NonGCtryMult->setChecked(false);
    ui->DXCCMult->setChecked(false);
    ui->M7LocatorMults->setChecked(false);
//    ui->MGMCheckBox->setChecked(false);  // bonus doesn't preclude MGM
    noMultRipple = false;
}

void ContestDetails::on_ProtectedOption_clicked()
{
    if (!suppressProtectedOnClick)
    {
       if (ui->ProtectedOption->isChecked() )
       {
          // move to protected
          if (!mShowYesNoMessage(this, tr("Are you sure you want to protect this contest?") ))
          {
             ui->ProtectedOption->setChecked(contestTransferObject->isProtected() && !contestTransferObject->isProtectedSuppressed());
          }
       }
       else // unchecked
       {
          if (!mShowYesNoMessage(this, tr("Are you sure you want to disable protection for this contest?") ))
          {
             ui->ProtectedOption->setChecked(contestTransferObject->isProtected() && !contestTransferObject->isProtectedSuppressed());
          }
       }
       enableControls();
    }
}
void ContestDetails::bundleChanged()
{
    getDetails( );   // override from the window

    contestTransferObject->setINIDetails();
    setDetails( );
    QWidget *next = getNextFocus();
    if (next)
    {
        next->setFocus();
    }
    else
    {
        ui->OKButton->setFocus();
    }
}


void ContestDetails::on_MGMCheckBox_stateChanged(int)
{
//    if (ui->MGMCheckBox->isChecked())
//    {
//        noMultRipple = true;

//        ui->LocatorMult->setChecked(true);
//        ui->GLocMult->setChecked(false);
//        ui->NonGCtryMult->setChecked(false);
//        ui->DXCCMult->setChecked(false);
//        ui->M7LocatorMults->setChecked(false);

//        ui->commencedKRB->setChecked(true);
//        ui->BonusComboBox->setCurrentIndex(0);

//        ui->SerialField->setChecked(false);

//        noMultRipple = false;
//    }
    enableControls();
}
void ContestDetails::on_RotatorList()
{
    ui->antennaNameEdit->clear();
    ui->antennaNameEdit->addItem("");
    ui->antennaNameEdit->addItems( LogContainer->sendDM->rotators());
    if (contestTransferObject)
    {
        ui->antennaNameEdit->setCurrentText(contestTransferObject->antennaName.getValue().toString());
    }
}
void ContestDetails::on_SetRadioList()
{
    ui->radioNameEdit->clear();
    ui->radioNameEdit->addItem("");
    ui->radioNameEdit->addItems( LogContainer->sendDM->rigs());
    if (contestTransferObject)
    {
        ui->radioNameEdit->setCurrentText(contestTransferObject->radioName.getValue().toString());
    }
}

void ContestDetails::on_ageProtectedcb_stateChanged(int /*arg1*/)
{
    enableControls();
}
