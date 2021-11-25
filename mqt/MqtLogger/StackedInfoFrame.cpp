#include "base_pch.h"
#include <QTabBar>
#include "StackedInfoFrame.h"
#include "tlogcontainer.h"
#include "tsinglelogframe.h"
#include "LoggerContest.h"

#include "ui_StackedInfoFrame.h"

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
    for(auto const &opt: qAsConst(auxoptions))
    {
        if (tr(opt.s) == s || (opt.s == s))
            return opt.type;
    }
    return aeClock;
}

const char * StackedInfoFrame::getRawAuxTypeString(AuxEntries t)
{
    for(auto const &opt: qAsConst(auxoptions))
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

    BandList &blist = BandList::getBandList();

    for (const auto &b:qAsConst(blist.bandList))
    {
       bool hf = b->getType() == HF_BANDTYPE;
       if (hf && b->enabled)
       {
           ui->tabbar->addTab(b->uk);
       }
    }
    ui->tabbar->setElideMode(Qt::ElideRight);
    connect( ui->tabbar , &QTabBar::currentChanged, this, &StackedInfoFrame::on_currentTabChangedSlot );

    ui->infoCombo->clear();
    int i = 0;
    for(auto const &opt:qAsConst( auxoptions))
    {
        ui->infoCombo->addItem(tr(opt.s), opt.type);
        ui->infoCombo->setItemData( i++, tr(opt.hint), Qt::ToolTipRole );
    }

    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::ScrollToCountry, this, &StackedInfoFrame::on_ScrollToCountry, Qt::QueuedConnection);
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::ScrollToDistrict, this, &StackedInfoFrame::on_ScrollToDistrict, Qt::QueuedConnection);
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::FontChanged, this, &StackedInfoFrame::on_FontChanged, Qt::QueuedConnection);
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::FiltersChanged, this, &StackedInfoFrame::onFiltersChanged, Qt::QueuedConnection);
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::UpdateStats, this, &StackedInfoFrame::onUpdateStats, Qt::QueuedConnection);
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::UpdateMemories, this, &StackedInfoFrame::onUpdateMemories, Qt::QueuedConnection);
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::refreshStackMults, this, &StackedInfoFrame::onRefreshStackMults);

    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::clearContestInFrame, this, &StackedInfoFrame::clearContestInFrame);
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::ContestBandChanged, this, &StackedInfoFrame::onContestBandChanged);

    connect(ui->infoCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &StackedInfoFrame::onInfoComboCurrentIndexChanged);

    QString n = QString("stackframe%1").arg(instance);
    setObjectName(n);
    //setStyleSheet(QString(" #%1 { border: 2px solid red; }").arg(n));

}

StackedInfoFrame::~StackedInfoFrame()
{
    delete ui;
}
void StackedInfoFrame::on_currentTabChangedSlot(int index)
{
    QString cb = ui->tabbar->tabText(index);

    if (dxccFrame)
        dxccFrame->setBand(cb);
    else if (districtFrame)
        districtFrame->setBand(cb);
    else if (statsFrame)
        statsFrame->setBand(cb);
    else if (locFrame)
        locFrame->setBand(cb);
    else if (locTreeFrame)
        locTreeFrame->setBand(cb);
}
void StackedInfoFrame::setCurrentFrameType(QString s)
{
    int n = getAuxEntryType(s);
    if (ui->infoCombo->currentIndex() == n)
    {
        if (!currStackFrame)
        {
            onInfoComboCurrentIndexChanged(n);
        }
    }
    else
    {
        ui->infoCombo->setCurrentIndex(n);
    }
}

void StackedInfoFrame::onInfoComboCurrentIndexChanged(int /*arg1*/)
{
    if (!contest)
    {
        return;
    }

    if (currStackFrame)
    {
        layout()->removeWidget(currStackFrame);
        currStackFrame->deleteLater();
        currStackFrame = nullptr;
    }

    delete clockFrame;
    delete dxccFrame;
    delete districtFrame;
    delete filterFrame;
    delete rigMemFrame;
    delete locFrame;
    delete locTreeFrame;
    delete statsFrame;

    clockFrame = nullptr;
    dxccFrame = nullptr;
    districtFrame = nullptr;
    filterFrame = nullptr;
    rigMemFrame = nullptr;
    locFrame = nullptr;
    locTreeFrame = nullptr;
    statsFrame = nullptr;

    bool setTabsVisible = contest->contestBands.getValue() == allHF;

    QString a = ui->infoCombo->currentText();

    switch ( getAuxEntryType(a) )
    {
    case aeClock:
        clockFrame = new TClockFrame(this);
        currStackFrame = clockFrame;
        layout()->addWidget(currStackFrame);
        clockFrame->setContest(contest);
        setTabsVisible = false;
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
        setTabsVisible = false;
        break;

    case aeMemories:
        rigMemFrame = new RigMemoryFrame(this);
        currStackFrame = rigMemFrame;
        layout()->addWidget(rigMemFrame);
        rigMemFrame->setContest(contest);
        setTabsVisible = false;
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
        setTabsVisible = false;
        break;
    }

    ui->tabbar->setVisible(setTabsVisible);
    ui->tabbar->setFocusPolicy(Qt::NoFocus);

    if (contest && stackInstance < STACKITEMS)
    {
        QString a = ui->infoCombo->currentText();
        AuxEntries ae = getAuxEntryType(a);
        contest->currentStackItems[stackInstance].setValue(auxoptions[ae].s);
        contest->commonSave(false);
    }
}



void StackedInfoFrame::setContest(LoggerContestLog *ct)
{
    if (contest != ct)
    {
        contest = ct;

        if (filterFrame)
        {
            filterFrame->setContest(contest);
        }
        else if (dxccFrame)
            dxccFrame->setContest(contest);
        else if (districtFrame)
            districtFrame->setContest(contest);
        else if (statsFrame)
            statsFrame->setContest(contest);
        else if (locFrame)
            locFrame->setContest(contest);
        else if (locTreeFrame)
            locTreeFrame->setContest(contest);
        else if (clockFrame)
        {
            clockFrame->setContest(contest);
        }
        else if (rigMemFrame)
        {
            rigMemFrame->setContest(contest);
        }

        if (contest && !contest->isReadOnly())
        {
            if (stackInstance < STACKITEMS)
            {
                QString aux = contest->currentStackItems[stackInstance].getValue();
                AuxEntries ae = getAuxEntryType(aux);   // contest setting

                QString a = ui->infoCombo->currentText();
                AuxEntries aec = getAuxEntryType(a);    // screen setting - from layout

                if(ct->currentStackItemsValid && ae != aec && ui->infoCombo->currentIndex() != ae)
                {
                    // set to contest value
                    ui->infoCombo->setCurrentIndex(ae);
                }
                else
                {
                    onInfoComboCurrentIndexChanged(-1);
                }
            }
        }
        onContestBandChanged(ct);
    }
}
void StackedInfoFrame::clearContestInFrame(BaseContestLog *ct)
{
    if (contest == ct)
    {
        trace("StackedInfoFrame::clearContestInFrame");
        setContest(nullptr);
    }
}

void StackedInfoFrame::onContestBandChanged(BaseContestLog *ct)
{
    if (ct && contest == ct)
    {
        for (int i = 0; i < ui->tabbar->count(); i++)
        {
            if (ui->tabbar->tabText(i) == ct->currentBand.getValue())
            {
                ui->tabbar->setCurrentIndex(i);
                break;
            }
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
           districtFrame->scrollToDistrict( dist->districtCode, true );
        }
        else
        {
            districtFrame->scrollToDistrict(QString(), false );
        }
    }
}

void StackedInfoFrame::on_ScrollToCountry( const QString &csCs, BaseContestLog *c )
{
    if (contest && contest == c && dxccFrame)
    {
        Callsign cs;
        cs.setFullCall( csCs );

        QSharedPointer<CountryEntry> ctryMult = findCtryPrefix( cs );
        if ( ctryMult )
        {
           dxccFrame->scrollToCountry( ctryMult->getBasePrefix(), true );
        }
        else
        {
            dxccFrame->scrollToCountry( QString(), false );
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
