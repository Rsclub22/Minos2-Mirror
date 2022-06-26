////////////////////////////////////////////////////////////////////////////
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Bandmap Client
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2019
//
/////////////////////////////////////////////////////////////////////////////

#include "MinosRPC.h"
#include "cutils.h"
#include "MinosLoggerEvents.h"
#include "tlogcontainer.h"
#include "SendRPCDM.h"
#include "tsinglelogframe.h"
#include "checkoperatingfreq.h"
#include "BandList.h"
#include "delayedaction.h"
#include "rigutils.h"
#include "ui_bandmapclientframe.h"
#include "bandmapclientframe.h"

BandmapClientFrame::BandmapClientFrame(QWidget *parent):
    QFrame(parent),
    ui(new Ui::BandmapClientFrame)
{

    ui->setupUi(this);

    ui->mouseInFrameLabel->setVisible(false);
    clusterStatusIndicatorToggle(false);
    radioStatusIndicatorToggle(false);
    ui->radioStatusMsg->clear();

    connect (ClusterClientServer::getClusterClientServer(), &ClusterClientServer::ClusterServerList, this, &BandmapClientFrame::clusterClientServerList);
    connect (ClusterClientServer::getClusterClientServer(), &ClusterClientServer::dxSpot, this, &BandmapClientFrame::dxSpots);

    bandmapDataModel = new BandmapDataModel();

    bandmapView = new BandmapView(this);
    bandmapView->move(-100, -100);
    bandmapView->resize(1, 1);
    bandmapView->setFilterSettings(&filterSettings);

    bandmapSpotProxyModel = new BandmapSortFilterProxyModel(parent);
    bandmapSpotProxyModel->setSourceModel(bandmapDataModel);
    bandmapView->setModel(bandmapSpotProxyModel);

    ui->textFilterEdit->setValidator(&ucValidator);

    bandmapView->initBandmapView(ui->bandmapGraphicsView);


    checkNewSpotsTimer = new QTimer(this);
    connect (checkNewSpotsTimer, &QTimer::timeout, this, &BandmapClientFrame::timerCheckNewBandMapSpots);
    checkNewSpotsTimer->start(CHECKSPOTS_DURATION);

    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::bandMapLimitsChanged, this, &BandmapClientFrame::on_bandmapLimitsChanged);
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::FontChanged, this, &BandmapClientFrame::on_FontChanged, Qt::QueuedConnection);
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::AfterLogContactToBandmap, this, &BandmapClientFrame::on_AfterLogContact);
    connect(bandmapView, &BandmapView::contextMenuSelected, this, &BandmapClientFrame::on_contextMenuSelected);
    connect(bandmapView, &BandmapView::newZoomlevel, this, &BandmapClientFrame::on_newZoomlevel);
    connect (ui->filtersPushBut, &QPushButton::clicked, this, &BandmapClientFrame::filterButtonSelected);

    purgeTimer = new QTimer(this);
    connect (purgeTimer, &QTimer::timeout, this, &BandmapClientFrame::purgeSpots);

    spotsMenu = new QMenu(ui->actionsButton);

    ui->actionsButton->setFocusPolicy(Qt::NoFocus);
    actionInObject = new BMP_MouseInObject(this, this);
    spotsMenu->installEventFilter(actionInObject);

    markSpotAction = new QAction(tr("M&ark Spot"), this);
    unMarkSpotAction = new QAction(tr("&Unmark Spot"), this);
    freqAction = new QAction(tr("Set &Freq"), this);
    bearingAction = new QAction(tr("Set &Bearing"), this);
    logAction = new QAction(tr("Send &Log"), this);
    memoryAction = new QAction(tr("Send &Memory"), this);
    saveZoomLevel = new QAction(tr("Save ZoomLevel"), this);
    readSavedZoomLevel = new QAction(tr("Read Saved ZoomLevel"), this);
    resendSpotsAction = new QAction(tr("&Resend Cluster Spots"), this);
    clearSpotAction = new QAction(tr("Clear &Spot"), this);
    clearAllSpotsAction = new QAction(tr("Clear All Spots"), this);
    clearClusterSpotsAction = new QAction(tr("Clear Cluster Spots"), this);

    spotsMenu->addAction(markSpotAction);
    spotsMenu->addAction(unMarkSpotAction);
    spotsMenu->addAction(freqAction);
    spotsMenu->addAction(bearingAction);
    spotsMenu->addAction(logAction);
    spotsMenu->addAction(memoryAction);
    spotsMenu->addAction(saveZoomLevel);
    spotsMenu->addAction(readSavedZoomLevel);
    spotsMenu->addAction(resendSpotsAction);
    spotsMenu->addAction(clearSpotAction);
    spotsMenu->addAction(clearAllSpotsAction);
    spotsMenu->addAction(clearClusterSpotsAction);

    ui->actionsButton->setMenu(spotsMenu);
    connect(spotsMenu, &QMenu::aboutToShow, this, &BandmapClientFrame::onMenuShow);

    zoomIn = new QShortcut(QKeySequence("Ctrl+<"), parent);   // Ctrl <
    connect(zoomIn, &QShortcut::activated, this, &BandmapClientFrame::on_zoomIn);

    zoomOut = new QShortcut(QKeySequence("Ctrl+>"), parent);   // Ctrl >
    connect(zoomOut, &QShortcut::activated, this, &BandmapClientFrame::on_zoomOut);

    connect( markSpotAction, &QAction::triggered, this, &BandmapClientFrame::on_markSpotActionSelected );
    connect( unMarkSpotAction, &QAction::triggered, this, &BandmapClientFrame::on_unMarkSpotActionSelected );
    connect( freqAction, &QAction::triggered, this, &BandmapClientFrame::on_freqActionSelected );
    connect( bearingAction, &QAction::triggered, this, &BandmapClientFrame::on_bearingActionSelected );
    connect( logAction, &QAction::triggered, this, &BandmapClientFrame::on_logActionSelected );
    connect( memoryAction, &QAction::triggered, this, &BandmapClientFrame::on_memoryActionSelected );
    connect( saveZoomLevel, &QAction::triggered, this, &BandmapClientFrame::on_saveZoomLevelActionSelected );
    connect( readSavedZoomLevel, &QAction::triggered, this, &BandmapClientFrame::on_readZoomLevelActionSelected );
    connect(resendSpotsAction, &QAction::triggered, this, &BandmapClientFrame::on_resendClusterSpotSelected);
    connect( clearSpotAction, &QAction::triggered, this, &BandmapClientFrame::on_clearSpotActionSelected );
    connect( clearAllSpotsAction, &QAction::triggered, this, &BandmapClientFrame::on_clearAllSpotsActionSelected );
    connect( clearClusterSpotsAction, &QAction::triggered, this, &BandmapClientFrame::on_clearClusterSpotsActionSelected );

    contextSpotsMenu = new QMenu(this);
    contextSpotsMenu_markSpotAction = new QAction(tr("M&ark Spot"), this);
    contextSpotsMenu_unMarkSpotAction = new QAction(tr("&Unmark Spot"), this);
    contextMoveFreqAction = new QAction(tr("Move spot to current frequency"));
    contextSpotsMenu_freqAction = new QAction(tr("Set &Freq"), this);
    contextSpotsMenu_bearingAction = new QAction(tr("Set &Bearing"), this);
    contextSpotsMenu_logAction = new QAction(tr("Send &Log"), this);
    contextSpotsMenu_memoryAction = new QAction(tr("Send &Memory"), this);
    contextSpotsMenu_clearSpotAction = new QAction(tr("Clear &Spot"), this);

    contextSpotsMenu->addAction(contextSpotsMenu_markSpotAction);
    contextSpotsMenu->addAction(contextSpotsMenu_unMarkSpotAction);
    contextSpotsMenu->addAction(contextMoveFreqAction);
    contextSpotsMenu->addAction(contextSpotsMenu_freqAction);
    contextSpotsMenu->addAction(contextSpotsMenu_bearingAction);
    contextSpotsMenu->addAction(contextSpotsMenu_logAction);
    contextSpotsMenu->addAction(contextSpotsMenu_memoryAction);
    contextSpotsMenu->addAction(contextSpotsMenu_clearSpotAction);

    connect( contextSpotsMenu_markSpotAction, &QAction::triggered, this, &BandmapClientFrame::context_markSpotActionSelected );
    connect( contextSpotsMenu_unMarkSpotAction, &QAction::triggered, this, &BandmapClientFrame::context_unMarkSpotActionSelected );
    connect( contextMoveFreqAction, &QAction::triggered, this, &BandmapClientFrame::context_moveFreqActionSelected );
    connect( contextSpotsMenu_freqAction, &QAction::triggered, this, &BandmapClientFrame::context_freqActionSelected );
    connect( contextSpotsMenu_bearingAction, &QAction::triggered, this, &BandmapClientFrame::context_bearingActionSelected );
    connect( contextSpotsMenu_logAction, &QAction::triggered, this, &BandmapClientFrame::context_logActionSelected );
    connect( contextSpotsMenu_memoryAction, &QAction::triggered, this, &BandmapClientFrame::context_memoryActionSelected );
    connect( contextSpotsMenu_clearSpotAction, &QAction::triggered, this, &BandmapClientFrame::context_clearSpotActionSelected );

    connect(this, &BandmapClientFrame::freqDisplayClicked, this, &BandmapClientFrame::on_FreqDisplayClicked);

    setMouseTracking(true);
    mouseInFrameTimer = new QTimer(this);
    connect (mouseInFrameTimer, &QTimer::timeout, this, &BandmapClientFrame::mouseTimerCheckNewSpots);

    BandList::getBandList().loadAllBands(bands);

    modeBandPlan = new checkModeAgainstFreq();
    if (modeBandPlan->loadBandsFromBandList())
    {
        traceMsg(QString("Mode frequency bandplan loaded OK"));
        modeBandPlanOk = true;

    }
    else
    {
        traceMsg(QString("Mode frequency bandplan loaded failed to Load"));
        modeBandPlanOk = false;

    }

    operatingFreqExclusions = new CheckOperatingFreq();
    if (operatingFreqExclusions->loadExclusionsFromBandList())
    {
        traceMsg(QString("Operating frequency bandplan loaded OK"));
        operatingFreqExclusionsPlanOk = true;
    }
    else
    {
        traceMsg(QString("Operating frequency bandplan failed to load"));
        operatingFreqExclusionsPlanOk = false;
    }

    ui->freqDisplay->installEventFilter(this);
    freqDisplayPalette = new QPalette();       // to change colour when tuning

    purgeTimer->start(PURGE_TIME);

    waitClusterServerLoadedTimer = new QTimer(this);

    // wait for clusterserver to load before asking for spots
    connect(waitClusterServerLoadedTimer, &QTimer::timeout, this, [=](){on_waitClusterServerLoadedTimeout();});
    waitClusterServerLoadedTimer->start(250);

    connect(ui->clusterStatusIndicator, &QPushButton::clicked, this, [=](){on_clusterStatusIndicatorClicked();});

    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::ContestBandChanged, this, &BandmapClientFrame::onContestBandChanged);

    ui->zoomSpinner->setMinimum(dialData::MIN_ZOOM_LEVEL);
    ui->zoomSpinner->setMaximum(dialData::MAX_ZOOM_LEVEL);

}

void BandmapClientFrame::onContestBandChanged(BaseContestLog *c)
{
    if (c && c == ct)
    {
        setContest(c);
    }
}

void BandmapClientFrame::on_zoomIn()
{
    bandmapView->updateZoom(true);
}

void BandmapClientFrame::on_zoomOut()
{
    bandmapView->updateZoom(false);
}

BandmapClientFrame::~BandmapClientFrame()
{

    delete ui;

    delete modeBandPlan;
    delete operatingFreqExclusions;
    delete bandmapDataModel;
    delete actionInObject;
    delete freqDisplayPalette;
    bandmapView->deleteLater();

}


void BandmapClientFrame::on_waitClusterServerLoadedTimeout()
{
    static int timeoutCount = 0;
    if (LogContainer->sendDM->isClusterServerLoaded())
    {
        waitClusterServerLoadedTimer->stop();
        on_resendClusterSpotSelected();

    }
    else
    {
        timeoutCount++;
        if (timeoutCount == 30 * 4)
        {
            //timed out
            waitClusterServerLoadedTimer->stop();
            traceMsg(QString("waitClusterServerLoadedTimed Out at %1 secs").arg(timeoutCount));
        }

    }
}


void BandmapClientFrame::on_FontChanged()
{
    QFont cf = QApplication::font();
    bandmapView->onFontChanged(cf);
}

void BandmapClientFrame::on_bandmapLimitsChanged()
{
    setContest(ct);
}
// this is for the Tool button Action Menu
void BandmapClientFrame::onMenuShow()
{
    traceMsg(QString("Action Menu Selected - ViewRowNum %1").arg(bandmapView->getSelectedSpotViewRowNum()));
}


void BandmapClientFrame::on_FiltersChanged(bool state)
{
    if (state)
    {
        trace("BandmapView::bandmapUpdate() on_FiltersChanged");
        bandmapView->bandmapUpdate(true);
    }
}

void BandmapClientFrame::on_resendClusterSpotSelected()
{
    if (ct  && !contestBandStr.isEmpty())
    {
        MinosLoggerEvents::SendRequestResendSpotsToClusterServer(resendFrameId::BANDMAP_CLIENT, RESEND_ALL_SPOTS, contestBandStr, ct->uuid);
    }
}




void BandmapClientFrame::on_markSpotActionSelected()
{
    if (bandmapView->getSelectedSpotDataPtr()->getIsSelected())
    {
        traceMsg(QString("menu mark spot selected for callsign %1").arg(bandmapView->getSelectedSpotDataPtr()->getDxCallStr()));
        bandmapSpotType::SPOT_TYPE spotType = static_cast<bandmapSpotType::SPOT_TYPE>(bandmapSpotProxyModel->data(bandmapSpotProxyModel->index(bandmapView->getSelectedSpotDataRowNum(), SPOT_TYPE_COL_NUM), BMP_DataStoredRole).toInt());
        if (spotType == bandmapSpotType::CLUSTER)
        {
            bandmapSpotProxyModel->setData(bandmapSpotProxyModel->index(bandmapView->getSelectedSpotDataRowNum(), SPOT_TYPE_COL_NUM), bandmapSpotType::CLUSTER_MARKED, BMP_DataStoredRole);
            bandmapView->bandmapUpdate(true);
        }
    }
}

void BandmapClientFrame::on_unMarkSpotActionSelected()
{
    if (bandmapView->getSelectedSpotDataPtr()->getIsSelected())
    {
        traceMsg(QString("menu unmark spot selected for callsign %1").arg(bandmapView->getSelectedSpotDataPtr()->getDxCallStr()));
        bandmapSpotType::SPOT_TYPE spotType = static_cast<bandmapSpotType::SPOT_TYPE>(bandmapSpotProxyModel->data(bandmapSpotProxyModel->index(bandmapView->getSelectedSpotDataRowNum(), SPOT_TYPE_COL_NUM), BMP_DataStoredRole).toInt());
        if (spotType == bandmapSpotType::CLUSTER_MARKED)
        {
            bandmapSpotProxyModel->setData(bandmapSpotProxyModel->index(bandmapView->getSelectedSpotDataRowNum(), SPOT_TYPE_COL_NUM), bandmapSpotType::CLUSTER, BMP_DataStoredRole);
            bandmapView->bandmapUpdate(true);
        }
    }
}

void BandmapClientFrame::on_FreqDisplayClicked()
{
    bandmapView->makeCursorVisibleInBandmap();
}

void BandmapClientFrame::on_freqActionSelected()
{
    if (bandmapView->getSelectedSpotDataPtr()->getIsSelected())
    {
        traceMsg(QString("menu freq selected for callsign %1, freq %2").arg(bandmapView->getSelectedSpotDataPtr()->getDxCallStr(), bandmapView->getSelectedSpotDataPtr()->getFreq().traceStr()));
         Frequency freq = bandmapView->getSelectedSpotDataPtr()->getFreq();
         sendFreqToRig(freq);
    }
}

void BandmapClientFrame::sendFreqToRig(Frequency freq)
{
    MinosLoggerEvents::SendFreqToRig(freq);
}

void BandmapClientFrame::on_bearingActionSelected()
{
    if (bandmapView->getSelectedSpotDataPtr()->getIsSelected())
    {
        traceMsg(QString("menu bearing selected for callsign %1, bearing %2").arg(bandmapView->getSelectedSpotDataPtr()->getDxCallStr(), bandmapView->getSelectedSpotDataPtr()->getRotBrg()));
        QString brg = bandmapView->getSelectedSpotDataPtr()->getDxBrg();
        QString loc = bandmapView->getSelectedSpotDataPtr()->getDxLocator();
        if (!brg.isEmpty())
        {
            if (loc.count() < 6)
            {
                brg = brg.append(SHORTLOCATOR_IDENTIFIER);

            }
            sendBrgToRot(brg);
        }
    }
}

void BandmapClientFrame::sendBrgToRot(QString brg)
{
    if (!brg.isEmpty())
    {
       MinosLoggerEvents::SendSpotBrgStrToRot(brg);
    }
}

void BandmapClientFrame::on_logActionSelected()
{
    if (bandmapView->getSelectedSpotDataPtr()->getIsSelected())
    {
        traceMsg(QString("menu send to log selected for callsign %1").arg(bandmapView->getSelectedSpotDataPtr()->getDxCallStr()));

        memoryData::memData spotData;
        spotData.callsign = bandmapView->getSelectedSpotDataPtr()->getDxCallStr();
        spotData.time = bandmapView->getSelectedSpotDataPtr()->getSpotTime();
        spotData.freq = bandmapView->getSelectedSpotDataPtr()->getFreq();

        bool showDerivedLocFlag;
        TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpShowDerivedLoc, showDerivedLocFlag );

        if (showDerivedLocFlag || !bandmapView->getSelectedSpotDataPtr()->getDxLocatorIsFromNode())
        {
            spotData.locator = bandmapView->getSelectedSpotDataPtr()->getDxLocator();
        }

        spotData.bearing = bandmapView->getSelectedSpotDataPtr()->getDxBrg().toInt();
        spotData.exchange = bandmapView->getSelectedSpotDataPtr()->getDistrict();
        spotData.fromBandmapOrMemory = true;

        MinosLoggerEvents::SendSpotToLog(spotData);
    }
}

void BandmapClientFrame::on_memoryActionSelected()
{
    if (bandmapView->getSelectedSpotDataPtr()->getIsSelected())
    {
        traceMsg(QString("menu send to memory selected for callsign %1").arg(bandmapView->getSelectedSpotDataPtr()->getDxCallStr()));

        memoryData::memData spotData;
        spotData.callsign = bandmapView->getSelectedSpotDataPtr()->getDxCallStr();
        spotData.time = bandmapView->getSelectedSpotDataPtr()->getSpotTime();
        spotData.freq = bandmapView->getSelectedSpotDataPtr()->getFreq();

        bool showDerivedLocFlag;
        TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpShowDerivedLoc, showDerivedLocFlag );

        if (showDerivedLocFlag || !bandmapView->getSelectedSpotDataPtr()->getDxLocatorIsFromNode())
        {
            spotData.locator = bandmapView->getSelectedSpotDataPtr()->getDxLocator();
        }

        spotData.bearing = bandmapView->getSelectedSpotDataPtr()->getDxBrg().toInt();
        spotData.dxLocFromNode = bandmapView->getSelectedSpotDataPtr()->getDxLocatorIsFromNode();
        spotData.exchange = bandmapView->getSelectedSpotDataPtr()->getDistrict();

        MinosLoggerEvents::SendSpotToMemory(ct,spotData);
    }
}

void BandmapClientFrame::on_clearSpotActionSelected()
{
    if (bandmapView->getSelectedSpotDataPtr()->getIsSelected())
    {
        int ret = QMessageBox::warning(this, tr("Bandmap"),
                                       tr("Please confirm you want to delete this spot - %1?").arg(bandmapView->getSelectedSpotDataPtr()->getDxCallStr()),
                                       QMessageBox::Yes | QMessageBox::No);
        if (ret == QMessageBox::Yes)
        {
            traceMsg(QString("menu clear spot selected for callsign %1").arg(bandmapView->getSelectedSpotDataPtr()->getDxCallStr()));
            bandmapSpotProxyModel->removeRows(bandmapView->getSelectedSpotDataRowNum(), 1);
            bandmapView->clearSelectedSpotData();
            bandmapView->bandmapUpdate(true);
        }
    }
}

void BandmapClientFrame::on_clearAllSpotsActionSelected()
{
    if (bandmapSpotProxyModel->rowCount() > 0)
    {
        int ret = QMessageBox::warning(this, tr("Bandmap"),
                                       tr("Please confirm you want to delete all the spots in the bandmap?"),
                                       QMessageBox::Yes | QMessageBox::No);
        if (ret == QMessageBox::Yes)
        {
            traceMsg(QString("menu clear all bandmap spots selected"));
            bandmapSpotProxyModel->removeRows(0, bandmapSpotProxyModel->rowCount(), QModelIndex());
            bandmapView->clearSelectedSpotData();
            bandmapView->bandmapUpdate(true);
        }
    }
}

void BandmapClientFrame::on_clearClusterSpotsActionSelected()
{
    if (bandmapSpotProxyModel->rowCount() > 0)
    {
        int ret = QMessageBox::warning(this, tr("Bandmap"),
                                       tr("Please confirm you want to delete all the cluster spots in the bandmap?"),
                                       QMessageBox::Yes | QMessageBox::No);
        if (ret == QMessageBox::Yes)
        {
            traceMsg(QString("menu clear cluster bandmap spots selected"));

            for (int row = 0; row < bandmapDataModel->rowCount(); row++)
            {
                bandmapSpotType::SPOT_TYPE savedSpotType = static_cast<bandmapSpotType::SPOT_TYPE>(bandmapDataModel->data(bandmapDataModel->index(row, SPOT_TYPE_COL_NUM ),  BMP_DataStoredRole).toInt());
                if (savedSpotType == bandmapSpotType::CLUSTER )
                {
                    // delete the old logged/saved entry, add the new one
                    bandmapDataModel->setData(bandmapDataModel->index(row, SPOT_TYPE_COL_NUM ), bandmapSpotType::DELETED, BMP_DataStoredRole);
                }
            }
            purgeSpots();
            bandmapView->clearSelectedSpotData();
            bandmapView->bandmapUpdate(true);
        }
    }
}

void BandmapClientFrame::on_contextMenuSelected(const QPoint& pos, const QPoint& mapP)
{
    int contextSelectedSpotViewRowNum = bandmapView->isClickInRegionOfSpot(mapP);
    traceMsg(QString("Context Menu Selected - ViewRowNum %1").arg(contextSelectedSpotViewRowNum));

    if (contextSelectedSpotViewRowNum != NO_SELECTED_ROWNUM)
    {
        bandmapView->getSpotData(contextMenuSelectedSpotDataRowNum, contextSelectedSpotViewRowNum, contextMenuSelectedSpotData);
        if (contextMenuSelectedSpotData.getSpotType() == bandmapSpotType::CQ)
        {
            traceMsg(QString("Context Menu Selected - ViewRowNum %1 - Error Selected CQ Marker").arg(contextSelectedSpotViewRowNum));
            return;
        }
        QPoint globalPos = ui->bandmapGraphicsView->viewport()->mapToGlobal( pos );
        contextSpotsMenu->popup(globalPos);
    }
}


void BandmapClientFrame::context_markSpotActionSelected()
{
    traceMsg(QString("menu mark spot selected for callsign %1").arg(contextMenuSelectedSpotData.getDxCallStr()));
    bandmapSpotType::SPOT_TYPE spotType = static_cast<bandmapSpotType::SPOT_TYPE>(bandmapSpotProxyModel->data(bandmapSpotProxyModel->index(contextMenuSelectedSpotDataRowNum, SPOT_TYPE_COL_NUM), BMP_DataStoredRole).toInt());
    if (spotType == bandmapSpotType::CLUSTER)
    {
        bandmapSpotProxyModel->setData(bandmapSpotProxyModel->index(contextMenuSelectedSpotDataRowNum, SPOT_TYPE_COL_NUM), bandmapSpotType::CLUSTER_MARKED, BMP_DataStoredRole);
        bandmapView->bandmapUpdate(true);
    }
}

void BandmapClientFrame::context_unMarkSpotActionSelected()
{
    traceMsg(QString("menu unmark spot selected for callsign %1").arg(contextMenuSelectedSpotData.getDxCallStr()));
    bandmapSpotType::SPOT_TYPE spotType = static_cast<bandmapSpotType::SPOT_TYPE>(bandmapSpotProxyModel->data(bandmapSpotProxyModel->index(contextMenuSelectedSpotDataRowNum, SPOT_TYPE_COL_NUM), BMP_DataStoredRole).toInt());
    if (spotType == bandmapSpotType::CLUSTER_MARKED)
    {
        bandmapSpotProxyModel->setData(bandmapSpotProxyModel->index(contextMenuSelectedSpotDataRowNum, SPOT_TYPE_COL_NUM), bandmapSpotType::CLUSTER, BMP_DataStoredRole);
        bandmapView->bandmapUpdate(true);
    }
}

void BandmapClientFrame::context_moveFreqActionSelected()
{
    traceMsg(QString("menu move frequency of spot selected for callsign %1").arg(contextMenuSelectedSpotData.getDxCallStr()));

    QVariant f;
    f.setValue(curFreq);
    bandmapSpotProxyModel->setData(bandmapSpotProxyModel->index(contextMenuSelectedSpotDataRowNum, FREQ_COL_NUM), f, BMP_DataStoredRole);
    bandmapDataModel->sortModel();
    bandmapView->bandmapUpdate(true);

}

void BandmapClientFrame::context_freqActionSelected()
{
    traceMsg(QString("menu freq selected for callsign %1, freq %2").arg(contextMenuSelectedSpotData.getDxCallStr(), contextMenuSelectedSpotData.getFreq().traceStr()));
     Frequency freq = contextMenuSelectedSpotData.getFreq();
     sendFreqToRig(freq);
}

void BandmapClientFrame::context_bearingActionSelected()
{
    traceMsg(QString("menu bearing selected for callsign %1, bearing %2").arg(contextMenuSelectedSpotData.getDxCallStr(), contextMenuSelectedSpotData.getRotBrg()));
    QString brg = contextMenuSelectedSpotData.getDxBrg();
    QString loc = contextMenuSelectedSpotData.getDxLocator();
    if (!brg.isEmpty())
    {
        if (loc.count() < 6)
        {
            brg = brg.append(SHORTLOCATOR_IDENTIFIER);
        }
        sendBrgToRot(brg);
        traceMsg(QString("Bandmap Context Menu Send to Rotator, locator bearing = %1").arg(brg));
    }
    else if (!contextMenuSelectedSpotData.getRotBrg().isEmpty())
    {
        sendBrgToRot(contextMenuSelectedSpotData.getRotBrg());
        traceMsg(QString("Bandmap Context Menu Send to Rotator, rotator bearing = %1").arg(brg));
    }
}

void BandmapClientFrame::context_logActionSelected()
{
    traceMsg(QString("menu send to log selected for callsign %1").arg(contextMenuSelectedSpotData.getDxCallStr()));

    memoryData::memData spotData;
    spotData.callsign = contextMenuSelectedSpotData.getDxCallStr();
    spotData.time = contextMenuSelectedSpotData.getSpotTime();
    spotData.freq = contextMenuSelectedSpotData.getFreq();

    bool showDerivedLocFlag;
    TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpShowDerivedLoc, showDerivedLocFlag );

    if (showDerivedLocFlag || !bandmapView->getSelectedSpotDataPtr()->getDxLocatorIsFromNode())
    {
        spotData.locator = contextMenuSelectedSpotData.getDxLocator();
    }

    spotData.bearing = contextMenuSelectedSpotData.getDxBrg().toInt();
    spotData.fromBandmapOrMemory = true;
    spotData.exchange = bandmapView->getSelectedSpotDataPtr()->getDistrict();

    MinosLoggerEvents::SendSpotToLog(spotData);
}

void BandmapClientFrame::context_memoryActionSelected()
{
    traceMsg(QString("menu send to memory selected for callsign %1").arg(contextMenuSelectedSpotData.getDxCallStr()));

    memoryData::memData spotData;
    spotData.callsign = contextMenuSelectedSpotData.getDxCallStr();
    spotData.time = contextMenuSelectedSpotData.getSpotTime();
    spotData.freq = contextMenuSelectedSpotData.getFreq();
    spotData.locator = contextMenuSelectedSpotData.getDxLocator();
    spotData.bearing = contextMenuSelectedSpotData.getDxBrg().toInt();
    spotData.dxLocFromNode = contextMenuSelectedSpotData.getDxLocatorIsFromNode();
    spotData.exchange = bandmapView->getSelectedSpotDataPtr()->getDistrict();

    MinosLoggerEvents::SendSpotToMemory(ct, spotData);
}

void BandmapClientFrame::context_clearSpotActionSelected()
{
    int ret = QMessageBox::warning(this, tr("Bandmap"),
                                   tr("Please confirm you want to delete this spot - %1?").arg(contextMenuSelectedSpotData.getDxCallStr()),
                                   QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes)
    {
        traceMsg(QString("menu clear spot selected for callsign %1").arg(contextMenuSelectedSpotData.getDxCallStr()));
        bandmapSpotProxyModel->removeRows(contextMenuSelectedSpotDataRowNum, 1);
    }
}

void BandmapClientFrame::on_saveZoomLevelActionSelected()
{
    int level = bandmapView->getDialZoomLevel();
    saveBandmapZoomLevel(level);
}

void BandmapClientFrame::on_readZoomLevelActionSelected()
{
    // Operating directly from the action seems to produce the wrong effect
    // The highest frequency is off the bottom, and not dispayed as text
    delayedAction(this, [=]{
        int zoomLevel = readBandmapZoomLevel();
        bandmapView->setBandmapZoom(zoomLevel);
        });
}
// end of actions
//============================================================================
void BandmapClientFrame::ShowFilter()
{
    QString ft;
    bool ignoreDistance = filterSettings.getIgnoreDistanceFlag();
    if (!ignoreDistance)
    {
        ft = tr("Filter %1").arg(filterSettings.getDistanceFilter());
    }
    ui->filterText->setText(ft);
}

void BandmapClientFrame::setContest(BaseContestLog *c)
{
    ct = c;
    if (ct == nullptr)
    {
        return;
    }

    LoggerContestLog* contest = dynamic_cast<LoggerContestLog *>( ct);

    if (bandmapView)
    {
        bandmapView->setContest(ct);
    }

    traceMsg(QString("Set Contest: contest uuid =  ContestUuid = %1").arg(ct->uuid));

    setContestBandMode(ct->currentBand.getValue(), ct->currentMode.getValue());

    if (bandmapView && operatingFreqExclusionsPlanOk)
    {
        // send operating freq to dial
        bandmapView->setFreqOperatingInfo(contestBandStr, contestModeStr, operatingFreqExclusions, operatingFreqExclusionsPlanOk);
    }

    if (!contestBandStr.isEmpty())
    {

        if (!contest->bandmapFilterSettingsExist)       // have settings been saved before?
        {
            readDefaultDistanceFilterSettings(&filterSettings);

            //set current mode
            if (contestModeStr == hamlibData::MGM)       //  have mode settings been saved before?
            {
                for (auto &m: mgmModes)
                {
                    filterSettings.setModeFilter(m, true); // set all the mgm modes in filter

                }
            }
            else if (contestMode >= 0)
            {
                // no, save current mode filter for this contest
                filterSettings.setModeFilter(contestModeStr, true);
            }

            //filterSetup->saveBandmapFilterToContest();
            contest->saveBandmapFilter(filterSettings);
        }
        else
        {
            filterSettings = contest->getBandmapFilter();
        }


    }
    ShowFilter();
}


void BandmapClientFrame::readDefaultDistanceFilterSettings(BandmapClientFilterSettings *filterSettings)
{

    ClusterFilterDefaultDistIniName defaultDistIniNames;
    defaultDistIniNames.initClusterFilterIdAndNames(bands);

    QSettings config(CLUSTER_FILTER_FILE, QSettings::IniFormat);
    config.beginGroup("Default Distance");

    filterSettings->setDistanceFilter(config.value(defaultDistIniNames.getDefaultDistIniName(contestBandStr).defaultDistanceName, DEFAULT_FILTER_DISTANCE).toInt());

    config.endGroup();
}



void BandmapClientFrame::getBandLimitsFromBandListXML()
{
    // use band list file
    BandList blist = BandList::getBandList();
    QSharedPointer<BandInfo>  bi;

    for(const auto &bi: qAsConst(blist.bandList))
    {
        if (bi->uk == contestBandStr)
        {
            QSettings config(BAND_LIST_INI, QSettings::IniFormat);
            contestBandFlow = convertFreqToFullDigit(config.value( BAND_LIST_SECT_FREQ_LOW + "/" + bi->uk, bi->bandmapLow.convertFreqStrDispSingle() ).toString());
            contestBandFHigh = convertFreqToFullDigit(config.value( BAND_LIST_SECT_FREQ_HIGH + "/" + bi->uk, bi->bandmapHigh.convertFreqStrDispSingle() ).toString());

            bandmapView->setBandFreqLimits(contestBandFlow, contestBandFHigh);
            bandmapView->setBandmapHeight(contestBandFlow, contestBandFHigh);
            traceMsg(QString("contestBand Freq low = %1, contestBand Freq high = %2").arg(contestBandFlow.traceStr(), contestBandFHigh.traceStr()));
            break;
        }
    }
}

int BandmapClientFrame::getModeOffSet(QString contestModeStr)
{
    int i = 0;
    while(i != clusterModes.count())
    {
        if (contestModeStr == clusterModes[i])
        {
            return i;
        }
        i++;
    }
    return -1;
}

// returns true if freq ok, false if it is not...it will return true and error to tracelog is mode or band
// is missing from operating freq file
bool BandmapClientFrame::isFreqLegal(const Frequency &freq, const QString band, const QString mode)
{
    int retCode;
    if (operatingFreqExclusionsPlanOk)
    {
            retCode =  operatingFreqExclusions->freqValid(band, mode, freq);
            switch (retCode)
            {
                case FREQ_NOT_OK:
                    return false;
                case FREQ_OK:
                    return true;
                case FREQ_NO_MATCH:
                    return false;
                case MODE_MISSING:
                    traceMsg(QString("isFreqLegal: mode is missing from file - band %1, mode %2").arg(band, mode));
                    return true;
                case BAND_MISSING:
                    traceMsg(QString("isFreqLegal: band is missing from file - band %1, mode %2").arg(band, mode));
                    return true;
            }
    }

    traceMsg(QString("isFreqLegal: Operating Freq file not loaded"));
    return true;
}

//---------------------- Cluster Spots -------------------------------------

void BandmapClientFrame::clusterClientServerList(QVector<ClusterServer> serverList)
{
    for ( QVector<ClusterServer>::iterator i = serverList.begin(); i != serverList.end(); i++ )
    {
        QString state = QString(clusterStateList[(*i).state]) + " " + (*i).app + "\r\n";
        traceMsg(QString("bandmapClientServerList - state = %1").arg(state));
    }
}

void BandmapClientFrame::dxSpots(QVector<ClusterMessage> spotMsg)
{
    // if contest is protected ignore
    if (ct && !ct->isReadOnly())
    {
        //get spot Message from queue
        for (int i = 0; i < spotMsg.count(); i++)
        {
            ClusterMessage msg = spotMsg[i];
            traceMsg(QString("retrieve cluster spot from queue - spot = %1 for loggeruuid = %2, this contest uuid = %3").arg(msg.getMessage(), msg.getLoggerUuid(), ct->uuid));

            // if loggerUuid is empty, message is for all frames
            if ((msg.getLoggerUuid().isEmpty() || msg.getLoggerUuid() == ct->uuid) && (msg.getFrameId() == resendFrameId::BANDMAP_CLIENT || msg.getFrameId() == resendFrameId::ALL_CLIENTS))
            {
                if (msg.getMessage().contains(DXSPOT) || msg.getMessage().contains(RESENTSPOT))
                {
                    traceMsg(QString("Spot for this loggeruuid = %1, add to queue").arg(ct->uuid));
                    QSharedPointer<BandmapSpotData> sp = stringToDxSpot(msg.getMessage());
                    if (sp)
                    {
                        spotQueue += sp;
                    }
                }
            }
        }
    }
}

void BandmapClientFrame::timerCheckNewBandMapSpots()
{
    if (ct && !purgeSpotFlag && !holdUpdateFlag)     // do nothing while purging spots
    {
        checkNewBandMapSpots();
    }
}
QSharedPointer<BandmapSpotData> BandmapClientFrame::stringToDxSpot(QString spot)
{
    QSharedPointer<BandmapSpotData> res;
    QDateTime spotDateTime = QDateTime::currentDateTimeUtc();

    QStringList sl;
    if (spot.contains(DXSPOT))
    {
       sl = spot.split(DXSPOT);
    }
    else if (spot.contains(RESENTSPOT))
    {
       sl = spot.split(RESENTSPOT);
    }
    if (sl.count() == 2)
    {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        QStringList spotlist = sl[1].split(':', Qt::KeepEmptyParts);
#else
        QStringList spotlist = sl[1].split(':', QString::KeepEmptyParts);
#endif

        if (spotlist.count() == TTLVALUE +1)
        {
            bool ok = false;
            int ttl = spotlist[TTLVALUE].toInt(&ok);
            if (ok)
            {
                if (ttl >= MIN_TTL && ttl <= MAX_TTL)
                {
                    timeToLive = ttl * 60; // seconds
                }
            }

            //-------------------------------------------------------

            //timeToLive = 120; // for testing.....

            //--------------------------------------------------------

            // check to see if spot is for this contest band

            QString band = spotlist[DXBANDSTR];
            QString bandlist = ct->contestBands.getValue();
            if (bandlist == allHF)
            {
                BandList &blist = BandList::getBandList();
                QSharedPointer<BandInfo>  bi;
                bool bandOK = blist.findBand(band, bi);
                if (!bandOK || bi->getType() != HF_BANDTYPE)
                {
                   return res;
                }

            }
            else if (spotlist[DXBANDSTR] != contestBandStr)
            {
                return res;  // not for this contest band
            }

            // check to see if call or locator worked
            bool callWorked = false;
            bool locWorked = false;

            Callsign cs;
            cs.setFullCall(spotlist[DXCALL]);
            checkSpotWorked(cs, spotlist[DXLOCATOR], spotlist[DXFREQ], &callWorked, &locWorked);

            QString distance;
            QString bearing;
            if (!spotlist[DXLOCATOR].isEmpty())
            {
                double dist = 0;
                int brg = 0;
                calcSpotDistanceBearing(spotlist[DXLOCATOR], &dist, &brg);
                distance = QString::number(static_cast< int> ( dist));
                bearing =  QString::number(brg);
            }

            bool dxLocFromNodeFlag = extractDxLocFromNodeFlag(spotlist[DXLOC_FROM_NODE_FLAG]);

            spotDateTime = QDateTime::fromString(spotlist[SPOTDATETIME], "yyyyMMMddHHmmss" );
            spotDateTime.setTimeSpec(Qt::UTC);
            qint64 rxTime = spotDateTime.toMSecsSinceEpoch() / 1000;

            traceMsg(QString("Add Cluster Spot to Bandmap %1, %2, %3, %4").arg(spotlist[DXCALL], spotlist[DXFREQ], spotlist[DXMODESTR], spotlist[DXLOCATOR]));

            res = QSharedPointer<BandmapSpotData>(new BandmapSpotData(bandmapSpotType::CLUSTER));

            res->setRxTime(rxTime);
            res->setSpotDateTime(spotDateTime);
            res->setFreq(spotlist[DXFREQ]);
            res->setBand(spotlist[DXBANDSTR]);
            res->setMode(spotlist[DXMODESTR]);
            res->setDxCall(spotlist[DXCALL]);
            res->setDxCallWorked(callWorked);
            res->setDxLocator(spotlist[DXLOCATOR]);
            res->setDxLocatorIsFromNode(dxLocFromNodeFlag);
            res->setDxLocatorWorked(locWorked);
            res->setDxDist(distance);
            res->setDxBrg(bearing);
            res->setSpotterCall(spotlist[SPOTCALL]);
            res->setSpotterLocator(spotlist[SPOTLOCATOR]);
            res->setSpotComment(spotlist[SPOTCOMMENT]);
            res->setSpotType(bandmapSpotType::SPOT_TYPE::CLUSTER);
       }
    }
    return res;
}
void BandmapClientFrame::checkNewBandMapSpots()
{
    if (!ct || ct->isReadOnly())
        return;

    TSingleLogFrame * tslf = LogContainer ->findContest( ct );
    if (!tslf || !tslf->bandMapLoaded)
    {
        bool disableNotShown;
        TContestApp::getContestApp()->loggerBundle.getBoolProfile(elpBandMapDisableNotShown, disableNotShown);

        if (disableNotShown)
        {
            spotQueue.clear();
            logSpotQueue.clear();
            return;
        }
    }

    bandmapView->setSuppressUpdate(true);
    bool doUpdate = false;
    // any cluster spots
    if (!spotQueue.isEmpty())
    {
        int sqsize = spotQueue.count();
        for (int i = sqsize -1 ; i > -1; i--)
        {
            traceMsg("New Cluster Spot: " + spotQueue[i]->getDxCall().getFullCall());
            addDxSpotToBandmapTable(spotQueue[i]);
            doUpdate = true;
        }
        bandmapDataModel->insertRows(bandmapDataModel->rowCount(), bandmapDataModel->rowData.size());
        trace("BandmapView::bandmapUpdate() addLogSpotToBandmapTable completion");
        bandmapView->bandmapUpdate(true);
        spotQueue.clear();
    }

    // any logger spots
    if (!logSpotQueue.isEmpty())
    {
        bool disableLoggedCalls;
        TContestApp::getContestApp()->loggerBundle.getBoolProfile(elpBandMapDisableLoggedCalls, disableLoggedCalls);
        if (!disableLoggedCalls)
        {
            for (int i = 0; i < logSpotQueue.count(); i++)
            {
                traceMsg(QString("New Logger Spot: %1 %2 %3 %4").arg(logSpotQueue[i]->spotName(),  logSpotQueue[i]->getDxCallStr(), logSpotQueue[i]->getFreq().traceStr(), logSpotQueue[i]->getDxLocator()));
                addLogSpotToBandmapTable(logSpotQueue[i]);
                doUpdate = true;
            }
        }
        bandmapDataModel->insertRows(bandmapDataModel->rowCount(), bandmapDataModel->rowData.size());
        logSpotQueue.clear();
    }
    bandmapView->setSuppressUpdate(false);
    if (doUpdate)
    {
        bandmapView->bandmapUpdate(false);
    }

}

void BandmapClientFrame::addDxSpotToBandmapTable(QSharedPointer<BandmapSpotData>  spot)
{
    if (!checkSpotInTable(spot))
    {
        return; // spot logged or marked and moved
    }

    traceMsg(QString("Add Cluster Spot to Bandmap %1, %2, %3, %4, dxLocatorIsFromNode = %5")
             .arg(spot->getDxCall().getFullCall(), spot->getFreq().traceStr(), (spot->getMode(), spot->getDxLocatorWorked(), spot->getDxLocatorIsFromNode() ? "true" : "false"))
                  );

    bandmapDataModel->rowData.push_back(spot);

    bandmapDataModel->insertRows(bandmapDataModel->rowCount(), 1);
}
//======================================================================================
// log spots
void BandmapClientFrame::addLogSpotToBandmapTable(QSharedPointer<BandmapSpotData>  spot)
{
    // is it a CQ Freq Spot
    if (spot->getSpotType() == bandmapSpotType::CQ)
    {
        addRemoveCQSpot(spot);
        trace("BandmapView::bandmapUpdate() addRemoveCQSpot");
        bandmapView->bandmapUpdate(true);
        return;
    }

    // look for an existing spot if the marker is a LOGGED or SAVE type
//    enum SPOT_TYPE {NONE, CLUSTER, CLUSTER_MARKED, LOGGED, MARKED, SAVED, CQ, DELETED};

    bool cqResponse = spot->getCqResponse();    // for logged QSOs
    if (spot->getSpotType() == bandmapSpotType::SAVED)
    {
        // If we are saving, we should ignore being on CQ freq (or not)?
        // Possibly a problem if tuning off CQ freq, with details present
        cqResponse = spot->getRunModeOn() && !spot->getOffRunFreq();
    }

    if (spot->getSpotType() == bandmapSpotType::LOGGED || spot->getSpotType() == bandmapSpotType::SAVED)
    {
        // IF it is a LOGGED spot, then check all spots (of all types), and mark call/loc worked as appropriate
        // NB This spot should already be marked as call and loc worked
        if (spot->getSpotType() == bandmapSpotType::LOGGED)
        {
            Callsign loggedCall = spot->getDxCall();
            QString band = spot->getBand();
            for (int row = 0; row < bandmapDataModel->rowCount(); row++)
            {
                QSharedPointer<BandmapSpotData> bsd = bandmapDataModel->getBandmapDataRow(row);

                const Callsign &savedCs = bsd->getDxCall();
                QString savedBand = bsd->getBand();

                if (ct->isHF() && savedBand != band )
                {
                    continue;
                }

                if (savedCs == loggedCall )
                {
                    bandmapSpotType::SPOT_TYPE savedSpotType = bsd->getSpotType();
                    if (savedSpotType == bandmapSpotType::LOGGED || savedSpotType == bandmapSpotType::SAVED)
                    {
                        // delete the old logged/saved entry, add the new one
                        bsd->setSpotType(bandmapSpotType::DELETED);
                        continue;
                    }
                    bsd->setDxCallWorked(true);
                }

                // update worked locators
                QString loc = spot->getDxLocator();
                if (!loc.isEmpty())
                {
                    QString locMajor = loc.mid(0,4);

                    QString storedLoc = bsd->getDxLocator();
                    if (!storedLoc.isEmpty())
                    {
                        storedLoc = storedLoc.mid(0,4);
                        if (locMajor == storedLoc)
                        {
                            bsd->setDxLocatorWorked(true);
                        }
                    }
                }
            }
        }
        // If it is a SAVED spot we need to test this spot for worked
        if (spot->getSpotType() == bandmapSpotType::SAVED)
        {
            QString loc = spot->getDxLocator();
            Callsign call = spot->getDxCall();
            if (!loc.isEmpty() || call.getValRes() == CS_OK)
            {
                // check to see if call or locator worked
                bool callWorked = false;
                bool locWorked = false;
                checkSpotWorked(call, loc, spot->getFreq(), &callWorked, &locWorked);
                if (locWorked)
                {
                    spot->setDxLocatorWorked(true);
                }
                if (callWorked)
                {
                    spot->setDxCallWorked(true);
                }
            }
        }
        if (!cqResponse)
        {
            // We don't put responses to CQ calls onto the bandmap
            //Check for existing spots for this call; move them rather than make new
            for (int row = 0; row < bandmapDataModel->rowCount(); row++)
            {
                QSharedPointer<BandmapSpotData> bsd = bandmapDataModel->getBandmapDataRow(row);

                const Callsign &savedCall = bsd->getDxCall();
                QString savedBand = bsd->getBand();

                Callsign loggedCall = spot->getDxCall();
                QString band = spot->getBand();

                if (savedCall == loggedCall && savedBand == band )
                {
                    Frequency savedFreq = bsd->getFreq();
                    bandmapSpotType::SPOT_TYPE savedSpotType = bsd->getSpotType();
                    //    enum SPOT_TYPE {NONE, CLUSTER, CLUSTER_MARKED, LOGGED, MARKED, SAVED, CQ, DELETED};
                    if (
                         (savedSpotType == bandmapSpotType::LOGGED
                         || savedSpotType == bandmapSpotType::SAVED
                         || savedSpotType == bandmapSpotType::CLUSTER_MARKED)

                            && spot->getSpotType() == bandmapSpotType::SAVED)
                    {
                        // we want to move the freq of the pre-existing spot if this is a SAVED spot
                        traceMsg(QString("AddLogSpot Callsign moved freq - %1, %2").arg(savedCall.getFullCall(), savedFreq.traceStr()));

                        bsd->setFreq(spot->getFreq());
                        bsd->setDxCallWorked(spot->getDxCallWorked());
                        bsd->setDxLocatorWorked(spot->getDxLocatorWorked());

                        // override the call - it may now be /P (or not /P), etc
                        bsd->setCallsign(spot->getDxCall());

                        QString exchange = spot->getDistrict();
                        if (!exchange.isEmpty())
                        {
                            bsd->setDistrict(exchange);
                        }
                        QString loc = spot->getDxLocator();

                        if (!loc.isEmpty())
                        {
                            // and override the loc - it may now be provided or changed
                            QString distance;

                            if (!spot->getDxLocator().isEmpty())
                            {
                                double dist = 0;
                                int brg = 0;
                                calcSpotDistanceBearing(spot->getDxLocator(), &dist, &brg);
                                distance = QString::number(static_cast<int>(dist));
                            }
                            QString rotBrg;
                            if (rotatorConnected)
                            {
                                rotBrg = curRotBearing;   // get rotator bearing
                            }
                            else
                            {
                                rotBrg = "0";
                            }
                            bsd->setDxLocator(loc);

                            // what was this for? we never add the spot!
//                            bandmapDataModel->rowData->setDxDist(distance);
//                            bandmapDataModel->rowData->setDxBrg(spot->getDxBrg());
//                            bandmapDataModel->rowData->setRotBrg(rotBrg);
//                            bandmapDataModel->rowData->setRotConnected(rotatorConnected);
                        }
                        bandmapDataModel->sortModel();
                        bandmapView->bandmapUpdate(true);

                        // do we need to update the time as well????
                        // we don't need to save this incomming logger spot as we have moved it..
                        return;
                    }
                    else if  (savedSpotType == bandmapSpotType::SAVED
                              || savedSpotType == bandmapSpotType::CLUSTER)
                    {
                        // overwrite saved or cluster spots from logged or saved spots; delete existing and later it will be re-added
                        traceMsg(QString("AddLogSpot Callsign removed - %1").arg(savedCall.getFullCall()));
                        bsd->setSpotType(bandmapSpotType::DELETED);
                    }
                }
            }
        }
    }

    if (!cqResponse)
    {
        // We don't put responses to CQ calls onto the bandmap

        // put the spot on the bandmap
        // find distance to station
        QString distance;

        if (!spot->getDxLocator().isEmpty())
        {
            double dist = 0;
            int brg = 0;
            calcSpotDistanceBearing(spot->getDxLocator(), &dist, &brg);
            distance = QString::number(static_cast<int>(dist));
        }

        QString rotBrg;
        if (rotatorConnected)
        {
            rotBrg = curRotBearing;   // get rotator bearing
        }
        else
        {
            rotBrg = "0";
        }

        qint64 logTime = spot->getSpotDateTime().toMSecsSinceEpoch() / 1000;
        QString logTimeStr = spot->getSpotDateTime().time().toString("HH:mm");

        traceMsg(QString("Add Log Spot to Bandmap %1, %2, %3, %4").arg(spot->getDxCallStr(), spot->getFreq().traceStr(), spot->getMode(), spot->getDxLocator()));


        spot->setRxTime(logTime);
        spot->setSpotTime(logTimeStr);

        spot->setDxDist(distance);
        spot->setDxBrg(spot->getDxBrg());
        spot->setRotBrg(rotBrg);
        spot->setRotConnected(rotatorConnected);

        bandmapDataModel->rowData.push_back(spot);

    }
}

void BandmapClientFrame::addRemoveCQSpot(QSharedPointer<BandmapSpotData>  spot)
{
    if (!spot->getRunModeOn())
    {
        // look for an existing CQ spot and remove
        for (int row = 0; row < bandmapDataModel->rowCount(); row++)
        {
            bandmapSpotType::SPOT_TYPE savedSpotType = static_cast<bandmapSpotType::SPOT_TYPE>(bandmapDataModel->data(bandmapDataModel->index(row, SPOT_TYPE_COL_NUM ),  BMP_DataStoredRole).toInt());
            if (savedSpotType == bandmapSpotType::CQ)
            {
                bandmapDataModel->setData(bandmapDataModel->index(row, SPOT_TYPE_COL_NUM ), bandmapSpotType::DELETED, BMP_DataStoredRole);
            }
        }
    }
    else
    {
        qint64 logTime = spot->getSpotDateTime().toMSecsSinceEpoch() / 1000;
        QString logTimeStr = spot->getSpotDateTime().time().toString("HH:mm");

        // does a CQ Spot exist?
        int rowNum = -1;
        bandmapSpotType::SPOT_TYPE savedSpotType = bandmapSpotType::SPOT_TYPE::NONE;
        for(int row = 0; row < bandmapDataModel->rowCount(); row++)
        {
            savedSpotType = static_cast<bandmapSpotType::SPOT_TYPE>(bandmapDataModel->data(bandmapDataModel->index(row, SPOT_TYPE_COL_NUM ),  BMP_DataStoredRole).toInt());
            if (savedSpotType == bandmapSpotType::CQ)
            {
                rowNum = row;
                break;
            }
        }

        if (rowNum == -1)
        {
            spot->setRxTime(logTime);
            spot->setSpotTime(logTimeStr);
            spot->setRotConnected(rotatorConnected);

            bandmapDataModel->rowData.push_back(spot);

            bandmapDataModel->insertRows(bandmapDataModel->rowCount(), bandmapDataModel->rowData.size());
        }
        else
        {
            // update the spot
            bandmapDataModel->setData(bandmapDataModel->index(rowNum, RUN_MODE_ON_COL_NUM ), spot->getRunModeOn() ,BMP_DataStoredRole);
            bandmapDataModel->setData(bandmapDataModel->index(rowNum, OFF_RUN_FREQ_COL_NUM ), spot->getOffRunFreq() ,BMP_DataStoredRole);
            QVariant f;
            f.setValue(spot->getFreq());
            bandmapDataModel->setData(bandmapDataModel->index(rowNum, FREQ_COL_NUM ), f ,BMP_DataStoredRole);
            bandmapDataModel->setData(bandmapDataModel->index(rowNum, DXSPOT_MODE_COL_NUM ), spot->getMode() ,BMP_DataStoredRole);
            bandmapDataModel->sortModel();
        }
    }
}

bool BandmapClientFrame::checkSpotInTable(QSharedPointer<BandmapSpotData> spot)
{
    Callsign dxCallsign = spot->getDxCall();
    Frequency dxFreq = spot->getFreq();

    if (bandmapDataModel->rowCount() != 0)
    {
        // check for repeat call
        for (int row = 0; row < bandmapDataModel->rowCount(); row++)
        {
            QSharedPointer<BandmapSpotData> bsd = bandmapDataModel->getBandmapDataRow(row);

            Callsign rowCall = bsd->getDxCall();
            if (dxCallsign == rowCall)
            {
                Frequency spotFreq = bsd->getFreq();
                if (ct->isHF())
                {
                    QSharedPointer<BandInfo>  bandChanged = ct->checkBandChange(dxFreq, spotFreq);
                    if (bandChanged)
                    {
                        continue;
                    }

                }
                bandmapSpotType::SPOT_TYPE spotType = bsd->getSpotType();
                if ( spotType == bandmapSpotType::LOGGED || spotType == bandmapSpotType::SAVED || spotType == bandmapSpotType::CLUSTER_MARKED)
                {
                    // move the logged or marked spot to new freq
                    traceMsg(QString("Check Spot in Table - Move Freq %1, %2 to %3")
                             .arg(rowCall.getFullCall(),
                             spotFreq.traceStr(),
                             dxFreq.traceStr())
                             );

                    bsd->setFreq(dxFreq);
                    bandmapDataModel->sortModel();
                    return  false;          // don't save this spot to the bandmap spot list

                }
                else if (spotType == bandmapSpotType::CLUSTER)
                {
                    // yes, remove old spot
                    traceMsg(QString("CheckSpot In Table Remove - Cluster Spot %1").arg(rowCall.getFullCall()));
                    bsd->setSpotType(bandmapSpotType::DELETED);
                    // and this spot will be used instead
                }
            }
        }
    }
    return true;
}

void BandmapClientFrame::checkSpotWorked(const Callsign &mcs, const QString &locator, const Frequency &freq, bool* callWorked, bool* locatorWorked)
{
    bool callfound = false;
    bool locfound = false;
    if (ct && !ct->isReadOnly())
    {
        for ( LogIterator i = ct->ctList.begin(); i != ct->ctList.end(); i++ )
        {
            if ((*i).wt->notValidContact() )
            {
                continue;
            }

            if (ct->isHF())
            {
                QSharedPointer<BandInfo> bandChanged = ct->checkBandChange(freq, (*i).wt->frequency.getValue().str());
                if (bandChanged)
                {
                    continue;
                }
            }
            if (!callfound)
            {
                if ((*i).wt->cs == mcs)
                {
                    *callWorked = true;
                    callfound = true;
                }
            }

            if (!locator.isEmpty())
            {
                QString loc = locator.mid(0,4);
                if ((*i).wt->loc.getLoc().mid(0,4) == loc)
                {
                    *locatorWorked = true;
                    locfound = true;
                }
            }

            if (callfound && locfound)
            {
                return;
            }
        }
    }
}


void BandmapClientFrame::calcSpotDistanceBearing(const QString& _locator, double* distance, int* bearing)
{
    bool locValid = true;
    QString locator = _locator;
    double latitude;
    double longitude;
    double dist;
    int brg = 0;

    if (ct && !locator.isEmpty())
    {
        if (locator.count() == 4)
        {
            locator.append("MM");
        }

        int locValres = lonlat( locator, longitude, latitude, MinosParameters::getMinosParameters() ->getAllowLoc4() );
        if ( ( locValres ) != LOC_OK )
        {
            locValid = false;
        }
        if (locValid)
        {
            ct->disbeara(longitude, latitude, dist, brg);
            *distance = dist;
            *bearing = brg;
        }
    }
}

void BandmapClientFrame::setClusterServerState(QString stateMsg)
{
    QStringList s = stateMsg.split("<>");
    if (s.size() < 2)
        return;

    if (LogContainer->sendDM->isClusterConnected())
    {
         traceMsg("cluster is connected");
         clusterStatusIndicatorToggle(true);
         clusterServerConnected = true;
         ui->clusterStatusIndicator->setEnabled(false);
    }
    else
    {
         traceMsg("cluster is not connected");
         clusterStatusIndicatorToggle(false);
         clusterServerConnected = false;
         ui->clusterStatusIndicator->setEnabled(true);  // enable to allow reconnect request
    }

    if (LogContainer->sendDM->isClusterServerLoaded() )
    {
        ui->clusterStatusIndicator->setToolTip(s[1]);
        traceMsg(QString("Cluster Status: %1").arg(stateMsg));
    }
    else
    {
        ui->clusterStatusIndicator->setToolTip(tr("Cluster Server Not Running"));
    }
}

void BandmapClientFrame::on_clusterStatusIndicatorClicked()
{
    if (!ui->clusterStatusIndicator->toolTip().isEmpty())  // haven't connected yet?
    {
            if (!clusterServerConnected && ui->clusterStatusIndicator->toolTip() != "Connected")
            {
                trace(QString("cluster server disconnected - request reconnect"));
                MinosLoggerEvents::sendReconnectFlagToClusterServer(true);
            }
    }
}

void BandmapClientFrame::clusterStatusIndicatorToggle(bool on)
{
    if (on)
    {
        ui->clusterStatusIndicator->setStyleSheet(STATUS_INDICATOR_CONNECT_STYLE);
    }
    else
    {
       ui->clusterStatusIndicator->setStyleSheet(STATUS_INDICATOR_DISCONNECT_STYLE);
    }
}

void BandmapClientFrame::radioStatusIndicatorToggle(bool on)
{
    if (on)
    {
        ui->radioStatusIndicator->setStyleSheet(STATUS_INDICATOR_CONNECT_STYLE);
        ui->radioStatusIndicator->setToolTip(tr("Connected"));
        ui->radioStatusIndicator->hide();
        ui->radioStatusLabel->hide();
        ui->freqDisplay->show();
        ui->modeLabel->show();
        ui->mode->show();
    }
    else
    {
       ui->radioStatusIndicator->setStyleSheet(STATUS_INDICATOR_DISCONNECT_STYLE);
       ui->radioStatusIndicator->setToolTip(tr("Disconnected"));
       ui->radioStatusIndicator->show();
       ui->radioStatusLabel->show();
       ui->freqDisplay->hide();
       ui->modeLabel->hide();
       ui->mode->hide();
    }
}

void BandmapClientFrame::checkLegalFrequencies(Frequency freq)
{
    bool legalOperatingFreqFlag;
    TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpContestTurnOffOperatingFreqColorRadioDial, legalOperatingFreqFlag );

    QString sf = freq.str();

    if (sf.count() >= 4)
    {
        ui->freqDisplay->setInputMask(maskData::freqMask[sf.count() - 4]);


        if (isFreqLegal(freq, contestBandStr, contestModeStr))
        {

            freqDisplayPalette->setColor(QPalette::Text, Qt::black);
            ui->freqDisplay->setPalette(*freqDisplayPalette);

            legalFreq = true;
        }
        else
        {
            if (!legalOperatingFreqFlag)
            {
                freqDisplayPalette->setColor(QPalette::Text,Qt::red);
                ui->freqDisplay->setPalette(*freqDisplayPalette);
            }
            else
            {
                freqDisplayPalette->setColor(QPalette::Text, Qt::black);
                ui->freqDisplay->setPalette(*freqDisplayPalette);
            }

            legalFreq = false;
        }

        ui->freqDisplay->setText(sf);
    }
    else
    {
        if (!legalOperatingFreqFlag)
        {
            freqDisplayPalette->setColor(QPalette::Text, Qt::red);
            ui->freqDisplay->setPalette(*freqDisplayPalette);
        }

        legalFreq = false;
        ui->freqDisplay->setText(sf);
    }
    bandmapView->setFreq(curFreq, legalFreq);
}

void BandmapClientFrame::setFreq(Frequency freq)
{
    if (!ct || ct->isReadOnly())
        return;

    if (lastfreq != freq)
    {
        QSharedPointer<BandInfo> bandChanged = ct->checkBandChange(freq, lastfreq);
        curFreq = freq;
        if (bandChanged)
        {
            setContestBandMode(bandChanged->uk, contestModeStr);
            checkLegalFrequencies(freq);
        }
        lastfreq = freq;

        // check freq matches contest band
        checkContestBandMatch(curFreq);

        checkLegalFrequencies(freq);
    }
}

void BandmapClientFrame::setContestBandMode(QString band, QString mode)
{
    if (!ct || ct->isReadOnly())
        return;

    contestBandStr = band;
    setMode(mode);

    getBandLimitsFromBandListXML();

    int zoomLevel = readBandmapZoomLevel();     // set zoom to saved zoomlevel
    bandmapView->setBandmapZoom(zoomLevel);
    setZoomLevelLabelText(zoomLevel);


}

bool BandmapClientFrame::checkContestBandMatch(Frequency cFreq)
{

    if (cFreq >= contestBandFlow && cFreq <= contestBandFHigh)
    {

        ui->radioStatusMsg->clear();
        return true;
    }
    else
    {
        ui->radioStatusMsg->setText(HtmlFontColour(Qt::red) + tr("Freq out of band"));
    }

    return false;
}


void BandmapClientFrame::setMode(QString mode)
{
    if (!ct || ct->isReadOnly())
        return;

    contestModeStr = mode;
    contestMode = getModeOffSet(contestModeStr);

    if (!mode.isEmpty())
    {
        if (mode.contains(':') && mode.contains(hamlibData::MGM))
        {
            QStringList ml = mode.split(':');
            if (ml.count() == 2)
            {
                radioMode = ml[0].trimmed();
            }
        }
        else
        {
            radioMode = mode.trimmed().remove(':');
        }

        bandmapView->setDialRadioMode(radioMode);
        ui->mode->setText(radioMode);
    }
}

void BandmapClientFrame::filterButtonSelected()
{
    BandmapClientFilterDialog* filterSetup =  new BandmapClientFilterDialog(ct, filterSettings, this);
    filterSetup->exec();

    if (filterSetup->getSettingsChangedFlag())
    {
        // reload filtersettings after change
        filterSettings = filterSetup->getFilterSettings();
        trace("BandmapView::bandmapUpdate() filterButtonSelected");
        ShowFilter();
        bandmapView->bandmapUpdate(true);

    }
    filterSetup->close();
    delete filterSetup;

}

bool BandmapClientFrame::event(QEvent *event)
{
   if (event->type() == QEvent::Enter)
   {
       bool minBFlag;
       TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpBandMapMouseInFrameDelay, minBFlag );
       if (minBFlag)
           setHoldUpdateFlag(true);
   }
   else if (event->type() == QEvent::Leave)
   {
       bool minBFlag;
       TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpBandMapMouseInFrameDelay, minBFlag );
       if (minBFlag)
       {
           mouseInFrameTimer->stop();
           if (!purgeSpotFlag)
           {
               checkNewBandMapSpots();
           }
           setHoldUpdateFlag(false);
       }
   }
   return QWidget::event(event);
}

bool BandmapClientFrame::eventFilter(QObject *obj, QEvent *event)
{
   if (obj == ui->freqDisplay && event->type() == QEvent::MouseButtonPress)
   {
       emit freqDisplayClicked();
   }

   return false;
}

void BandmapClientFrame::setTextToFrameTitle(QString text1, QString col, QString text2)
{
    QFontMetrics metrix(ui->mouseInFrameLabel->font());

    int width = ui->mouseInFrameLabel->width()  - 2;
    QString clippedText;
    if (width > 0)
        clippedText = col + metrix.elidedText(text2, Qt::ElideRight, width);
    ui->mouseInFrameLabel->setText(text1 + clippedText);

    ui->mouseInFrameLabel->setToolTip(text1 + col + text2);
}
void BandmapClientFrame::setHoldUpdateFlag(bool state)
{
    QString clText /*= tr("Bandmap")*/;
    holdUpdateFlag = state;
    if (state)
    {
        setTextToFrameTitle(clText, HtmlFontColour(Qt::red), tr("Mouse in frame, updates paused"));
        ui->mouseInFrameLabel->setVisible(true);
    }
    else
    {
        ui->mouseInFrameLabel->setVisible(false);
        ui->mouseInFrameLabel->setText(clText);
        ui->mouseInFrameLabel->setToolTip(clText);  // not really needed, as if we hover we must be in frame
    }
}

void BandmapClientFrame::buttonHandleDxSpots()
{
    checkNewBandMapSpots();
}

void BandmapClientFrame::mouseMoveEvent(QMouseEvent *event)
{
    static QPoint mousePos = QPoint(0, 0);
    if (holdUpdateFlag)
    {
       mouseInFrameTimer->start(MOUSE_IN_FRAME_TIMEOUT);
       if (mousePos != event->pos())
       {
           mousePos = event->pos();
           mouseInFrameTimer->start(MOUSE_IN_FRAME_TIMEOUT);
       }
    }
}

void BandmapClientFrame::mouseTimerCheckNewSpots()
{
    if (holdUpdateFlag)
    {
        if (!purgeSpotFlag)
        {
            checkNewBandMapSpots();
        }
        mouseInFrameTimer->start(MOUSE_IN_FRAME_TIMEOUT);
    }
}

void BandmapClientFrame::purgeSpots()
{
    if (!ct || ct->isReadOnly())
        return;

    if (timeToLive > 0 && !holdUpdateFlag)      // don't purge spots if == 0 and holdupdateflag is on
    {
        if (bandmapDataModel->rowCount() > 0)
        {
           purgeSpotFlag = true;
           bandmapSpotType::SPOT_TYPE spotType;

           int idx = bandmapDataModel->rowCount() - 1;
           while (idx >= 0 && bandmapDataModel->rowCount() > 0)
           {
               spotType = static_cast<bandmapSpotType::SPOT_TYPE>(bandmapDataModel->data(bandmapDataModel->index(idx, SPOT_TYPE_COL_NUM), BMP_DataStoredRole).toInt());
               if (spotType == bandmapSpotType::CLUSTER)
               {
                   if (spotTimedOut(bandmapDataModel->data(bandmapDataModel->index(idx, RXTIME_COL_NUM), BMP_DataStoredRole).toLongLong(), timeToLive))
                   {
                       traceMsg(QString("Cluster Spot purged - %1").arg(bandmapDataModel->data(bandmapDataModel->index(idx, DXSPOT_CALL_COL_NUM), BMP_DataStoredRole).toString()));
                       bandmapDataModel->removeRows(idx, 1, QModelIndex());
                   }
               }
               else if (spotType == bandmapSpotType::DELETED)
               {
                   traceMsg(QString("Deleted Spot purged - %1").arg(bandmapDataModel->data(bandmapDataModel->index(idx, DXSPOT_CALL_COL_NUM), BMP_DataStoredRole).toString()));
                   bandmapDataModel->removeRows(idx, 1, QModelIndex());
               }

               idx--;
           }
           purgeSpotFlag = false;
        }
    }
    trace("BandmapView::bandmapUpdate() purgeSpots");
    bandmapView->bandmapUpdate(false);
}

void BandmapClientFrame::on_AfterLogContact(BaseContestLog *c, QSharedPointer<BaseContact> lct)
{
    if (ct == c && !ct->isReadOnly())
    {
        if ( lct->notValidContact() )
            return;

        Frequency freq = lct->frequency.getValue();
        if (freq.isClear())
        {
            return;
        }
        Callsign cs = lct->cs;
        QString loc = lct->loc.getLoc();
        QString brg = QString::number(lct->bearing);
        QDateTime time = QDateTime::currentDateTimeUtc();

        QString logBandStr;
        QString logBandType;

        getBand(bands, freq, logBandStr, logBandType);

        QString logModeStr = lct->mode.getValue();

        traceMsg(QString("afterlog contact add marker - callsign %1, freq %2, loc %3, brg %4, mode %5")
                 .arg(cs.getFullCall(), freq.traceStr(), loc, brg, logModeStr));


        QSharedPointer<BandmapSpotData> spot(new BandmapSpotData(bandmapSpotType::LOGGED));
        spot->setCallsign(cs);
        spot->setDxLocator(loc);
        spot->setDxBrg(brg);
        spot->setMode(logModeStr);
        spot->setFreq(freq);
        spot->setBand(logBandStr);
        spot->setDxCallWorked(true);
        spot->setDxLocatorWorked(true);
        spot->setSpotDateTime(time);
        spot->setRunModeOn(runModeOn);
        spot->setOffRunFreq(offRunFreq);
        spot->setCqResponse(lct->cqResponse.getValue());
        if (!lct->extraText.getValue().isEmpty())
        {
            spot->setDistrict(lct->extraText.getValue());
            spot->setDistrictWorked(true);
        }

        logSpotQueue.append(spot);
    }
}


void BandmapClientFrame::setRunOnFlag(Frequency _runFreq, QString _mode, bool _runModeOn)
{
    runFreq = _runFreq;
    runMode = _mode;
    runModeOn = _runModeOn;
    setCQFreq();
}

void BandmapClientFrame::setRunOffFreqFlag(Frequency _runFreq, bool _offRunFreq)
{
    runFreq = _runFreq;
    offRunFreq = _offRunFreq;
    setCQFreq();
}

void BandmapClientFrame::setCQFreq()
{
    if (ct && !ct->isReadOnly())
    {
        traceMsg(QString("set CQFreq - runFreq %1, runModeOn %2, offRunFreq %3").arg(runFreq.traceStr(), runModeOn ? "True" : "False", offRunFreq ? "True" : "False"));
        QDateTime time = QDateTime::currentDateTimeUtc();


        Frequency freq = runFreq;
        QString logModeStr = runMode;

        QString logBandStr;
        QString logBandType;

        getBand(bands, freq, logBandStr, logBandType);

        QSharedPointer<BandmapSpotData> spot(new BandmapSpotData(bandmapSpotType::CQ));
        spot->setDxCall("???");
        spot->setDxLocator("");
        spot->setDxBrg("");
        spot->setMode(logModeStr);
        spot->setFreq(freq);
        spot->setBand(logBandStr);
        spot->setDxCallWorked(false);
        spot->setSpotDateTime(time);
        spot->setRunModeOn(runModeOn);
        spot->setOffRunFreq(offRunFreq);

        logSpotQueue.append(spot);
    }
}

void BandmapClientFrame::setBandmapMarkFreq(Frequency _freq, QString mode)
{
    if (ct && !ct->isReadOnly())
    {
        traceMsg(QString("mark freq add marker - freq %1, mode %2").arg(_freq.traceStr(), mode));
        QDateTime time = QDateTime::currentDateTimeUtc();

        QString logBandStr;
        QString logBandType;

        getBand(bands, _freq, logBandStr, logBandType);
        QString logModeStr = mode;
        if(mode.isEmpty())
        {
            logModeStr = getMode(modeBandPlan, _freq, logBandStr);
        }

        QSharedPointer<BandmapSpotData> spot( new BandmapSpotData(bandmapSpotType::MARKED));
        spot->setDxCall("???");
        spot->setDxLocator("");
        spot->setDxBrg("");
        spot->setMode(logModeStr);
        spot->setFreq(_freq);
        spot->setBand(logBandStr);
        spot->setDxCallWorked(false);
        spot->setSpotDateTime(time);
        spot->setRunModeOn(false);
        spot->setOffRunFreq(false);
        spot->setDistrict("????");

        logSpotQueue.append(spot);
    }
}

void BandmapClientFrame::setBandmapSaveFreq(QString cs, Frequency _freq, QString mode, QString loc, QString brg, QString exchange)
{
    if (ct && !ct->isReadOnly())
    {
        traceMsg(QString("save freq  add marker - callsign %1, freq %2, loc %3, brg %4").arg(cs, _freq.traceStr(), loc, brg));
        QDateTime time = QDateTime::currentDateTimeUtc();

        QString logBandStr;
        QString logBandType; // not using bandType

        getBand(bands, _freq, logBandStr, logBandType);
        QString logModeStr = mode;
        if(mode.isEmpty())
        {
            logModeStr = getMode(modeBandPlan, _freq, logBandStr);
        }

        QSharedPointer<BandmapSpotData> spot(new BandmapSpotData(bandmapSpotType::SAVED));
        spot->setDxCall(cs);
        spot->setDxLocator(loc);
        spot->setDxBrg(brg);
        spot->setMode(logModeStr);
        spot->setFreq(_freq);
        spot->setBand(logBandStr);
        spot->setDxCallWorked(false);
        spot->setSpotDateTime(time);
        spot->setRunModeOn(false);
        spot->setOffRunFreq(false);
        spot->setDistrict(exchange);

        logSpotQueue.append(spot);
    }
}

void BandmapClientFrame::setBandmapRadioIsConnect(bool state)
{
    radioStatusIndicatorToggle(state);

    ui->radioStatusMsg->clear();
    radioError.clear();
}

void BandmapClientFrame::setBandmapRadioHasError(QString error)
{
    radioError = error;
    ui->radioStatusMsg->setText(HtmlFontColour(Qt::red) + error);
}

void BandmapClientFrame::setRotatorBearing(QString s)
{
    QStringList sl = s.split(':');
    if (sl.size() < 3)
    {
        return;
    }
    curRotBearing = sl[0];
}

void BandmapClientFrame::setRotatorConnected(bool connected)
{
    rotatorConnected = connected;
}

void BandmapClientFrame::updateZoom(bool dir)
{
    bandmapView->updateZoom(dir);
}


void BandmapClientFrame::traceMsg(QString msg)
{
    trace(QString("[bandmapFrame] %1").arg(msg));
}

void BandmapClientFrame::saveBandmapZoomLevel(int &level)
{
    if (contestBandStr.isEmpty())
    {
        trace(QString("BandmapFreqDial - Error, set Bandmap zoom contestband = %1").arg(contestBandStr));
        return;
    }

    BandmapZoomLevelIdAndNames bandmapId;
    TContestApp::getContestApp()->loggerBundle.setIntProfile(bandmapId.getStartZoomLevelId(contestBandStr), level);

}

int BandmapClientFrame::readBandmapZoomLevel()
{
    if (contestBandStr.isEmpty())
    {
        return dialData::MAX_ZOOM_LEVEL;
    }

    BandmapZoomLevelIdAndNames bandmapId;
    int zoomLevel;
    QString bandId = bandmapId.getStartZoomLevelId(contestBandStr);
    int defZoomLevel = bandmapId.getStartZoomLevelDefault(contestBandStr);
    TContestApp::getContestApp() ->loggerBundle.getIntProfile( bandId, zoomLevel, defZoomLevel );
    return zoomLevel;
}

void BandmapClientFrame::on_newZoomlevel(int level)
{
    setZoomLevelLabelText(level);
}

void BandmapClientFrame::setZoomLevelLabelText(int level)
{
    ui->zoomSpinner->setValue(level);
}

void BandmapClientFrame::on_textFilterEdit_textChanged(const QString &filter)
{
    bandmapSpotProxyModel->setFilterString(filter);
    trace("BandmapView::bandmapUpdate() on_textFilterEdit_textChanged");

    if (filter.isEmpty())
    {
        ui->bandmapGraphicsView->setBackgroundBrush(QBrush());
    }
    else
    {
        ui->bandmapGraphicsView->setBackgroundBrush(QBrush(static_cast< QColor> ( 0x00FF80C0 ).lighter(135)));
    }
    bandmapView->bandmapUpdate(true);
}
void BandmapClientFrame::keyPressEvent(QKeyEvent *event)
{
    int Key = event->key();
    if (Key == Qt::Key_Escape)
    {
        ui->textFilterEdit->clear();
    }

    QWidget::keyPressEvent(event);

}

void BandmapClientFrame::on_zoomSpinner_valueChanged(int z)
{
    if (bandmapView->getDialZoomLevel() != z)
    {
        bandmapView->setBandmapZoom(z);
    }
}

