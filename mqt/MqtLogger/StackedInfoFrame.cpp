#include "base_pch.h"
#include "StackedInfoFrame.h"
#include "tlogcontainer.h"
#include "tsinglelogframe.h"
#include "LoggerContest.h"

#include "ui_StackedInfoFrame.h"

ContList contlist[ CONTINENTS ] =
   {
      {"EU", true},
      {"AS", false},
      {"AF", false},
      {"OC", false},
      {"SA", false},
      {"NA", false},
   };
bool showWorked = false;
bool showUnworked = false;

StackedInfoFrame::StackedInfoFrame(QWidget *parent, int instance) :
    QFrame(parent),
    ui(new Ui::StackedInfoFrame),
    stackInstance(instance),
    contest(nullptr)
{
    ui->setupUi(this);

    QStringList infoList =
    {
        "Clock",
        "DXCC",
        "District",
        "Filter",
        "Memories",
        "Locator Map",
        "Locator Tree",
        "Stats"
    };
    ui->infoCombo->addItems(infoList);

    connect(&MinosLoggerEvents::mle, SIGNAL(ScrollToCountry(QString,BaseContestLog*)), this, SLOT(on_ScrollToCountry(QString,BaseContestLog*)), Qt::QueuedConnection);
    connect(&MinosLoggerEvents::mle, SIGNAL(ScrollToDistrict(QString,BaseContestLog*)), this, SLOT(on_ScrollToDistrict(QString,BaseContestLog*)), Qt::QueuedConnection);
    connect(&MinosLoggerEvents::mle, SIGNAL(FontChanged()), this, SLOT(on_FontChanged()), Qt::QueuedConnection);
    connect(&MinosLoggerEvents::mle, SIGNAL(FiltersChanged(BaseContestLog*)), this, SLOT(onFiltersChanged(BaseContestLog*)), Qt::QueuedConnection);
    connect(&MinosLoggerEvents::mle, SIGNAL(UpdateStats(BaseContestLog*)), this, SLOT(onUpdateStats(BaseContestLog*)), Qt::QueuedConnection);
    connect(&MinosLoggerEvents::mle, SIGNAL(UpdateMemories(BaseContestLog*)), this, SLOT(onUpdateMemories(BaseContestLog*)), Qt::QueuedConnection);
    connect(&MinosLoggerEvents::mle, SIGNAL(RefreshMults(BaseContestLog*)), this, SLOT(onRefreshMults(BaseContestLog*)), Qt::QueuedConnection);
    connect(&MinosLoggerEvents::mle, SIGNAL(setStackContest(LoggerContestLog *)), this, SLOT(setContest(LoggerContestLog *)));
    connect(&MinosLoggerEvents::mle, SIGNAL(refreshStackMults(LoggerContestLog *)), this, SLOT(refreshMults(LoggerContestLog *)));
}

StackedInfoFrame::~StackedInfoFrame()
{
    delete ui;
}

void StackedInfoFrame::on_infoCombo_currentIndexChanged(int arg1)
{
    ui->StackedMults-> setCurrentIndex(arg1);
    if (contest)
    {
        if (contest)
        {
            if (stackInstance == 0)
                contest->currentStackItem.setValue(ui->infoCombo->currentText());
            else if (stackInstance == 1)
                contest->currentStack1Item.setValue(ui->infoCombo->currentText());
            else if (stackInstance == 2)
                contest->currentStack2Item.setValue(ui->infoCombo->currentText());
            else if (stackInstance == 3)
                contest->currentStack3Item.setValue(ui->infoCombo->currentText());
        }
        contest->commonSave(false);
    }
}

void StackedInfoFrame::setContest(LoggerContestLog *ct)
{
    if (contest != ct)
    {
        contest = ct;

        ui->MultFilters->setContest(contest);
        ui->dxccFrame->setContest(contest);
        ui->districtFrame->setContest(contest);
        ui->StatsFrame->setContest(contest);
        ui->locFrame->setContest(contest);
        ui->locTreeFrame->setContest(contest);
        ui->clockFrame->setContest(contest);
        ui->rigMemFrame->setContest(contest);

        if (contest)
        {
            if (stackInstance == 0)
                ui->infoCombo->setCurrentText(contest->currentStackItem.getValue());   // start up on the clock - useful outside the contest!
            else if (stackInstance == 1)
                ui->infoCombo->setCurrentText(contest->currentStack1Item.getValue());   // start up on the clock - useful outside the contest!
            else if (stackInstance == 2)
                ui->infoCombo->setCurrentText(contest->currentStack2Item.getValue());   // start up on the clock - useful outside the contest!
            else if (stackInstance == 3)
                ui->infoCombo->setCurrentText(contest->currentStack3Item.getValue());   // start up on the clock - useful outside the contest!
        }
    }
}
void StackedInfoFrame::on_ScrollToDistrict( const QString &qth, BaseContestLog *c )
{
    if (contest && contest == c)
    {
        QSharedPointer<DistrictEntry> dist = MultLists::getMultLists() ->searchDistrict( qth );
        if ( dist )
        {
            int district_ind = MultLists::getMultLists() ->getDistListIndexOf( dist );
           ui->districtFrame->scrollToDistrict( district_ind, true );
        }
    }
}

void StackedInfoFrame::on_ScrollToCountry( const QString &csCs, BaseContestLog *c )
{
    if (contest && contest == c)
    {
        Callsign cs( csCs );
        cs.validate( );	// we don't use the result

        QSharedPointer<CountryEntry> ctryMult = findCtryPrefix( cs );
        if ( ctryMult )
        {
           int ctry_ind = MultLists::getMultLists() ->getCtryListIndexOf( ctryMult );
           ui->dxccFrame->scrollToCountry( ctry_ind, true );
        }
    }
}
void StackedInfoFrame::refreshMults(LoggerContestLog *ct)
{
    if (contest == ct)
    {
        MinosLoggerEvents::sendRefreshMults(contest);
    }
}
void StackedInfoFrame::onUpdateStats(BaseContestLog *ct)
{
    if (contest == ct)
    {
        ui->StatsFrame->reInitialiseStats();
    }
}
void StackedInfoFrame::onUpdateMemories(BaseContestLog *ct)
{
    if (contest == ct)
    {
        ui->rigMemFrame->doMemoryUpdates();
    }
}
void StackedInfoFrame::onRefreshMults(BaseContestLog *ct)
{
    if (contest == ct)
    {
        ui->locFrame->reInitialiseLocators();
        ui->locTreeFrame->reInitialiseLocators();
        ui->dxccFrame->reInitialiseCountries();
        ui->districtFrame->reInitialiseDistricts();
        //ui->rigMemFrame->reInitialiseMemories();
        //ui->rigMemFrame->doMemoryUpdates();
    }
}

void StackedInfoFrame::on_FontChanged()
{
    refreshMults(contest);
}



void StackedInfoFrame::onFiltersChanged(BaseContestLog *ct)
{
    if (contest && ct == contest)
    {
        ui->MultFilters->initFilters();
        ui->dxccFrame->reInitialiseCountries();
        ui->districtFrame->reInitialiseDistricts();
        ui->locFrame->reInitialiseLocators();
        ui->locTreeFrame->reInitialiseLocators();
        ui->StatsFrame->reInitialiseStats();
        //ui->rigMemFrame->reInitialiseMemories();
    }
}
void StackedInfoFrame::on_StackedMults_currentChanged(int /*arg1*/)
{
    ui->StatsFrame->reInitialiseStats();
}
