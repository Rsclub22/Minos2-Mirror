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

QVector <AuxTypeOption> StackedInfoFrame::auxoptions = {
    {aeClock, QT_TR_NOOP("Clock"), QT_TR_NOOP("Clock")},
    {aeDXCC, QT_TR_NOOP("DXCC"), QT_TR_NOOP("DXCC")},
    {aeDistrict, QT_TR_NOOP("District"), QT_TR_NOOP("District")},
    {aeFilter, QT_TR_NOOP("Filter"), QT_TR_NOOP("Filter")},
    {aeMemories, QT_TR_NOOP("Memories"), QT_TR_NOOP("Memories")},
    {aeLocatorMap, QT_TR_NOOP("Locator Map"), QT_TR_NOOP("Locator Map")},
    {aeLocatorTree, QT_TR_NOOP("Locator Tree"), QT_TR_NOOP("Locator Tree")},
    {aeStats, QT_TR_NOOP("Stats"), QT_TR_NOOP("Stats")},
};

AuxEntries StackedInfoFrame::getAuxEntryType(QString s)
{
    foreach(const AuxTypeOption &opt, auxoptions)
    {
        if (tr(opt.s) == s || (opt.s == s))
            return opt.type;
    }
    return aeClock;
}

const char * StackedInfoFrame::getRawAuxTypeString(AuxEntries t)
{
    foreach(const AuxTypeOption &opt, auxoptions)
    {
        if (opt.type == t)
            return opt.s;
    }
    return getRawAuxTypeString(aeClock);
}
QString StackedInfoFrame::getTrAuxTypeString(AuxEntries t)
{
    return tr(getRawAuxTypeString(t));
}

bool showWorked = false;
bool showUnworked = false;

StackedInfoFrame::StackedInfoFrame(QWidget *parent, int instance) :
    QFrame(parent),
    ui(new Ui::StackedInfoFrame),
    stackInstance(instance),
    contest(nullptr)

{
    ui->setupUi(this);

    ui->infoCombo->clear();
    int i = 0;
    foreach(const AuxTypeOption &opt, auxoptions)
    {
        ui->infoCombo->addItem(tr(opt.s), opt.type);
        ui->infoCombo->setItemData( i++, tr(opt.hint), Qt::ToolTipRole );
    }

    connect(&MinosLoggerEvents::mle, SIGNAL(ScrollToCountry(QString,BaseContestLog*)), this, SLOT(on_ScrollToCountry(QString,BaseContestLog*)), Qt::QueuedConnection);
    connect(&MinosLoggerEvents::mle, SIGNAL(ScrollToDistrict(QString,BaseContestLog*)), this, SLOT(on_ScrollToDistrict(QString,BaseContestLog*)), Qt::QueuedConnection);
    connect(&MinosLoggerEvents::mle, SIGNAL(FontChanged()), this, SLOT(on_FontChanged()), Qt::QueuedConnection);
    connect(&MinosLoggerEvents::mle, SIGNAL(FiltersChanged(BaseContestLog*)), this, SLOT(onFiltersChanged(BaseContestLog*)), Qt::QueuedConnection);
    connect(&MinosLoggerEvents::mle, SIGNAL(UpdateStats(BaseContestLog*)), this, SLOT(onUpdateStats(BaseContestLog*)), Qt::QueuedConnection);
    connect(&MinosLoggerEvents::mle, SIGNAL(UpdateMemories(BaseContestLog*)), this, SLOT(onUpdateMemories(BaseContestLog*)), Qt::QueuedConnection);
    connect(&MinosLoggerEvents::mle, SIGNAL(refreshStackMults(BaseContestLog *)), this, SLOT(onRefreshStackMults(BaseContestLog *)));

    connect(&MinosLoggerEvents::mle, SIGNAL(clearContestInFrame(BaseContestLog *)), this, SLOT(clearContestInFrame(BaseContestLog *)));
}

StackedInfoFrame::~StackedInfoFrame()
{
    trace(QString("Deleting %1").arg(ui->infoCombo->currentText()));
    delete ui;
}
void StackedInfoFrame::setCurrentFrameType(QString s)
{
    ui->infoCombo->setCurrentText(s);
}

void StackedInfoFrame::on_infoCombo_currentIndexChanged(int /*arg1*/)
{
    if (currStackFrame)
    {
        layout()->removeWidget(currStackFrame);
        currStackFrame->deleteLater();
        currStackFrame = nullptr;
    }

    clockFrame = nullptr;
    dxccFrame = nullptr;
    districtFrame = nullptr;
    filterFrame = nullptr;
    rigMemFrame = nullptr;
    locFrame = nullptr;
    locTreeFrame = nullptr;
    statsFrame = nullptr;



    switch ( getAuxEntryType(ui->infoCombo->currentText()) )
    {
    case aeClock:
        clockFrame = new TClockFrame(this);
        currStackFrame = clockFrame;
        layout()->addWidget(currStackFrame);
        clockFrame->setContest(contest);
        break;

    case aeDXCC:
        dxccFrame = new DXCCFrame(this);
        currStackFrame = dxccFrame;
        layout()->addWidget(currStackFrame);
        dxccFrame->setContest(contest);
        break;

    case aeDistrict:
        districtFrame = new DistrictFrame(this);
        currStackFrame = districtFrame;
        layout()->addWidget(districtFrame);
        districtFrame->setContest(contest);
        break;

    case aeFilter:
        filterFrame = new FilterFrame(this);
        currStackFrame = filterFrame;
        layout()->addWidget(filterFrame);
        filterFrame->setContest(contest);
        break;

    case aeMemories:
        rigMemFrame = new RigMemoryFrame(this);
        currStackFrame = rigMemFrame;
        layout()->addWidget(rigMemFrame);
        rigMemFrame->setContest(contest);
        break;

    case aeLocatorMap:
        locFrame = new LocFrame(this);
        currStackFrame = locFrame;
        layout()->addWidget(locFrame);
        locFrame->setContest(contest);
        break;

    case aeLocatorTree:
        locTreeFrame = new LocTreeFrame(this);
        currStackFrame = locTreeFrame;
        layout()->addWidget(locTreeFrame);
        locTreeFrame->setContest(contest);
        break;

    case aeStats:
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

        if (contest)
        {
            if (stackInstance < STACKITEMS)
            {
                QString aux = contest->currentStackItems[stackInstance].getValue();
                if (!aux.isEmpty())
                {
                    ui->infoCombo->setCurrentText(aux);
                }
            }
        }
    }
}
void StackedInfoFrame::clearContestInFrame(BaseContestLog *ct)
{
    if (contest == ct)
    {
        setContest(nullptr);
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
