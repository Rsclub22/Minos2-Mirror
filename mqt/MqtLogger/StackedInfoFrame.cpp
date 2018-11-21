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
        //"Cluster",
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
    //connect(&MinosLoggerEvents::mle, SIGNAL(RefreshMults(BaseContestLog*)), this, SLOT(onRefreshMults(BaseContestLog*)), Qt::QueuedConnection);
    connect(&MinosLoggerEvents::mle, SIGNAL(refreshStackMults(BaseContestLog *)), this, SLOT(onRefreshStackMults(BaseContestLog *)));
}

StackedInfoFrame::~StackedInfoFrame()
{
    delete ui;
}

void StackedInfoFrame::on_infoCombo_currentIndexChanged(int arg1)
{
    if (currStackFrame)
    {
        layout()->removeWidget(currStackFrame);
        currStackFrame->deleteLater();
        currStackFrame = nullptr;
    }

    clockFrame = nullptr;
    //clusterClientFrame = nullptr;
    dxccFrame = nullptr;
    districtFrame = nullptr;
    filterFrame = nullptr;
    rigMemFrame = nullptr;
    locFrame = nullptr;
    locTreeFrame = nullptr;
    statsFrame = nullptr;

    switch ( arg1 )
    {
    case 0:
        clockFrame = new TClockFrame(this);
        currStackFrame = clockFrame;
        layout()->addWidget(currStackFrame);
        clockFrame->setContest(contest);
        break;
/*
    case 1:


        if (contest->clusterFrameCount < 3)
        {
            int instanceNum = getClusterInstanceNum();
            if (instanceNum != -1)
            {
                qDebug() << "stackframe cluster count = " << contest->clusterFrameCount;
                contest->clusterFrameCount++;
                clusterClientFrame = new ClusterClientFrame(this, instanceNum);
                currStackFrame = clusterClientFrame;
                layout()->addWidget(currStackFrame);
                clusterClientFrame->setContest(contest);
            }

        }
        else
        {
            // default to clock if max cluster frames
            clockFrame = new TClockFrame(this);
            currStackFrame = clockFrame;
            layout()->addWidget(currStackFrame);
            clockFrame->setContest(contest);
        }
        break;
*/
    case 1:
        dxccFrame = new DXCCFrame(this);
        currStackFrame = dxccFrame;
        layout()->addWidget(currStackFrame);
        dxccFrame->setContest(contest);
        break;
    case 2:
//    "District",
        districtFrame = new DistrictFrame(this);
        currStackFrame = districtFrame;
        layout()->addWidget(districtFrame);
        districtFrame->setContest(contest);
        break;
    case 3:
//    "Filter",
        filterFrame = new FilterFrame(this);
        currStackFrame = filterFrame;
        layout()->addWidget(filterFrame);
        filterFrame->setContest(contest);
        break;
    case 4:
//    "Memories",
        rigMemFrame = new RigMemoryFrame(this);
        currStackFrame = rigMemFrame;
        layout()->addWidget(rigMemFrame);
        rigMemFrame->setContest(contest);
        break;
    case 5:
//    "Locator Map",
        locFrame = new LocFrame(this);
        currStackFrame = locFrame;
        layout()->addWidget(locFrame);
        locFrame->setContest(contest);
        break;
    case 6:
//    "Locator Tree",
        locTreeFrame = new LocTreeFrame(this);
        currStackFrame = locTreeFrame;
        layout()->addWidget(locTreeFrame);
        locTreeFrame->setContest(contest);
        break;
    case 7:
//    "Stats"
        statsFrame = new TStatsDispFrame(this);
        currStackFrame = statsFrame;
        layout()->addWidget(statsFrame);
        statsFrame->setContest(contest);
        break;
    }
    if (contest)
    {
        if (contest)
        {
            if (stackInstance < STACKITEMS)
                contest->currentStackItems[stackInstance].setValue(ui->infoCombo->currentText());
        }
        contest->commonSave(false);
    }
}

void StackedInfoFrame::setContest(LoggerContestLog *ct)
{
    if (contest != ct)
    {
        contest = ct;

        if (filterFrame)
            filterFrame->setContest(contest);
        if (dxccFrame)
            dxccFrame->setContest(contest);
        if (districtFrame)
            districtFrame->setContest(contest);
        if (statsFrame)
            statsFrame->setContest(contest);
        if (locFrame)
            locFrame->setContest(contest);
        if (locTreeFrame)
            locTreeFrame->setContest(contest);
        if (clockFrame)
            clockFrame->setContest(contest);
        if (rigMemFrame)
            rigMemFrame->setContest(contest);
/*
        if (clusterClientFrame)
            clusterClientFrame->setContest(contest);
*/
        if (contest)
        {
            if (stackInstance < STACKITEMS)
                ui->infoCombo->setCurrentText(contest->currentStackItems[stackInstance].getValue());   // start up on the clock - useful outside the contest!
        }
    }
}
void StackedInfoFrame::on_ScrollToDistrict( const QString &qth, BaseContestLog *c )
{
    if (contest && contest == c && districtFrame)
    {
        QSharedPointer<DistrictEntry> dist = MultLists::getMultLists() ->searchDistrict( qth );
        if ( dist )
        {
            int district_ind = MultLists::getMultLists() ->getDistListIndexOf( dist );
           districtFrame->scrollToDistrict( district_ind, true );
        }
    }
}

void StackedInfoFrame::on_ScrollToCountry( const QString &csCs, BaseContestLog *c )
{
    if (contest && contest == c && dxccFrame)
    {
        Callsign cs( csCs );
        cs.validate( );	// we don't use the result

        QSharedPointer<CountryEntry> ctryMult = findCtryPrefix( cs );
        if ( ctryMult )
        {
           int ctry_ind = MultLists::getMultLists() ->getCtryListIndexOf( ctryMult );
           dxccFrame->scrollToCountry( ctry_ind, true );
        }
    }
}
/*
void StackedInfoFrame::refreshMults(LoggerContestLog *ct)
{
    if (contest == ct)
    {
        MinosLoggerEvents::sendRefreshMults(contest);
    }
}
*/
void StackedInfoFrame::onUpdateStats(BaseContestLog *ct)
{
    if (contest == ct && statsFrame)
    {
        statsFrame->reInitialiseStats();
    }
}
void StackedInfoFrame::onUpdateMemories(BaseContestLog *ct)
{
    if (contest == ct && rigMemFrame)
    {
        rigMemFrame->doMemoryUpdates();
    }
}
void StackedInfoFrame::onRefreshStackMults(BaseContestLog *ct)
{
    if (contest == ct)
    {
        if (locFrame)
            locFrame->reInitialiseLocators();
        if (locTreeFrame)
            locTreeFrame->reInitialiseLocators();
        if (dxccFrame)
            dxccFrame->reInitialiseCountries();
        if (districtFrame)
            districtFrame->reInitialiseDistricts();
    }
}

void StackedInfoFrame::on_FontChanged()
{
    //refreshMults(contest);
}



void StackedInfoFrame::onFiltersChanged(BaseContestLog *ct)
{
    if (contest && ct == contest)
    {
        if (filterFrame)
            filterFrame->initFilters();
        if (dxccFrame)
            dxccFrame->reInitialiseCountries();
        if (districtFrame)
            districtFrame->reInitialiseDistricts();
        if (locFrame)
            locFrame->reInitialiseLocators();
        if (locTreeFrame)
            locTreeFrame->reInitialiseLocators();
        if (statsFrame)
            statsFrame->reInitialiseStats();
    }
}

/*
int StackedInfoFrame::getClusterInstanceNum()
{
    for (int i = 0; i < contest->clusterInstanceFlags.count(); i++)
    {
        if (!contest->clusterInstanceFlags[i])
        {
            contest->clusterInstanceFlags[i] = true;
            return i;
        }
    }

    return -1;   // no slots found

}
*/
