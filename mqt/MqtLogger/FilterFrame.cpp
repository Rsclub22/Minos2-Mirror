#include "base_pch.h"
#include "MinosLoggerEvents.h"

#include "LoggerContest.h"
#include "FilterFrame.h"
#include "ui_FilterFrame.h"

FilterFrame::FilterFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::FilterFrame)
{
    ui->setupUi(this);
}

FilterFrame::~FilterFrame()
{
    delete ui;
}
void FilterFrame::filtersChanged()
{
    if (contest)
    {
        MinosLoggerEvents::sendFiltersChanged(contest);
    }
}
void FilterFrame::setContest(LoggerContestLog *ct)
{
    contest = ct;

    initFilters();
}
void FilterFrame::initFilters()
{
   filterClickEnabled = false;  // stop them being saved while we are setting up

   if (contest)
   {
       contlist[ 0 ].allow = contest->showContinentEU.getValue();
       contlist[ 1 ].allow = contest->showContinentAS.getValue();
       contlist[ 2 ].allow = contest->showContinentAF.getValue();
       contlist[ 3 ].allow = contest->showContinentOC.getValue();
       contlist[ 4 ].allow = contest->showContinentSA.getValue();
       contlist[ 5 ].allow = contest->showContinentNA.getValue();

       ui->ContEU->setChecked(contlist[ 0 ].allow);
       ui->ContAS->setChecked(contlist[ 1 ].allow);
       ui->ContAF->setChecked(contlist[ 2 ].allow);
       ui->ContOC->setChecked(contlist[ 3 ].allow);
       ui->ContSA->setChecked(contlist[ 4 ].allow);
       ui->ContNA->setChecked(contlist[ 5 ].allow);
       ui->WorkedCB->setChecked(contest->showWorked.getValue());
       ui->UnworkedCB->setChecked(contest->showUnworked.getValue());
    }
   filterClickEnabled = true;
}
void FilterFrame::saveFilters()
{
    if ( filterClickEnabled )
    {
        contlist[ 0 ].allow = ui->ContEU->isChecked();
        contlist[ 1 ].allow = ui->ContAS->isChecked();
        contlist[ 2 ].allow = ui->ContAF->isChecked();
        contlist[ 3 ].allow = ui->ContOC->isChecked();
        contlist[ 4 ].allow = ui->ContSA->isChecked();
        contlist[ 5 ].allow = ui->ContNA->isChecked();
        contest->showWorked.setValue(ui->WorkedCB->isChecked());
        contest->showUnworked.setValue(ui->UnworkedCB->isChecked());

        contest->showContinentEU.setValue(contlist[ 0 ].allow);
        contest->showContinentAS.setValue(contlist[ 1 ].allow);
        contest->showContinentAF.setValue(contlist[ 2 ].allow);
        contest->showContinentOC.setValue(contlist[ 3 ].allow);
        contest->showContinentSA.setValue(contlist[ 4 ].allow);
        contest->showContinentNA.setValue(contlist[ 5 ].allow);

        contest->commonSave(false);
        filtersChanged();
    }
}

void FilterFrame::on_WorkedCB_clicked()
{
    saveFilters();
}

void FilterFrame::on_UnworkedCB_clicked()
{
    saveFilters();
}

void FilterFrame::on_ContEU_clicked()
{
    saveFilters();
}

void FilterFrame::on_ContOC_clicked()
{
    saveFilters();
}

void FilterFrame::on_ContAS_clicked()
{
    saveFilters();
}

void FilterFrame::on_ContSA_clicked()
{
    saveFilters();
}

void FilterFrame::on_ContAF_clicked()
{
    saveFilters();
}

void FilterFrame::on_ContNA_clicked()
{
    saveFilters();
}
