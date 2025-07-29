#include "ContestApp.h"
#include "MMessageDialog.h"
#include "tloccalcform.h"
#include "contest.h"
#include "ScreenContact.h"
#include "tforcelogdlg.h"
#include "ui_tforcelogdlg.h"

TForceLogDlg::TForceLogDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TForceLogDlg)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    ui->CheckBox3->setFocus();
}

TForceLogDlg::~TForceLogDlg()
{
    delete ui;
}
void TForceLogDlg::on_OKButton_clicked()
{
    accept();
}

void TForceLogDlg::on_LocCalcButton_clicked()
{
    TLocCalcForm loccalc( this );
    loccalc.S1Loc = TContestApp::getContestApp() ->getCurrentContest() ->myloc.getLoc();
    if ( loccalc.exec() == QDialog::Accepted )
    {
       ui->CheckBox4->setChecked(false);	// Scored - so not non-scoring!
       ui->ScoreIl->setText( loccalc.Distance);
    }
    return ;
}

void TForceLogDlg::on_CancelButton_clicked()
{
    reject();
}
int TForceLogDlg::doexec(BaseContestLog *contest,  ScreenContact &screenContact, ErrorList &errs)
{
    for ( auto const &e: errs )
    {
        QString es = Validator::tr(e->errStr);
        new QListWidgetItem(es, ui->ErrList);
    }
    ui->ErrList->setCurrentRow(0);

    int s = screenContact.contactScore.getValue();
    if ( s < 0 )
       s = 0;
    QString temp = QString::number(s);
    ui->ScoreIl->setText(temp);

    ui->CheckBox1->setChecked(screenContact.contactFlags.getValue() & TO_BE_ENTERED);
    ui->CheckBox2->setChecked(screenContact.contactFlags.getValue() & VALID_DUPLICATE);
    ui->CheckBox3->setChecked(screenContact.contactFlags.getValue() & MANUAL_SCORE);
    ui->CheckBox4->setChecked(screenContact.contactFlags.getValue() & NON_SCORING);
    ui->CheckBox5->setChecked(screenContact.contactFlags.getValue() & DONT_PRINT);
    ui->CheckBox6->setChecked(screenContact.contactFlags.getValue() & COUNTRY_FORCED);
    ui->CheckBox7->setChecked(screenContact.contactFlags.getValue() & VALID_DISTRICT);
    ui->CheckBox8->setChecked(screenContact.contactFlags.getValue() & XBAND);

    if ((screenContact.cs.getValRes() == ERR_DUPCS) ||
            ( screenContact.contactFlags.getValue() & ( NON_SCORING | MANUAL_SCORE | DONT_PRINT | VALID_DUPLICATE | TO_BE_ENTERED | XBAND ) ) )

    {
        // set nothing! DUPs are dealt with!
    }
    else
        if ( errs.size() != 0 )  				// no errors -> OK
            ui->CheckBox4->setChecked(screenContact.contactFlags.getValue() | NON_SCORING);

    if ( screenContact.contactFlags.getValue() & COUNTRY_FORCED )
    {
        ui->CtryMultIl->setText(screenContact.forcedMult.getValue());
    }
    else
        if ( contest->countryMult.getValue() && screenContact.ctryMult )
        {
            ui->CtryMultIl->setText(screenContact.ctryMult->getBasePrefix());
        }

    bool tryagain = true;
    int res = QDialog::Rejected;

    while ( tryagain && (( res = QDialog::exec() ) == QDialog::Accepted) )
    {
        //if ( contest->countryMult.getValue() )
        {
            temp = ui->CtryMultIl->text();
        }
//        else
//            break;

        if ( !ui->CheckBox6->isChecked() )
        {
            screenContact.contactFlags.setValue(screenContact.contactFlags.getValue() & ~ COUNTRY_FORCED);
            screenContact.ctryMult.reset();
            screenContact.forcedMult.setValue(QString());
            break;
        }

        temp = temp.trimmed();

        QSharedPointer<CountryEntry> ctryMult = MultLists::getMultLists() ->getCtryForPrefix( temp );
        if ( ctryMult )
        {
            tryagain = false;
            if ( screenContact.ctryMult != ctryMult )
            {
                screenContact.ctryMult = ctryMult;
                screenContact.contactFlags.setValue(screenContact.contactFlags.getValue() | COUNTRY_FORCED);
                screenContact.forcedMult.setValue(temp);
            }
        }
        else
        {
            if ( mShowYesNoMessage( this, tr("Country not in CTY.DAT. Leave for now?")) )
            {
                tryagain = false;
                screenContact.contactFlags.setValue(screenContact.contactFlags.getValue() & ~COUNTRY_FORCED);
                screenContact.forcedMult.setValue(QString());
            }
        }
    }
    if ( res == QDialog::Accepted )
    {
        unsigned short cf = screenContact.contactFlags.getValue();
        // save contact...
        cf |= FORCE_LOG;
        // here read it all off the dialog

        cf &= ~( NON_SCORING | MANUAL_SCORE | DONT_PRINT | VALID_DUPLICATE | TO_BE_ENTERED | VALID_DISTRICT | XBAND );

        if ( ui->CheckBox1->isChecked() )
        {
            cf |= TO_BE_ENTERED;
        }
        if ( ui->CheckBox2->isChecked() )
        {
            cf |= VALID_DUPLICATE;
        }
        if ( ui->CheckBox3->isChecked() )
        {
            cf |= MANUAL_SCORE;
            temp = ui->ScoreIl->text().trimmed();
            screenContact.contactScore.setValue( temp.toInt());
        }
        if ( cf & ( TO_BE_ENTERED | VALID_DUPLICATE | MANUAL_SCORE ) )
            ui->CheckBox4->setChecked(false);

        if ( ui->CheckBox4->isChecked() )
            cf |= NON_SCORING;
        if ( ui->CheckBox5->isChecked() )
        {
            cf |= ( DONT_PRINT | NON_SCORING );
        }
        if ( ui->CheckBox7->isChecked() )
            cf |= VALID_DISTRICT;
        if ( ui->CheckBox8->isChecked() )
            cf |= XBAND;
        screenContact.contactFlags.setValue(cf);
    }
    return res;
}
