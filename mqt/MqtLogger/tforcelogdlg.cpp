#include "base_pch.h"

#include "ContestApp.h"
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
        new QListWidgetItem(tr(e->errStr), ui->ErrList);
    }
    ui->ErrList->setCurrentRow(0);

    int s = screenContact.contactScore;
    if ( s < 0 )
       s = 0;
    QString temp = QString::number(s);
    ui->ScoreIl->setText(temp);

    ui->CheckBox1->setChecked(screenContact.contactFlags & TO_BE_ENTERED);
    ui->CheckBox2->setChecked(screenContact.contactFlags & VALID_DUPLICATE);
    ui->CheckBox3->setChecked(screenContact.contactFlags & MANUAL_SCORE);
    ui->CheckBox4->setChecked(screenContact.contactFlags & NON_SCORING);
    ui->CheckBox5->setChecked(screenContact.contactFlags & DONT_PRINT);
    ui->CheckBox6->setChecked(screenContact.contactFlags & COUNTRY_FORCED);
    ui->CheckBox7->setChecked(screenContact.contactFlags & VALID_DISTRICT);
    ui->CheckBox8->setChecked(screenContact.contactFlags & XBAND);

    if ((screenContact.cs.getValRes() == ERR_DUPCS) ||
            ( screenContact.contactFlags & ( NON_SCORING | MANUAL_SCORE | DONT_PRINT | VALID_DUPLICATE | TO_BE_ENTERED | XBAND ) ) )

    {
        // set nothing! DUPs are dealt with!
    }
    else
        if ( errs.size() != 0 )  				// no errors -> OK
            ui->CheckBox4->setChecked(screenContact.contactFlags | NON_SCORING);

    if ( screenContact.contactFlags & COUNTRY_FORCED )
    {
        ui->CtryMultIl->setText(screenContact.forcedMult);
    }
    else
        if ( contest->countryMult.getValue() && screenContact.ctryMult )
        {
            ui->CtryMultIl->setText(screenContact.ctryMult->basePrefix);
        }

    bool tryagain = true;
    int res = QDialog::Rejected;

    while ( tryagain && (( res = QDialog::exec() ) == QDialog::Accepted) )
    {
        if ( contest->countryMult.getValue() )
        {
            temp = ui->CtryMultIl->text();
        }
        else
            break;

        if ( !ui->CheckBox6->isChecked() )
        {
            tryagain = false;
            screenContact.contactFlags &= ~ COUNTRY_FORCED;
            screenContact.ctryMult.reset();
            screenContact.forcedMult = "";
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
                screenContact.contactFlags |= COUNTRY_FORCED;
                screenContact.forcedMult = temp;
            }
        }
        else
        {
            if ( mShowYesNoMessage( this, tr("Country not in CTY.DAT. Leave for now?")) )
            {
                tryagain = false;
                screenContact.contactFlags &= ~COUNTRY_FORCED;
                screenContact.forcedMult = "";
            }
        }
    }
    if ( res == QDialog::Accepted )
    {
        // save contact...
        screenContact.contactFlags |= FORCE_LOG;
        // here read it all off the dialog

        screenContact.contactFlags &= ~( NON_SCORING | MANUAL_SCORE | DONT_PRINT | VALID_DUPLICATE | TO_BE_ENTERED | VALID_DISTRICT | XBAND );

        if ( ui->CheckBox1->isChecked() )
        {
            screenContact.contactFlags |= TO_BE_ENTERED;
        }
        if ( ui->CheckBox2->isChecked() )
        {
            screenContact.contactFlags |= VALID_DUPLICATE;
        }
        if ( ui->CheckBox3->isChecked() )
        {
            screenContact.contactFlags |= MANUAL_SCORE;
            temp = ui->ScoreIl->text().trimmed();
            screenContact.contactScore = temp.toInt();
        }
        if ( screenContact.contactFlags & ( TO_BE_ENTERED | VALID_DUPLICATE | MANUAL_SCORE ) )
            ui->CheckBox4->setChecked(false);

        if ( ui->CheckBox4->isChecked() )
            screenContact.contactFlags |= NON_SCORING;
        if ( ui->CheckBox5->isChecked() )
        {
            screenContact.contactFlags |= ( DONT_PRINT | NON_SCORING );
        }
        if ( ui->CheckBox7->isChecked() )
            screenContact.contactFlags |= VALID_DISTRICT;
        if ( ui->CheckBox8->isChecked() )
            screenContact.contactFlags |= XBAND;

    }
    return res;
}
