#include <QScrollArea>
#include <QDesktopServices>

#include <QLabel>
#include "AppStartup.h"
#include "MShowMessageDlg.h"
#include "MinosLoggerEvents.h"

#include "ContestApp.h"
#include "MatchThread.h"
#include "BandList.h"
#include "cutils.h"
#include "dmbuttonframe.h"
#include "fileutils.h"
#include "qheaderview.h"
#include "qtableview.h"
#include "tqsoeditdlg.h"
#include "tentryoptionsform.h"

#include "SendRPCDM.h"
#include "RPCPubSub.h"
#include "tlogcontainer.h"
#include "focuswatcher.h"
#include "htmldelegate.h"
#include "enqdlg.h"
#include "MatchTreeFrame.h"
#include "rigmemdialog.h"
#include "LoggerContest.h"

#include "ScreenConfigFile.h"
#include "MatchArchiveFrame.h"
#include "MatchOtherFrame.h"
#include "MatchThisFrame.h"
#include "minossplitter.h"
#include "qsologframe.h"
#include "rigcontrolframe.h"
#include "rotcontrolframe.h"
#include "RotPresets.h"
#include "ChatFrame.h"
#include "clusterclientframe.h"
#include "bandmapclientframe.h"
#include "delayedaction.h"
#include "ContestPageControl.h"
#include "MTrace.h"

#include "tsinglelogframe.h"
#include "ui_tsinglelogframe.h"

void TSingleLogFrame::buildFrame(int slotNo)
{
    createScreenComponents();

    buildScreenLayout(slotNo);

    OtherMatchTreeFW = new FocusWatcher(otherMatchFrame->getTreeView());
    connect(OtherMatchTreeFW, &FocusWatcher::focusChanged, this, &TSingleLogFrame::onOtherMatchTreeFocused);
    ArchiveMatchTreeFW = new FocusWatcher(archiveMatchFrame->getTreeView());
    connect(ArchiveMatchTreeFW, &FocusWatcher::focusChanged, this, &TSingleLogFrame::onArchiveTreeFocused);

    createColumnsMenu(columnsMenu, QSOTable->horizontalHeader(), this,
              [=]{
                    viewColumn();
              });

    restoreQSOTableColumns();

    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::ContestPageChanged, this, &TSingleLogFrame::on_ContestPageChanged);

    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::TimerDistribution, this, &TSingleLogFrame::NextContactDetailsTimerTimer);
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::TimerDistribution, this, &TSingleLogFrame::PublishTimerTimer);
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::TimerDistribution, this, &TSingleLogFrame::HideTimerTimer);
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::MakeEntry, this, &TSingleLogFrame::on_MakeEntry);
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::AfterSelectContact, this, &TSingleLogFrame::on_AfterSelectContact, Qt::QueuedConnection);
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::AfterLogContact, this, &TSingleLogFrame::on_AfterLogContact, Qt::QueuedConnection);
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::setMemory, this, &TSingleLogFrame::on_SetMemory);
    // from cluster frame or bandmap frame
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::DxSpotToMemory, this, &TSingleLogFrame::on_dxSpotToMemory);
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::MatchStarting, this, &TSingleLogFrame::on_MatchStarting);

    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::ColumnsChanged, this, &TSingleLogFrame::onColumnsChanged);
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::NextUnfilled, this, &TSingleLogFrame::on_NextUnfilled);
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::GoToSerial, this, &TSingleLogFrame::on_GoToSerial);

    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::XferPressed, this, &TSingleLogFrame::on_XferPressed);
    connect(thisMatchFrame, &MatchThisFrame::editContact, this, &TSingleLogFrame::EditContact);

    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::XferEnabled, GJVQSOLogFrame, &QSOLogFrame::setXferEnabled);
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::MatchTreeSelected, this, &TSingleLogFrame::MatchTreeSelected);

    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::doColumnChanges, this, &TSingleLogFrame::on_doColumnChanges);

    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::ShowCribBand, this, &TSingleLogFrame::onShowCribBand);
    // RigControl Updates
    // From rig controller
    connect(LogContainer->sendDM, &TSendDM::setRadioList, this, &TSingleLogFrame::on_SetRadioList);

    // To rig controller

    connect(FKHRigControlFrame, &RigControlFrame::radioDisconnected, this, &TSingleLogFrame::invalidateCacheOnDisconnect);
    connect(FKHRigControlFrame, &RigControlFrame::selectRadio, this, &TSingleLogFrame::sendSelectRadio);

    connect(FKHRigControlFrame, &RigControlFrame::sendFreqControl, this, &TSingleLogFrame::sendRadioFreq);
    connect(FKHRigControlFrame, &RigControlFrame::sendBandToRigControl, this, &TSingleLogFrame::sendBandToRig);
    connect(FKHRigControlFrame, &RigControlFrame::sendRitFreq, this, &TSingleLogFrame::sendRadioRitFreq);
    connect(FKHRigControlFrame, &RigControlFrame::sendVolumeToRadio, this, &TSingleLogFrame::sendRadioVolume);
    connect(FKHRigControlFrame, &RigControlFrame::ritStatus, this, &TSingleLogFrame::sendRadioRitStatus);
    connect(FKHRigControlFrame, &RigControlFrame::sendModeToControl, this, &TSingleLogFrame::sendRadioMode);
    connect(GJVQSOLogFrame, &QSOLogFrame::sendModeControl, this , &TSingleLogFrame::sendRadioMode);

    connect(runButtonsFrame, &RunButtonsFrame::sendRunOnFlag, this, &TSingleLogFrame::sendRunOnFlag);
    connect(runButtonsFrame, &RunButtonsFrame::sendRunOffFreqFlag, this, &TSingleLogFrame::sendRunOffFreqFlag);


    // Rotator updates
    // From rotator controller
    connect(LogContainer->sendDM, &TSendDM::RotatorList, this, &TSingleLogFrame::on_RotatorList);

    // To rotator controller
    connect(FKHRotControlFrame, &RotControlFrame::sendRotator, this, &TSingleLogFrame::sendRotator);
    connect(rotPresets, &RotPresets::sendRotatorPreset, this, &TSingleLogFrame::sendRotatorPreset);
    connect(FKHRotControlFrame, &RotControlFrame::selectRotator, this, &TSingleLogFrame::sendSelectRotator);
    connect(FKHRotControlFrame, &RotControlFrame::selectRotator, rotPresets, &RotPresets::selectRotator);
    connect(rotPresets, &RotPresets::presetTurn, this, &TSingleLogFrame::presetTurn);
    connect(FKHRotControlFrame, &RotControlFrame::rotatorConnected, this, &TSingleLogFrame::on_rotatorConnected);

    // from cluster frame
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::DxSpotToLog, this, &TSingleLogFrame::dxSpotToLog);
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::ResendSpotsFromClusterCommand, this, &TSingleLogFrame::on_ResendSpotsFromClusterCommand);
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::SendReconnectFlagToServer, this, &TSingleLogFrame::on_sendReconnectFlagToClusterServer);

    // to cluster server
    connect(GJVQSOLogFrame, &QSOLogFrame::sendSpotToClusterServer, this, &TSingleLogFrame::on_SendSpotToClusterServer);

    // from cluster server
    connect(LogContainer->sendDM, &TSendDM::setClusterState, this, &TSingleLogFrame::on_clusterServerState);
    connect(LogContainer->sendDM, &TSendDM::setClusterTXSpotEnableState, this, &TSingleLogFrame::on_setClusterTXSpotEnableState);


    // to bandmap
    connect(GJVQSOLogFrame, &QSOLogFrame::bandmapMarkFreq, this, &TSingleLogFrame::on_BandmapMarkFreq);
    connect(GJVQSOLogFrame, &QSOLogFrame::bandmapSaveFreq, this, &TSingleLogFrame::on_BandmapSaveFreq);

    // to Qrz Display Panel
    connect(GJVQSOLogFrame, &QSOLogFrame::qrzCallsignRequest, this, &TSingleLogFrame::onQrzCallsignRequest);

    // from Qrz Display Panel
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::QRZInfoToLog, this, &TSingleLogFrame::onQrzInfoToLog );

    // from tx Voice Memory Panel
    connect(txVmButtonsFrame, &TxVmButtonsFrame::sendRadioMode, this, &TSingleLogFrame::sendRadioMode);

    connect(FKHRigControlFrame, &RigControlFrame::radioIsConnected, this, &TSingleLogFrame::sendBandmapRadioIsConnected);
    connect(FKHRigControlFrame, &RigControlFrame::radioHasError, this, &TSingleLogFrame::sendBandmapRadioHasError);


    connect( QSOTable->horizontalHeader(), &QHeaderView::sectionResized, this, &TSingleLogFrame::on_sectionResized);
    connect(QSOTable, &QTableView::doubleClicked, this, &TSingleLogFrame::onQSOTable_doubleClicked);

    connect(LogContainer, &TLogContainer::sendKeyerPlay, this, &TSingleLogFrame::sendKeyerPlay);
    connect(LogContainer, &TLogContainer::sendKeyerRecord, this, &TSingleLogFrame::sendKeyerRecord);
    connect(LogContainer, &TLogContainer::sendKeyerTone, this, &TSingleLogFrame::sendKeyerTone);
    connect(LogContainer, &TLogContainer::sendKeyerTwoTone, this, &TSingleLogFrame::sendKeyerTwoTone);
    connect(LogContainer, &TLogContainer::sendKeyerStop, this, &TSingleLogFrame::sendKeyerStop);
    connect(LogContainer, &TLogContainer::logRadioSettingsChanged, this, &TSingleLogFrame::onLogRadioSettingsChanged);

    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::FontChanged, this, &TSingleLogFrame::on_FontChanged, Qt::QueuedConnection);
}

TSingleLogFrame::TSingleLogFrame(QWidget *parent, BaseContestLog * contest) :
    ContestPage(parent, contest),
    ui(new Ui::TSingleLogFrame),
    bandMapLoaded(false),
    lastStanzaCount( 0 )


{
    qRegisterMetaType< QSharedPointer<BaseContact> > ( "QSharedPointer<BaseContact>" );

    ui->setupUi(this);
}

void TSingleLogFrame::on_FontChanged()
{
    applyScreenLayout();
}
TSingleLogFrame::~TSingleLogFrame()
{
    delete ui;

    ui = nullptr;
    contest = nullptr;

    // we need to delete all the dependant ContestPage as well


    delete clusterControlFrame;
    delete wsjtxFrame;
}
void TSingleLogFrame::createScreenComponents()
{
    // create component frames, parentless
    LoggerContestLog *ct = dynamic_cast<LoggerContestLog *>( getContest() );
    traceMsg("createScreenComponents for " + ct->name.getValue() + " uuid " + ct->uuid);

    QSOTable = new QTableView(this);
    QSOTable->setObjectName(QStringLiteral("QSOTable"));
    QSOTable->setFocusPolicy(Qt::ClickFocus);
    QSOTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    QSOTable->setAlternatingRowColors(true);
    QSOTable->setSelectionMode(QAbstractItemView::SingleSelection);
    QSOTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    QSOTable->setWordWrap(false);
    QSOTable->setCornerButtonEnabled(false);

    QSOTable->verticalHeader()->setVisible(false);
    QSOTable->verticalHeader()->setMinimumSectionSize(1);
    QSOTable->verticalHeader()->setDefaultSectionSize(1);

    QSOTable->horizontalHeader()->setHighlightSections(false);
    QSOTable->horizontalHeader()->setStretchLastSection(true);
    QSOTable->horizontalHeader()->setMinimumSectionSize(10);
    QSOTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    QSOTable->horizontalHeader() ->setSectionsMovable( true );
    QSOTable->horizontalHeader()->setContextMenuPolicy( Qt::CustomContextMenu );

    connect( QSOTable->horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &TSingleLogFrame::onQSOGrid_customContextMenuRequested );
    connect( QSOTable->horizontalHeader(), &QHeaderView::sectionMoved, this, &TSingleLogFrame::onQSOGrid_sectionMoved);

    int lcf;
    TContestApp::getContestApp() ->getIntDisplayProfile(edpListCompression, lcf);
    delegate = QSharedPointer<HtmlDelegate>(new HtmlDelegate(1.0, lcf/100.0));
    qsoModel.delegate = delegate;
    qsoModel.initialise(contest);
    QSOTable->setModel(&qsoModel);

    QSOTable->setItemDelegate( delegate.data() );
    QSize ms = delegate->docSize("XX");
    QSOTable->verticalHeader()->setDefaultSectionSize(ms.height() );
    QSOTable->verticalHeader()->setMinimumSectionSize(10);

    QSOTable->setVisible(false);

    GJVQSOLogFrame = new QSOLogFrame(this);
    GJVQSOLogFrame->setObjectName(QStringLiteral("GJVQSOLogFrame"));

    GJVQSOLogFrame->setFrameShape(QFrame::NoFrame);
    GJVQSOLogFrame->setFrameShadow(QFrame::Plain);
    GJVQSOLogFrame->setLineWidth(2);
    GJVQSOLogFrame->setMidLineWidth(2);

    GJVQSOLogFrame->setVisible(false);
    GJVQSOLogFrame->setAsEdit(false, "Logger");
    GJVQSOLogFrame->initialise( );

    txVmButtonsFrame = new TxVmButtonsFrame(this);
    txVmButtonsFrame->setObjectName(QStringLiteral("txVmButtonsFrame"));
    txVmButtonsFrame->setVisible(false);
    txVmButtonsFrame->setContest(contest);

    bandSwitchFrame = new BandSwitchFrame(this);
    bandSwitchFrame->setObjectName(QStringLiteral("bandSwitchFrame"));
    bandSwitchFrame->setVisible(false);
    bandSwitchFrame->setContest(contest);

    FKHRigControlFrame = new RigControlFrame(this);
    FKHRigControlFrame->setObjectName(QStringLiteral("FKHRigControlFrame"));
    FKHRigControlFrame->setFrameShape(QFrame::StyledPanel);
    FKHRigControlFrame->setFrameShadow(QFrame::Raised);
    FKHRigControlFrame->setVmButtonsFrame(txVmButtonsFrame);

    FKHRigControlFrame->setVisible(false);
    FKHRigControlFrame->setContest(contest);

    bandSwitchFrame->setRigControl(FKHRigControlFrame);

    runButtonsFrame = new RunButtonsFrame(this);
    runButtonsFrame->setObjectName(QStringLiteral("runButtonsFrame"));
    runButtonsFrame->setVisible(false);
    runButtonsFrame->setRigControl(FKHRigControlFrame);
    runButtonsFrame->setContest(contest);


    qrzDisplayFrame = new QrzDisplayFrame(this);
    qrzDisplayFrame->setObjectName(QStringLiteral("qrzDisplayFrame"));
    qrzDisplayFrame->setVisible(false);
    qrzDisplayFrame->setContest(contest);
    setQrzDisplayFrameLoaded(false);

    FKHRotControlFrame = new RotControlFrame(this);

    FKHRotControlFrame->setObjectName(QStringLiteral("FKHRotControlFrame"));
    FKHRotControlFrame->setFrameShape(QFrame::StyledPanel);
    FKHRotControlFrame->setFrameShadow(QFrame::Raised);

    FKHRotControlFrame->setVisible(false);
    FKHRotControlFrame->setContest(contest);

    clusterControlFrame = new ClusterClientFrame(this);
    clusterControlFrame->setObjectName(QStringLiteral("ClusterControlFrame"));
    clusterControlFrame->setFrameShape(QFrame::StyledPanel);
    clusterControlFrame->setFrameShadow(QFrame::Raised);

    clusterControlFrame->setVisible(false);
    clusterControlFrame->setContest(contest);

    bandmapControlFrame = new BandmapClientFrame(this);
    bandmapControlFrame->setObjectName(QStringLiteral("BandmapControlFrame"));
    bandmapControlFrame->setFrameShape(QFrame::StyledPanel);
    bandmapControlFrame->setFrameShadow(QFrame::Raised);

    bandmapControlFrame->setVisible(false);
    bandmapControlFrame->setContest(contest);
    setBandmapLoaded(false);


    rotPresets = new RotPresets(this);

    rotPresets->setObjectName(QStringLiteral("rotPresets"));
    rotPresets->setVisible(false);

    CribSheet = new QFrame(this);

    CribSheet->setObjectName(QStringLiteral("CribSheet"));

    CribSheet->setFrameShape(QFrame::NoFrame);
    CribSheet->setFrameShadow(QFrame::Plain);
    CribSheet->setLineWidth(0);
    CribSheet->setMidLineWidth(0);

    cribSplitter = new MinosSplitter(CribSheet);
    cribSplitter->setObjectName(QStringLiteral("cribSplitter"));
    cribSplitter->setOrientation(Qt::Vertical);

    QVBoxLayout *verticalLayout_5 = new QVBoxLayout(CribSheet);
    verticalLayout_5->setSpacing(0);
    verticalLayout_5->setObjectName(QStringLiteral("verticalLayout_5"));
    verticalLayout_5->setContentsMargins(0, 0, 0, 0);

    CurrentBandLabel = new QLabel(CribSheet);
    CurrentBandLabel->setObjectName(QStringLiteral("CurrentBandLabel"));
    CurrentBandLabel->setTextFormat(Qt::RichText);
    CurrentBandLabel->setWordWrap(true);

    cribSplitter->addWidget(CurrentBandLabel);

    NextContactDetailsLabel = new QLabel(CribSheet);
    NextContactDetailsLabel->setObjectName(QStringLiteral("NextContactDetailsLabel"));
    NextContactDetailsLabel->setTextFormat(Qt::RichText);
    NextContactDetailsLabel->setWordWrap(true);
    NextContactDetailsLabel->setAlignment(Qt::AlignTop);

    cribSplitter->addWidget(NextContactDetailsLabel);

    verticalLayout_5->addWidget(cribSplitter);
    cribSplitter->setSizes({1, 100});

    CribSheet->setVisible(false);

    thisMatchFrame = new MatchThisFrame(this);

    thisMatchFrame->setObjectName(QStringLiteral("thisMatchFrame"));
    thisMatchFrame->setFrameShape(QFrame::StyledPanel);
    thisMatchFrame->setFrameShadow(QFrame::Raised);

    thisMatchFrame->setVisible(false);
    thisMatchFrame->initialise();
    thisMatchFrame->setBaseName("Logger");

    otherMatchFrame = new MatchOtherFrame(this);

    otherMatchFrame->setObjectName(QStringLiteral("otherMatchFrame"));
    otherMatchFrame->setFrameShape(QFrame::StyledPanel);
    otherMatchFrame->setFrameShadow(QFrame::Raised);

    otherMatchFrame->setVisible(false);
    otherMatchFrame->initialise();
    otherMatchFrame->setBaseName("Logger");

    archiveMatchFrame = new MatchArchiveFrame(this);

    archiveMatchFrame->setObjectName(QStringLiteral("archiveMatchFrame"));
    archiveMatchFrame->setFrameShape(QFrame::StyledPanel);
    archiveMatchFrame->setFrameShadow(QFrame::Raised);

    archiveMatchFrame->setVisible(false);
    archiveMatchFrame->initialise();
    archiveMatchFrame->setBaseName("Logger");

    chatFrame = new ChatFrame(this);
    chatFrame->setObjectName(QStringLiteral("chatFrame"));
    chatFrame->setFrameShape(QFrame::StyledPanel);
    chatFrame->setFrameShadow(QFrame::Raised);

    chatFrame->setVisible(false);

    wsjtxFrame = new WsjtxFrame(this);
    wsjtxFrame->setObjectName(QStringLiteral("wsjtxFrame"));
    wsjtxFrame->setFrameShape(QFrame::StyledPanel);
    wsjtxFrame->setFrameShadow(QFrame::Raised);

    wsjtxFrame->setVisible(false);

    qsoMapFrame = new QSOMapFrame(this);
    qsoMapFrame->setObjectName(QStringLiteral("qsoMapFrame"));
    qsoMapFrame->setFrameShape(QFrame::StyledPanel);
    qsoMapFrame->setFrameShadow(QFrame::Raised);

    qsoMapFrame->setVisible(false);

    dmButtonFrame = new DMButtonFrame(this);
    dmButtonFrame->setObjectName(QStringLiteral("DMButtonFrame"));
    dmButtonFrame->setFrameShape(QFrame::StyledPanel);
    dmButtonFrame->setFrameShadow(QFrame::Raised);
    dmButtonFrame->setContest(contest);

    dmButtonFrame->setVisible(false);
}
void TSingleLogFrame::clearScreenLayout(bool clearAllTabs)
{
    // clear down the screen elements, but don't delete them (except for the aux frames) - they will be used to rebuild the screen
    // BUT on contest creation, the contest address may change, so clear the contest

    inClearScreenLayout = true; // stop cutils saving headers
    LoggerContestLog *ct = dynamic_cast<LoggerContestLog *>( getContest() );
    QString msg;
    if (ct != nullptr)
    {
        msg = ct->name.getValue() + " uuid " + ct->uuid;
        traceMsg("clearScreenLayout starts for " + msg);
    }
    else
    {
        msg = "null contest";
        traceMsg("clearScreenLayout starts for " + msg);
    }

    qsoModel.initialise(nullptr);

    FKHRigControlFrame->setContest(nullptr);
    runButtonsFrame->setContest(nullptr);
    bandSwitchFrame->setContest(nullptr);
    txVmButtonsFrame->setContest(nullptr);
    FKHRotControlFrame->setContest(nullptr);
    rotPresets->setContest(nullptr);
    // CribSheet
    // NextContactDetailsLabel
    // CurrentBandLabel
    GJVQSOLogFrame->setContest(nullptr);
    thisMatchFrame->setContest(nullptr);
    otherMatchFrame->setContest(nullptr);
    archiveMatchFrame->setContest(nullptr);
    //chatFrame
    wsjtxFrame->setContest(nullptr);
    clusterControlFrame->setContest(nullptr);
    bandmapControlFrame->setContest(nullptr);
    qsoMapFrame->setContest(nullptr, false, false, false, false, 0);
    dmButtonFrame->setContest(nullptr);

    setBandmapLoaded(false);
    setQrzDisplayFrameLoaded(false);

    // we need to setContest(nullptr) on all aux frames
    MinosLoggerEvents::SendClearContestInFrame(ct);

    clusterControlFrame->setParent(this);
    clusterControlFrame->hide();

    bandmapControlFrame->setParent(this);
    bandmapControlFrame->hide();

    if (LogContainer->isLoggerClosing())
    {
        // do nothing more...
    }
    else
    {
        QSOTable->setParent(this);
        QSOTable->hide();

        FKHRigControlFrame->setParent(this);
        FKHRigControlFrame->hide();

        runButtonsFrame->setParent(this);
        runButtonsFrame->hide();

        bandSwitchFrame->setParent(this);
        bandSwitchFrame->hide();

        txVmButtonsFrame->setParent(this);
        txVmButtonsFrame->hide();

        FKHRotControlFrame->setParent(this);
        FKHRotControlFrame->hide();

        rotPresets->setParent(this);
        rotPresets->hide();

        CribSheet->setParent(this);
        CribSheet->hide();

        GJVQSOLogFrame->setParent(this);
        GJVQSOLogFrame->hide();

        thisMatchFrame->setParent(this);
        thisMatchFrame->hide();

        otherMatchFrame->setParent(this);
        otherMatchFrame->hide();

        archiveMatchFrame->setParent(this);
        archiveMatchFrame->hide();

        chatFrame->setParent(this);
        chatFrame->hide();

        wsjtxFrame->setParent(this);
        wsjtxFrame->hide();

        txVmButtonsFrame->setParent(this);
        txVmButtonsFrame->hide();

        qrzDisplayFrame->setParent(this);
        qrzDisplayFrame->hide();

        qsoMapFrame->setParent(this);
        qsoMapFrame->hide();

        dmButtonFrame->setParent(this);
        dmButtonFrame->hide();

        if (clearAllTabs)
        {
            for (auto cpc = LogContainer->contestPageControls.begin(); cpc != LogContainer->contestPageControls.end(); cpc++)
            {
                if ((*cpc) == nullptr)
                {
                    continue;
                }
                for  (auto cp = (*cpc)->pages.begin(); cp != (*cpc)->pages.end(); cp++)
                {
                    if (cp.key() == contest)
                    {
                        if (cp.value())
                        {
                            cp.value()->clearScreen();
                            if (cp.value() != this)
                            {
                                cp.value()->deleteLater();
                            }
                        }
                        (*cpc)->pages.remove(cp.key());
                        break;
                    }
                }
            }
        }
    }
    inClearScreenLayout = false;
    traceMsg("clearScreenLayout complete for " + msg);
}
void TSingleLogFrame::applyScreenLayout()
{
    LoggerContestLog *ct = dynamic_cast<LoggerContestLog *>( getContest() );
    if (!ct)
        return;

    int slotNo = LogContainer->getSlotNo(this);

    traceMsg("applyScreenLayout for " + ct->name.getValue() + " uuid " + ct->uuid);
    QSOTable->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);

    clearScreenLayout(true);
    buildScreenLayout(slotNo);

    QSOTable->setItemDelegate( delegate.data() );
    QSize ms = delegate->docSize("XX");
    QSOTable->verticalHeader()->setDefaultSectionSize(ms.height());
    QSOTable->verticalHeader()->setMinimumSectionSize(10);

    updateTrees();  //in apply screen layout
}

QString TSingleLogFrame::getCurScreenLayout() const
{
    return curScreenLayout;
}

void TSingleLogFrame::setCurScreenLayout(const QString &value)
{
    trace(QString("setCurScreenLayout %1").arg(value));
    curScreenLayout = value;
    LoggerContestLog *ct = dynamic_cast<LoggerContestLog *>( contest );
    ct->screenLayout.setValue(value);
    ct->commonSave(false);
    if (thisMatchFrame)
    {
        thisMatchFrame->setCurScreenLayout(curScreenLayout);
    }
    if (otherMatchFrame)
    {
        otherMatchFrame->setCurScreenLayout(curScreenLayout);
    }
    if (archiveMatchFrame)
    {
        archiveMatchFrame->setCurScreenLayout(curScreenLayout);
    }
}
void TSingleLogFrame::buildRow(ContestPage *cp, SCRow &scrow, int &auxInstance, MinosSplitter *splitterParent)
{
    // This builds the dependant ContestPage (including the one we derive from)
    if (scrow.elements.count())
    {
        LoggerContestLog *ct = dynamic_cast<LoggerContestLog *>( contest );

        // insert horizontal splitter in splitterParent
        MinosSplitter *hs = new MinosSplitter();
        hs->setObjectName("row" + QString::number(cp->rowSplitters.size()) + "splitter");
        hs->setOrientation(Qt::Horizontal);
        hs->setChildrenCollapsible(false);

        // we want this in the contest page...
        cp->rowSplitters.push_back(hs);

        for (auto &scele: scrow.elements)
        {
            //SCElement scele = scrow.elements[srele];
            SCType type = scele.type;
            if (type == sctNone)
                continue;

            QScrollArea *elementScrollArea = nullptr;
            if (type != sctLog
                    && type != sctThisMatch
                    && type != sctOtherMatch
                    && type != sctArchiveMatch
                    && type != sctSplit
                    && type != sctCluster
                    && type != sctWsjtx
                    && type != sctBandmap
                    )
            {
                elementScrollArea = new QScrollArea();
                elementScrollArea->setWidgetResizable(true);
                elementScrollArea->setFocusPolicy(Qt::NoFocus);
                elementScrollArea->setFrameStyle(QStyleOptionFrame::None);
                elementScrollArea->setFrameShadow(QFrame::Plain);

                hs->addWidget(elementScrollArea);
            }

            // insert correct widget type in horizontal splitter

            switch (type)
            {
                case sctNone:
                case sctMainScreen:
                case sctScreen:
                {
                    break;
                }
                case sctAux:
                {
                    StackedInfoFrame *f = new StackedInfoFrame(elementScrollArea, auxInstance++, this);

                    f->setCurrentFrameType(StackedInfoFrame::getTrAuxTypeString(scele.auxType));
                    f->setContest(ct);
                    elementScrollArea->setWidget(f);
                    f->setVisible(true);
                    break;
                }
                case sctLog:
                {
                    QSOTable->setParent(hs);
                    hs->addWidget(QSOTable);
                    QSOTable->setVisible(true);
                    break;
                }
                case sctRigControl:
                {
                    elementScrollArea->setWidget(FKHRigControlFrame);
                    // don't set contest here
                    break;
                }
                case sctRunButtons:
                {
                    elementScrollArea->setWidget(runButtonsFrame);
                    runButtonsFrame->setContest(ct);
                    break;
                }
                case sctBandSwitch:
                {
                    elementScrollArea->setWidget(bandSwitchFrame);
                    bandSwitchFrame->setContest(ct);
                    break;
                }
                case sctTxVmButtons:
                {
                    elementScrollArea->setWidget(txVmButtonsFrame);
                    txVmButtonsFrame->setContest(ct);
                    break;
                }
                case sctRotControl:
                {
                    elementScrollArea->setWidget(FKHRotControlFrame);
                    // don't set contest here
                    break;
                }
                case sctQrzDisplay:
                {
                    elementScrollArea->setWidget(qrzDisplayFrame);
                    setQrzDisplayFrameLoaded(true);
                    break;
                }
                case sctRotPresets:
                {
                    elementScrollArea->setWidget(rotPresets);
                    rotPresets->setContest(ct);
                    break;
                }
                case sctQSOEdit:
                {
                    elementScrollArea->setWidget(GJVQSOLogFrame);
                    GJVQSOLogFrame->setContest(ct);
                    GJVQSOLogFrame->setVisible(true);
                    break;
                }
                case sctNextQSODetails:
                {
                    elementScrollArea->setWidget(CribSheet);
                    CribSheet->setVisible(true);
                    onShowCribBand();
                    break;
                }
                case sctThisMatch:
                {
                    hs->addWidget(thisMatchFrame);
                    thisMatchFrame->setVisible(true);
                    thisMatchFrame->setContest(ct);
                    break;
                }
                case sctOtherMatch:
                {
                    hs->addWidget(otherMatchFrame);
                    otherMatchFrame->setVisible(true);
                    otherMatchFrame->setContest(ct);
                    break;
                }
                case sctArchiveMatch:
                {
                    hs->addWidget(archiveMatchFrame);
                    archiveMatchFrame->setVisible(true);
                    archiveMatchFrame->setContest(ct);
                    break;
                }
                case sctChat:
                {
                    elementScrollArea->setWidget(chatFrame);
                    chatFrame->setVisible(true);
                    break;
                }
                case sctCluster:
                {
                    hs->addWidget(clusterControlFrame);
                    clusterControlFrame->setVisible(true);
                    clusterControlFrame->setContest(ct);
                    break;

                }
                case sctBandmap:
                {
                    hs->addWidget(bandmapControlFrame);
                    bandmapControlFrame->setVisible(true);
                    bandmapControlFrame->setContest(ct);
                    setBandmapLoaded(true);
                    break;

                }
                case sctWsjtx:
                {
                    hs->addWidget(wsjtxFrame);
                    wsjtxFrame->setVisible(true);

                    // don't set contest here
                    break;
                }
                case sctQsoMap:
                {
                    elementScrollArea->setWidget(qsoMapFrame);
                    qsoMapFrame->setVisible(true);

                    bool grid = false;
                    if (ct->locatorMandatoryField.getValue())
                    {
                        TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpShowQSOMapGrid, grid );
                    }
                    bool lines;
                    TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpShowQSOMapLines, lines );

                    bool spots;
                    TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpMapShowCluster, spots );

                    int sd;
                    TContestApp::getContestApp() ->loggerBundle.getIntProfile( elpMapClusterDistance, sd );

                    qsoMapFrame->setContest(ct, false, grid, lines, spots, sd);
                    break;
                }
                case sctDMButtons:
                {
                    elementScrollArea->setWidget(dmButtonFrame);
                    dmButtonFrame->setVisible(true);
                    dmButtonFrame->setContest(ct);
                    break;
                }
                case sctSplit:
                {
                    MinosSplitter *vs = new MinosSplitter();
                    vs->setObjectName("splitRow" + QString::number(cp->rowSplitters.size()) + "splitter");
                    vs->setOrientation(Qt::Vertical);
                    vs->setChildrenCollapsible(false);
                    cp->rowSplitters.push_back(vs);

                    for (auto &srow: scele.rows)
                    {
                        buildRow(cp, srow, auxInstance, vs);
                    }

                    hs->addWidget(vs);
                    break;
                }
            }
        }
        splitterParent->addWidget(hs);
    }

}
void TSingleLogFrame::buildScreen(SCScreen &s, int t, int &auxInstance)
{
    // we need to add this contest page to the relevant contestPageControl
    // as a new tab

    // How do we make sure that ALL contests are in ALL page controls, even when
    // they have no such screen?

    ContestPage *cp = this;
    if (t > 0)
    {
        for (int i = 0; i <= t; i++)
        {
            if (LogContainer->contestPageControls.count() <= i
               || LogContainer->contestPageControls[i] == nullptr)
            {
                ContestPageControl *cpc = new ContestPageControl();

                cpc->setInstance(i);
                if (i < LogContainer->contestPageControls.count())
                {
                    LogContainer->contestPageControls[i] = cpc;
                }
                else
                {
                    LogContainer->contestPageControls.append(cpc);
                }
                cpc->setWindowFlags(/*Qt::Tool |*/ Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowMinMaxButtonsHint);
                if (i != 0)
                {
                    cpc->setAttribute(Qt::WA_ShowWithoutActivating);
                }
                cpc->show();
            }
        }
        cp = new ContestPage(nullptr, contest);
        QString n = QString("contestpage%1").arg(t);
        cp->setObjectName(n);
        //cp->setStyleSheet(QString(" #%1 { border: 2px solid blue; }").arg(n));
    }
    ContestPageControl *cpc = LogContainer->contestPageControls[t];
    cpc->pages[contest] = cp;
    cp->pageNo = t;
    cp->buildScreen(this, s, auxInstance);

}
void TSingleLogFrame::buildScreenLayout(int slotNo)
{
    ScreenConfigFile &scf = ScreenConfigFile::getScreenConfigFile(this);

    LoggerContestLog *ct = dynamic_cast<LoggerContestLog *>( contest );

    QString curConfigName;
    if (ct->isReadOnly())
    {
        MinosParameters::getMinosParameters() -> getStringDisplayProfile( edpProtectedLayout, curConfigName );
    }
    else
    {
        curConfigName = ct->screenLayout.getValue();
    }
    traceMsg("buildScreenLayout for " + ct->name.getValue() + " uuid " + ct->uuid + " to layout " + curConfigName);
    if (curConfigName.isEmpty() || !scf.configs.contains(curConfigName))
    {
        curConfigName = defaultLayoutName();
        ct->screenLayout.setValue(curConfigName);
    }
    setCurScreenLayout(curConfigName);

    SC sc = scf.configs[curConfigName];

    // build the pages
    int auxInstance = 0;
    int t = 0;
    for (auto &s: sc.baseElement->screens)
    {
        buildScreen(s, t++, auxInstance);
    }

    // and now insert/show them
    bool temp = TContestApp::getContestApp() ->suppressWritePreload;
    TContestApp::getContestApp() ->suppressWritePreload = true;
    for (int t = 0; t < LogContainer->contestPageControls.count(); t++)
    {
        ContestPageControl *cp = LogContainer->contestPageControls[t];
        if (!cp)
            continue;
        ContestPage *p = cp->pages[contest];
        if (!p)
            continue;

        QString sname  = sc.baseElement->screens[t].name;
        if (t == 0 || sname.isEmpty())
        {
            sname = ExtractFileName( contest->cfileName );
        }

        LogContainer->contestPageControls[t]->insertTab(slotNo, p, sname);
    }
    TContestApp::getContestApp() ->suppressWritePreload = temp;
    qsoModel.initialise(contest);
    QSOTable->setModel(&qsoModel);

    // ALWAYS link the wsjt frame to the contest; then we can log
    // even without showing it
    wsjtxFrame->setContest(ct);
    FKHRigControlFrame->setContest(ct);
    FKHRotControlFrame->setContest(ct);

    LogContainer->raise();  // get it back in front

    MinosLoggerEvents::SendMainRaised();
}


bool TSingleLogFrame::doKeyPressEvent( QKeyEvent* event )
{
    // each dependant ContestPage also needs this
    return GJVQSOLogFrame->doKeyPressEvent(event);
}
void TSingleLogFrame::doSendEntry(QString /*expName*/)
{
    // expName is the exported full filename, not yet usable

    // here we have to set up and execute the magic
    // to trigger Petes web site

    // first, check for VHF or HF
    // For HF the link is e.g.
    // https://www.rsgbcc.org/cgi-bin/hfenter.pl?Contest=DX%20Contest&year=2022
    // I suspect section and club ar as for VHF

    // https://www.rsgbcc.org/cgi-bin/vhfentertest.pl?year=2022&Contest=70MHz+UKAC&Band=17+Nov&Req=Date&Section=AO&Category=&Club=Parallel+Lines+CG&this=NEXT

    LoggerContestLog * ct = dynamic_cast<LoggerContestLog *>( contest );
    if ( !ct )
    {
       return;
    }
    QString cname = ct->VHFContestName.getValue();
    QString club = ct->entrant.getValue();
    QDateTime  contestStart = CanonicalToTDT(ct->DTGStart.getValue());

    QString band;
    QString year = contestStart.toString("yyyy");
    if (cname.contains("UKAC", Qt::CaseSensitive))
    {
        band = contestStart.toString("dd MMM");
    }
    else
    {
        band = ct->contestBands.getValue();
    }
    QString section = ct->entSect.getValue();
    QString category;

    // https://www.rsgbcc.org/cgi-bin/hfenter.pl?Contest=DX%20Contest&year=2022
    // I suspect section and club ar as for VHF

    // https://www.rsgbcc.org/cgi-bin/vhfentertest.pl?year=2022&Contest=70MHz+UKAC&Band=17+Nov&Req=Date&Section=AO&Category=&Club=Parallel+Lines+CG&this=NEXT
    QString site;
    if (ct->isHF())
    {
        site = "https://www.rsgbcc.org/cgi-bin/hfenter.pl";
    }
    else
    {
        site = "https://www.rsgbcc.org/cgi-bin/vhfentertest.pl";
    }
    QString target = QString("%1?"
                             "year=%2&"
                             "Contest=%3&"
                             "Band=%4&"
                             "Req=Date&"
                             "Section=%5&"
                             "Category=%6&"
                             "Club=%7&"
                             "this=NEXT"
                             )
                         .arg(site)
                         .arg(year)
                         .arg(cname)
                         .arg(band)
                         .arg(section)
                         .arg(category)
                         .arg(club)
        ;

    target.replace(" ", "+");    // replaces in-situ

//    trace("Working version is          https://www.rsgbcc.org/cgi-bin/vhfentertest.pl?year=2022&Contest=70MHz+UKAC&Band=17+Nov&Req=Date&Section=AO&Category=&Club=Parallel+Lines+CG&this=NEXT");

    trace("About to open URL for entry " + target);
    bool openRet = QDesktopServices::openUrl(QUrl(target));
    if (openRet == false)
    {
        mShowMessage(tr("Failed to open %1").arg(target), this);
    }
}
QString TSingleLogFrame::makeEntry( bool saveMinos, bool sendEntry )
{
   LoggerContestLog * ct = dynamic_cast<LoggerContestLog *>( contest );
   if ( !ct )
   {
      return "";
   }

   TEntryOptionsForm EntryDlg( this, QSharedPointer<ContestDetailsTransferObject>(), ct, saveMinos, sendEntry  );
   if ( saveMinos )
   {
      EntryDlg.setWindowTitle(tr("Save imported log as a .minos file"));
   }
   if ( EntryDlg.exec() == QDialog::Accepted )
   {
      ct->commonSave( false );
      QString expName = EntryDlg.doFileSave( );
      if (sendEntry)
      {
          doSendEntry(expName);
      }
      return expName;
   }
   return "";
}

void TSingleLogFrame::closeContest()
{
    if ( TContestApp::getContestApp() )
    {
       FKHRigControlFrame->closeContest();          // this disconnects rig on last closing contest
       FKHRotControlFrame->closeContest();
       GJVQSOLogFrame->closeContest();
       if (contest)
       {
            RPCPubSub::publish( rpcConstants::monitorLogCategory, contest->publishedName, QString::number( 0 ), psRevoked );
       }

       clearScreenLayout(false);
       TContestApp::getContestApp() ->closeFile( contest );
       qsoModel.initialise(nullptr);

       contest = nullptr;
    }
}

void TSingleLogFrame::addAllQSOsToBandmap()
{
    for ( auto const &c: qAsConst(contest->ctList ))
    {
        QSharedPointer<BaseContact> cct = c.wt;

        if ( cct->notValidContact() )
           continue;

        bandmapControlFrame->on_AfterLogContact(contest, cct);
    }
}
void TSingleLogFrame::restoreQSOTableColumns()
{
    inRestoreColumns = true;
    QString fname(getDirectoryLocation(dlConfiguration) + "/LoggerTableHeaders.ini");
    restoreHeaderColumns(fname, "QSOTable", curScreenLayout, QSOTable->horizontalHeader());

    columnsChanged = false;
    inRestoreColumns = false;
}
void TSingleLogFrame::saveQSOTableColumns()
{
    if (!inRestoreColumns)
    {
        QString fname(getDirectoryLocation(dlConfiguration) + "/LoggerTableHeaders.ini");
        saveHeaderColumns(fname, "QSOTable", curScreenLayout, QSOTable->horizontalHeader());

        MinosLoggerEvents::SendColumnsChanged();
    }
}

void TSingleLogFrame::on_sectionResized(int a, int b, int c)
{
    trace(QString("TSingleLogFrame::on_sectionResized %1 %2 %3").arg(a).arg(b).arg(c));
    saveQSOTableColumns();
}

void TSingleLogFrame::onColumnsChanged()
{
    columnsChanged = true;
}
void TSingleLogFrame::onQSOGrid_customContextMenuRequested(const QPoint &pos)
{
    QPoint globalPos = QSOTable->mapToGlobal( pos );
    popupColumnsMenu(columnsMenu, globalPos, QSOTable->horizontalHeader());
}
void TSingleLogFrame::viewColumn()
{
    // a columnsMenu entry has been clicked... action it
    QAction *act = dynamic_cast<QAction *>(sender());
    if (act)
    {
        int col = act->data().toInt();
        if (col >= 0)
        {
            bool check = act->isChecked();
            QSOTable->horizontalHeader()->setSectionHidden(col, !check);
        }
        else
        {
            QString fname(getDirectoryLocation(dlConfiguration) + "/LoggerTableHeaders.ini");
            resetHeaderColumns(fname, "QSOTable", curScreenLayout, QSOTable->horizontalHeader());
        }
    }
    trace("TSingleLogFrame::viewColumn()");
    saveQSOTableColumns();
}
void TSingleLogFrame::onQSOGrid_sectionMoved(int, int, int)
{
    trace("TSingleLogFrame::onQSOGrid_sectionMoved");
    saveQSOTableColumns();
}

void TSingleLogFrame::startNextEntry()
{
    ScreenContact *p = GJVQSOLogFrame->getPartialContact();
    GJVQSOLogFrame->setPartialContact(nullptr);

   NextContactDetailsTimerTimer( );

   restoreQSOTableColumns();
   columnsChanged = false;

   GJVQSOLogFrame->clearCurrentField();
   GJVQSOLogFrame->startNextEntry();

   GJVQSOLogFrame->killPartial();
   GJVQSOLogFrame->setPartialContact(p);

}
void TSingleLogFrame::on_ContestPageChanged ()
{
    if (!ui)
        return;

    if (contest == nullptr)
    {
        return;
    }

    if ( this != LogContainer->getCurrentLogFrame() )
    {

       GJVQSOLogFrame->savePartial();  // we kill it on (re) entry, so not needed
       return ;
    }

    LoggerContestLog *ct = dynamic_cast<LoggerContestLog *>( getContest() );
    if (ct)
    {
        traceMsg("on_ContestPageChanged to " + ct->name.getValue() + " uuid " + ct->uuid);
    }
    else
    {
        traceMsg("on_ContestPageChanged, no contest set in page");
    }
    TContestApp::getContestApp() ->setCurrentContest( ct );

    MinosLoggerEvents::SendContestShownChanged();

    if ( columnsChanged )
    {
        MinosLoggerEvents::SendDoColumnChanges(ct);             // this does a restorePartial in showQSOs
        columnsChanged = false;
    }

    refreshMults();

    GJVQSOLogFrame->selectField(nullptr);
    GJVQSOLogFrame->logTabChanged();

    MinosLoggerEvents::SendShowOperators();

    if (ct)
    {
        LogContainer->sendDM->invalidateRigCache(ct->radioName.getValue());
        LogContainer->sendDM->invalidateRotatorCache(ct->antennaName.getValue());
    }


    LogContainer->sendDM->notifyRigChanges();
    LogContainer->sendDM->notifyRotChanges();

    FKHRigControlFrame->on_ContestPageChanged();
    FKHRotControlFrame->on_ContestPageChanged();

    updateQSODisplay();

    RPCPubSub::publish( rpcConstants::currentLogCategory, rpcConstants::currentLogCategory, contest->publishedName, psPublished );

    GJVQSOLogFrame->restorePartial();
    update();   // this queues a repaint
}
void TSingleLogFrame::on_doColumnChanges(BaseContestLog *b)
{
    trace("TSingleLogFrame::on_doColumnChanges");
    if (b == contest)
    {
        startNextEntry();             // (on_doColumnChanges) this does a restorePartial
    }
}
void TSingleLogFrame::onShowCribBand()
{
    bool show;
    TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpShowCribBand, show );
    if (contest->isHF())
    {
        if (contest->contestBands != contest->currentBand)
        {
            show = true;
        }
    }
    int topSplit = show?1:0;
    cribSplitter->setSizes({topSplit, 100});
}
void TSingleLogFrame::NextContactDetailsTimerTimer( )
{
    if ( contest )
    {
        QColor bic = Qt::black;
        QString cb = contest->currentBand.getValue().trimmed();

        BandList &blist = BandList::getBandList();
        QSharedPointer<BandInfo>  bi;
        bool bandOK = blist.findBand(cb, bi);
        if (bandOK)
        {
            cb = bi->uk;
            bic = bi->bandColour;
        }
        else
        {
            bic = Qt::red;
        }

        // we want to put a line across, and colour the bands - need a map of band->colour
        // ideally we want it configurable...

        CurrentBandLabel->setText( HtmlFontColour(bic) + "<b><center><nobr><p><big><h1>" + cb);
        QString qthBuff;

        if (contest->exchangeRequired.getValue())
        {
            if (contest->location.getValue().size())
            {
                qthBuff = "<br>" + contest->location.getValue();
            }
        }

        QString locBuff;
        if (contest->locatorMandatoryField.getValue())
        {
            locBuff = "<br>" + contest->myloc.getLoc();
        }

        if ( contest->isReadOnly() )
        {
            NextContactDetailsLabel->setText( "<b><center><nobr><p><big><h1>"
                                                  + contest->mycall.getFullCall()
                                                  + locBuff
                                                  + qthBuff
                                              );
        }
        else
        {
            QString snBuff;
            if (contest->serialMandatoryField.getValue())
            {
                snBuff = "<br>" + QString("%1").arg( contest->maxSerial + 1, 3, 10, QChar('0') );
            }
            NextContactDetailsLabel->setText( "<b><center><nobr><p><big><h1>"
                                                  + contest->mycall.getFullCall()
                                                  + snBuff
                                                  + locBuff
                                                  + qthBuff
                                              );
        }
    }
}
void TSingleLogFrame::PublishTimerTimer(  )
{
   LoggerContestLog * ct = dynamic_cast<LoggerContestLog *>( contest );
   if ( ct && ct->isMinosFile() && !ct->isReadOnly())
   {
       // We also need to publish if the times have changed
       // although this is really a dev problem!

       // BUT this looks like it should work

      int stanzaCount = contest->getCtStanzaCount();
      if ( lastStanzaCount != stanzaCount )
      {
         // publish this contest details - what to use?
         // category LoggerContestLog
         // name filename(?)
         // value stanzaCount
          QString name = contest->name.getValue();
          QString band = contest->contestBands.getValue();

          // cell is stanzacount;[band] name;start time;end time
          name.replace(';', ':');
          QString tstart = contest->DTGStart.getValue();
          QString tend = contest->DTGEnd.getValue();
          QString cell = QString::number( stanzaCount ) + ";[" + band + "] " + name + ";" + tstart + ";" + tend;

         RPCPubSub::publish( rpcConstants::monitorLogCategory, contest->publishedName, cell, psPublished );
         lastStanzaCount = stanzaCount;
      }
   }
}
void TSingleLogFrame::HideTimerTimer(  )
{
    if (!contest)
        return;


}

void TSingleLogFrame::updateQSODisplay()
{
   GJVQSOLogFrame->updateQSODisplay();
}

void TSingleLogFrame::onOtherMatchTreeFocused(QObject *, bool in, QFocusEvent * )
{
    if (!in)
    {
        archiveMatchFrame->getTreeView()->viewport()->update();
        otherMatchFrame->getTreeView()->viewport()->update();
        return;
    }

    xferTree = otherMatchFrame;
    otherMatchFrame->setCurrentModel(true);
    archiveMatchFrame->setCurrentModel(false);

    archiveMatchFrame->getTreeView()->viewport()->update();
    otherMatchFrame->getTreeView()->viewport()->update();
}

void TSingleLogFrame::onArchiveTreeFocused(QObject *, bool in, QFocusEvent * )
{
    if (!in)
    {
        archiveMatchFrame->getTreeView()->viewport()->update();
        otherMatchFrame->getTreeView()->viewport()->update();
        return;
    }

    xferTree = archiveMatchFrame;
    archiveMatchFrame->setCurrentModel( true);
    otherMatchFrame->setCurrentModel(false);

    archiveMatchFrame->getTreeView()->viewport()->update();
    otherMatchFrame->getTreeView()->viewport()->update();
}
void TSingleLogFrame::on_MatchStarting(BaseContestLog *ct)
{
      // clear down match trees
    if (contest == ct)
    {
      xferTree = nullptr;

      thisMatchFrame->treeClickIndex = QModelIndex();
      otherMatchFrame->treeClickIndex = QModelIndex();
      archiveMatchFrame->treeClickIndex = QModelIndex();

      GJVQSOLogFrame->setXferEnabled(false, contest, "Logger");
    }
}

MatchTreeItem * TSingleLogFrame::getXferItem()
{
   // transfer from current match

   // copy relevant parts of match contact to screen contact
    if ( thisMatchFrame->treeClickIndex.isValid() && ( xferTree == nullptr ||  thisMatchFrame == xferTree ) )
    {
       MatchTreeItem * MatchTreeIndex = static_cast< MatchTreeItem * >(thisMatchFrame->treeClickIndex.internalPointer());

       return MatchTreeIndex;

    }
   if ( archiveMatchFrame->treeClickIndex.isValid() && ( xferTree == nullptr ||  archiveMatchFrame == xferTree ) )
   {
      MatchTreeItem * MatchTreeIndex = static_cast< MatchTreeItem * >(archiveMatchFrame->treeClickIndex.internalPointer());

      return MatchTreeIndex;

   }
   else
   {
      if ( otherMatchFrame->treeClickIndex.isValid() && ( xferTree == nullptr || otherMatchFrame == xferTree) )
      {
         MatchTreeItem * MatchTreeIndex = static_cast< MatchTreeItem * > (otherMatchFrame->treeClickIndex.internalPointer());

         return  MatchTreeIndex;
      }
   }
   return nullptr;
}
void TSingleLogFrame::on_XferPressed(BaseContestLog *c, QString basename)
{
   // transfer from current match
   if (!contest || contest->isReadOnly() || c != contest || basename != "Logger" )
      return ;

   MatchTreeItem *mi = getXferItem();

   transferDetails(mi);
}
void TSingleLogFrame::MatchTreeSelected(MatchType m, BaseContestLog *c, QString basename)
{
    if (contest == c && basename == "Logger")
    {
        switch (m)
        {
        case ThisMatch:
            xferTree =  thisMatchFrame;
            break;

        case OtherMatch:
            xferTree = otherMatchFrame;
            break;

        case ArchiveMatch:
            xferTree = archiveMatchFrame;
            break;
        }
    }
}

//==============================================================================
void TSingleLogFrame::transferDetails(MatchTreeItem *MatchTreeIndex )
{
    if ( !contest || !MatchTreeIndex  )
    {
       return ;
    }
   // needs to be transferred into QSOLogFrame.cpp
   QSharedPointer<MatchContact> mc = MatchTreeIndex->getMatchContact();
   BaseMatchContest *mct = MatchTreeIndex->getMatchContest();

   if (mct && mc)
   {
       CheckableContact *bct = mc->getBaseContact();
       if (bct)
       {
           const BaseContestLog *mcl = mc->getContactLog();
           GJVQSOLogFrame->transferDetails( bct, mcl );
       }
       else
       {
           ListContact *lct = mc->getListContact();
           if (lct)
           {
               ContactList *matct = mc->getContactList();
               GJVQSOLogFrame->transferDetails( lct, matct );
           }
       }
   }
}

void TSingleLogFrame::dxSpotToLog(memoryData::memData m )
{
    transferDetails(m);
}


void TSingleLogFrame::on_SendSpotToClusterServer(Frequency freq, QString callsign, QString loc)
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
    {
        LogContainer->sendDM->sendSpotToClusterServer(freq, callsign, loc);
    }
}

void TSingleLogFrame::on_ResendSpotsFromClusterCommand(resendFrameId frameId, QString cmd, QString bandmask, QString uuid)
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
    {
        LogContainer->sendDM->sendRequestSpotsResentFromClusterServer(frameId, cmd, bandmask, uuid );
    }
}

void TSingleLogFrame::on_sendReconnectFlagToClusterServer(bool state)
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
    {
        LogContainer->sendDM->sendReconnectFlagToClusterServer(state);
    }
}


void TSingleLogFrame::transferFromWSJTX(QString call)
{
    GJVQSOLogFrame->transferFromWSJTX(call);
}


void TSingleLogFrame::transferDetails(memoryData::memData &m )
{
    if ( !contest  )
    {
       return ;
    }

    GJVQSOLogFrame->transferDetails( m.callsign, m.locator, m.exchange, m.fromBandmapOrMemory );
    FKHRigControlFrame->transferDetails(m);
}
void TSingleLogFrame::getDetails(memoryData::memData &m)
{
    FKHRigControlFrame->getDetails(m);
}
void TSingleLogFrame::getCurrentDetails(memoryData::memData &m)
{
    FKHRigControlFrame->getRigDetails(m);
    FKHRotControlFrame->getRotDetails(m);
}
//---------------------------------------------------------------------------

void TSingleLogFrame::QSOTreeSelectContact( QSharedPointer<BaseContact> lct )
{
   if (lct)
   {
      EditContact( lct.data(), false );
   }
}
void TSingleLogFrame::onQSOTable_doubleClicked(const QModelIndex &index)
{
    QSOTreeSelectContact(contest->pcontactAt( index.row() ));
}
void TSingleLogFrame::EditContact( CheckableContact *cct, bool nextUnfilled )
{
   TQSOEditDlg qdlg( this, nextUnfilled );
   qdlg.setContest(contest);
   qdlg.setFirstContact( cct );

   trace(QString("TSingleLogFrame::EditContact %1").arg(cct->cs.getFullCall()));

   qdlg.exec();

   trace(QString("TSingleLogFrame::EditContact finished %1").arg(cct->cs.getFullCall()));

   contest->scanContest();  // as edit contact can change things mid-contest
   updateTrees();


   GJVQSOLogFrame->refreshOps();
   refreshMults();
   GJVQSOLogFrame->startNextEntry();

}
//---------------------------------------------------------------------------
ScreenContact &TSingleLogFrame::getScreenEntry()
{
    GJVQSOLogFrame->getScreenEntry();

    GJVQSOLogFrame->calcLoc();

    return GJVQSOLogFrame->screenContact;
}
//---------------------------------------------------------------------------
int TSingleLogFrame::getBearingFrmQSOLog()
{
    return FKHRotControlFrame->getAngle(GJVQSOLogFrame->getBearing());
}

int TSingleLogFrame::getCurrentBearing()
{
    return FKHRotControlFrame->getCurrentBearing();
}
//---------------------------------------------------------------------------

void TSingleLogFrame::on_MakeEntry(BaseContestLog *ct, bool e)
{
    if (ct == contest)
    {
       makeEntry( false, e );
    }
}
void TSingleLogFrame::on_AfterSelectContact( QSharedPointer<BaseContact>lct, BaseContestLog *ct)
{
    if (ct == contest && !lct)
    {
        // use a lambda on a short timer as when contest is first opened, it doesn't actually scroll
        delayedAction(this, [=]()
        {
            // NB a lambda function
            QSOTable->scrollToBottom();
            int row = QSOTable->model()->rowCount() - 1;
            if (row >= 0)
            {
                QModelIndex oldIndex = QSOTable->currentIndex();
                if (oldIndex.row() != row)
                {
                    QModelIndex index = QSOTable->model()->index( row, 0 );
                    QSOTable->setCurrentIndex(index);
                }
            }
        }
        );
    }
}
void TSingleLogFrame::on_AfterLogContact( BaseContestLog *ct)
{
      if (ct == contest)
      {
         NextContactDetailsTimerTimer( );   // so that the details get updated
      }
}
void TSingleLogFrame::refreshMults()
{
    LoggerContestLog *ct = dynamic_cast<LoggerContestLog *>( contest );
    MinosLoggerEvents::SendRefreshStackMults(ct);
}

void TSingleLogFrame::updateTrees()
{
   qsoModel.reset();
   refreshMults();
}
bool TSingleLogFrame::getStanza( unsigned int stanza, QString &stanzaData )
{
   return contest->getStanza( stanza, stanzaData );
}
void TSingleLogFrame::goNextUnfilled()
{
   QSharedPointer<BaseContact> nuc = contest->findNextUnfilledContact( );
   if ( nuc )
   {
       trace("Goto next unfiled");
       EditContact(nuc.data(), true);
   }
   else
   {
      MinosParameters::getMinosParameters() ->mshowMessage( tr("No unfilled contacts") );
   }

}
void TSingleLogFrame::on_NextUnfilled(BaseContestLog *ct)
{
    if (ct == contest)
    {
       goNextUnfilled();
    }
}

void TSingleLogFrame::goSerial( )
{
    static int serial = 0;
    do
    {
       if ( serial == -1 )
          serial = 0;
       if ( !enquireDialog( this, tr("Please give serial wanted"), serial ) )
          return ;
    }
    while ( serial == -1 );

    QSharedPointer<BaseContact> cfu;
    for ( auto const &c: qAsConst(contest->ctList ))
    {
        bool ok;
        int s = c.wt->serials.getValue().toInt(&ok );
       if ( ok && serial == s )
       {
          cfu = c.wt;
          break;
       }
    }

    if ( cfu )
    {
       EditContact( cfu.data(), false );
    }
    else
       MinosParameters::getMinosParameters() ->mshowMessage( tr("Serial number %1 not found").arg(serial) );
}

void TSingleLogFrame::on_GoToSerial(BaseContestLog *ct)
{
    if (ct == contest)
    {
       goSerial();
    }
}
//---------------------------------------------------------------------------

// Memory

void TSingleLogFrame::on_SetMemory(BaseContestLog *c, QString call, QString loc)
{
    if (contest == c)
    {
        int n = -1;
        LoggerContestLog *ct = dynamic_cast<LoggerContestLog *>( contest );
        int mcount = ct->rigMemories.size();
        for (int i = 0; i <= mcount; i ++)
        {
            memoryData::memData m = ct->getRigMemoryData(i);

            if ( m.callsign == memDefData::DEFAULT_CALLSIGN)
            {
                n = i;
                break;
            }
        }

        if (n == -1)
        {
            mShowMessage("Panic", this);
            return;
        }
        memoryData::memData logData;
        getDetails(logData);
        logData.callsign = call;
        logData.locator = loc;
        logData.time = QTime::currentTime().toString("HH:mm");

        RigMemDialog memDialog(this);
        memDialog.setLogData(&logData, n, ct);
        memDialog.setWindowTitle(QString("M%1 - Write").arg(QString::number(n + 1)));
       if ( memDialog.exec() == QDialog::Accepted)
       {
           ct->saveRigMemory(n, logData);

           MinosLoggerEvents::SendUpdateMemories(ct);
       }
    }
}



// send to memory from DXCluster frame or Bandmapframe

void TSingleLogFrame::on_dxSpotToMemory(BaseContestLog *c, memoryData::memData dxData)
{

    if (contest == c)
    {


        int n = -1;
        LoggerContestLog *ct = dynamic_cast<LoggerContestLog *>( contest );

        int mcount = ct->rigMemories.size();
        for (int i = 0; i <= mcount; i ++)  // <= - extra one gets blank
        {
            memoryData::memData m = ct->getRigMemoryData(i);

            if ( m.callsign == memDefData::DEFAULT_CALLSIGN)
            {
                n = i;
                break;
            }
        }

        if (n == -1)
        {
            mShowMessage("Panic", this);
            return;
        }

        ct->saveRigMemory(n, dxData);

        MinosLoggerEvents::SendUpdateMemories(ct);
    }
}




//---------------------------------------------------------------------------

void TSingleLogFrame::sendKeyerPlay( int fno )
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
        LogContainer->sendDM->sendKeyerPlay(this, fno);
}

void TSingleLogFrame::sendKeyerRecord( int fno )
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
        LogContainer->sendDM->sendKeyerRecord(this, fno);
}

void TSingleLogFrame::on_BandmapMarkFreq(Frequency freq, QString mode)
{
    bandmapControlFrame->setBandmapMarkFreq(freq, mode);
}


void TSingleLogFrame::on_BandmapSaveFreq(QString cs, Frequency freq, QString mode, QString loc, QString brg, QString exchange)
{
    bandmapControlFrame->setBandmapSaveFreq(cs, freq, mode, loc, brg, exchange);
}

void TSingleLogFrame::sendBandmapRadioIsConnected(bool state)
{
    bandmapControlFrame->setBandmapRadioIsConnect(state);
}

void TSingleLogFrame::sendBandmapRadioHasError(QString error)
{
    bandmapControlFrame->setBandmapRadioHasError(error);
}

//void TSingleLogFrame::on_SendCQFreq(QString runFreq, bool showMarker)
//{
//    bandmapControlFrame->setCQFreq(runFreq, showMarker);
//}

void TSingleLogFrame::sendRunOnFlag(Frequency runFreq, QString mode, bool runModeOn)
{
    GJVQSOLogFrame->setRunOnFlag(runModeOn);
    bandmapControlFrame->setRunOnFlag(runFreq, mode, runModeOn);
}

void TSingleLogFrame::sendRunOffFreqFlag(Frequency runFreq, bool offRunFreq)
{
    GJVQSOLogFrame->setRunOffFreqFlag(offRunFreq);
    bandmapControlFrame->setRunOffFreqFlag(runFreq, offRunFreq);
}

void TSingleLogFrame::sendKeyerTone()
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
        LogContainer->sendDM->sendKeyerTone(this);
}

void TSingleLogFrame::sendKeyerTwoTone()
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
        LogContainer->sendDM->sendKeyerTwoTone(this);
}

void TSingleLogFrame::sendKeyerStop()
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
        LogContainer->sendDM->sendKeyerStop(this);
}


//---------------------------------------------------------------------------

// Bandmap

void TSingleLogFrame::setBandmapLoaded(bool loaded)
{
   bandMapLoaded = loaded;
}

bool TSingleLogFrame::isBandMapLoaded()
{
   return bandMapLoaded;
}

//---------------------------------------------------------------------------

// Cluster

void TSingleLogFrame::on_setClusterTXSpotEnableState(QString state)
{
   bool txEnableState = false;
    if (state == SPOT_TX_ON)
    {
        txEnableState = true;
    }

    GJVQSOLogFrame->setClusterTXSpotEnableState(txEnableState);
}



void TSingleLogFrame::on_clusterServerState(QString state)
{
    bandmapControlFrame->setClusterServerState(state);
    clusterControlFrame->setClusterServerState(state);
}

//---------------------------------------------------------------------------
void TSingleLogFrame::checkConnections()
{
    // check on rig and rotator connections
    if ( this == LogContainer->getCurrentLogFrame() )
    {
        if (FKHRigControlFrame)
            FKHRigControlFrame->checkConnection();
        if (FKHRotControlFrame)
            FKHRotControlFrame->checkConnection();
    }
}
//---------------------------------------------------------------------------

// RigControl

void TSingleLogFrame::on_SetMode(QString m)
{
    if (contest->isHF())
    {
        if (m == hamlibData::USB + ":" || m == hamlibData::LSB + ":")
        {
            m = "PH:";
        }
    }
    if ( this == LogContainer->getCurrentLogFrame() )
    {
        sCurMode = m;
        FKHRigControlFrame->setMode(m);
        GJVQSOLogFrame->modeSentFromRig(m);
        txVmButtonsFrame->setMode(m);
        bandmapControlFrame->setMode(m);
        bandmapControlFrame->checkLegalFrequencies(sCurFreq);
    }
}

void TSingleLogFrame::on_SetFreq(Frequency f)
{
    traceMsg(QString("on_SetFreq: freq from radio = %1").arg(f.traceStr()));

    if (f.isClear())
    {
        traceMsg(QString("onSetFreq: freq is zero"));
        return;
    }

    if ( this == LogContainer->getCurrentLogFrame() )
    {
        bool stopKeyer = false;
        if (f != sCurFreq)
        {
            stopKeyer = true;
            trace(QString("Setting stop keyer f = %1 sCurFreq = %2").arg(f.traceStr(), sCurFreq.traceStr()));
        }
        QSharedPointer<BandInfo>  bandChanged = contest->checkBandChange(f, sCurFreq);
        if (bandChanged)
        {
            contest->setCurrentBand(bandChanged->uk);
            FKHRigControlFrame->setContestBand(bandChanged->uk);
            MinosLoggerEvents::SendContestBandChanged(contest);
        }

        updateFreq(f);

        if (stopKeyer)
        {
            trace("sendKeyerStop from TSingleLogFrame::on_setFreq");
            sendKeyerStop();    // if we have tuned, stop keyer
        }
    }

}

void TSingleLogFrame::updateFreq(Frequency f)
{
    sCurFreq = f;
    FKHRigControlFrame->setFreq(f);
    runButtonsFrame->setFreq(f);
    GJVQSOLogFrame->setFreq(f);
    bandmapControlFrame->setFreq(f);

    MinosLoggerEvents::SendRigFreqChanged(f, contest);
}




void TSingleLogFrame::on_SetRitFreq(ShortFreq f)
{
    if (curRitFreq != f)
        if ( this == LogContainer->getCurrentLogFrame() )
        {
            curRitFreq = f;
            FKHRigControlFrame->setRitFreq(f);
        }
}

void TSingleLogFrame::on_SetRitRadioStatus(bool status)
{
    if (this == LogContainer->getCurrentLogFrame())
    {
        FKHRigControlFrame->setRitRadioStatus(status);
    }

}

void TSingleLogFrame::on_NoRadioSetFreq(Frequency f)
{
    if ( this == LogContainer->getCurrentLogFrame() )
    {
        GJVQSOLogFrame->setFreq(f);
    }
}

void TSingleLogFrame::on_NoRadioSetMode(QString m)
{
    if ( this == LogContainer->getCurrentLogFrame() )
    {
        GJVQSOLogFrame->modeSentFromRig(m);
    }
}

void TSingleLogFrame::on_SetVolume(int level)
{
    if ( this == LogContainer->getCurrentLogFrame() )
    {
        FKHRigControlFrame->setVolume(level);
    }
}

void TSingleLogFrame::onLogRadioSettingsChanged(QSharedPointer<RadioSettingsDialogChangeFlag> logRadioSettingsFlags)
{
    if(FKHRigControlFrame)
    {
        FKHRigControlFrame->logRadioSettingsChanged(logRadioSettingsFlags);
    }
}

void TSingleLogFrame::on_SetRadioList()
{
    FKHRigControlFrame->setRadioListFromTslf();
}

void TSingleLogFrame::on_SetTransVertOffset(double offset, PubSubName psn)
{
    FKHRigControlFrame->setTransVertOffset( offset, psn);
}

void TSingleLogFrame::on_SetTransVertSwitch(int switchNum, PubSubName psn)
{
    FKHRigControlFrame->setTransVertSwitch(switchNum, psn);
}

void TSingleLogFrame::on_SetTransVertEnabled(bool status, PubSubName psn)
{
    FKHRigControlFrame->setTransVertEnabled(status, psn);
}


void TSingleLogFrame::on_SetTransVertStatus(bool status, PubSubName psn)
{
    FKHRigControlFrame->setTransVertStatus(status, psn);
}

void TSingleLogFrame::on_SetVolumeStatus(bool status, PubSubName psn)
{
    FKHRigControlFrame->setVolumeStatus( status, psn);
}

void TSingleLogFrame::on_SetRitEnableStatus(bool status, PubSubName psn)
{
    FKHRigControlFrame->setRitEnableStatus(status, psn);
}

void TSingleLogFrame::on_SetRitMaxKHzFreq(int maxRitFreq, PubSubName psn)
{
    FKHRigControlFrame->setRitMaxKHzFreq(maxRitFreq, psn);
}
void TSingleLogFrame::on_SetBandList(QString s,PubSubName psn)
{
    FKHRigControlFrame->setBandList(s, psn);
}

void TSingleLogFrame::onSetPttEnabled(bool state, PubSubName psn)
{
    txVmButtonsFrame->setPttEnabled(state, psn);
}

void TSingleLogFrame::onSetPttType(int type, PubSubName psn)
{
    txVmButtonsFrame->setPttType(type, psn);
}

void TSingleLogFrame::on_SetPttState(bool state)
{
    if ( this == LogContainer->getCurrentLogFrame() )
    {
        txVmButtonsFrame->setPttState(state);
    }
}

void TSingleLogFrame::setPlaceholders(QStringList nearMatches)
{
    GJVQSOLogFrame->setPlaceholders(nearMatches);
}

void TSingleLogFrame::onSetVoiceMemAvail(bool avail, PubSubName psn)
{
    if ( this == LogContainer->getCurrentLogFrame() )
    {
        txVmButtonsFrame->setVoiceMemAvail(avail, psn);
    }
}

void TSingleLogFrame::onSetNumVoiceMessages(int numMsgs, PubSubName psn)
{
    if ( this == LogContainer->getCurrentLogFrame() )
    {
        txVmButtonsFrame->setNumVoiceMessages(numMsgs, psn);
    }
}

void TSingleLogFrame::onSetCwMemType(int cwMemType, PubSubName psn)
{
    if ( this == LogContainer->getCurrentLogFrame() )
    {
        txVmButtonsFrame->setCwMemType(cwMemType, psn);
    }
}

void TSingleLogFrame::onSetNumCwMessages(int numMsgs, PubSubName psn)
{
    if ( this == LogContainer->getCurrentLogFrame() )
    {
        if ( this == LogContainer->getCurrentLogFrame() )
        {
            txVmButtonsFrame->setNumCwMessages(numMsgs, psn);
        }
    }
}

void TSingleLogFrame::on_SetRadioStatus(QString s)
{
    if ( this == LogContainer->getCurrentLogFrame() )
    {
        FKHRigControlFrame->setRadioState(s);
        GJVQSOLogFrame->setRadioState(s);
    }
}

//---- Send to RigController

void TSingleLogFrame::sendRigTxVoiceMessage(QString msgNum)
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
    {
        //sendKeyerStop();    // don't keep calling while tuning!
        LogContainer->sendDM->sendRigTxVoiceMessage(this, msgNum);

    }
}

void TSingleLogFrame::sendRigStopTxVoiceMessage(QString msg)
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
    {
        //sendKeyerStop();    // don't keep calling while tuning!
        LogContainer->sendDM->sendRigStopTxVoiceMessage(this, msg);

    }
}

void TSingleLogFrame::sendRigTxCwMessage(QString msg)
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
    {
        LogContainer->sendDM->sendRigTxCwMessage(this, msg);
    }
}

void TSingleLogFrame::sendRadioFreq(Frequency freq)
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
    {
        trace("sendKeyerStop from TSingleLogFrame::sendRadioFreq");
        sendKeyerStop();    // don't keep calling while tuning!

        QSharedPointer<BandInfo>  bandChanged = contest->checkBandChange(freq, sCurFreq);
        if (bandChanged)
        {
            contest->setCurrentBand(bandChanged->uk);
            FKHRigControlFrame->setContestBand(bandChanged->uk);
            MinosLoggerEvents::SendContestBandChanged(contest);
        }

        LogContainer->sendDM->sendRigControlFreq(this, freq);

        if (bandChanged)
        {
            QStringList ms = sCurMode.split(":");
            QString m = ms[0];
            LogContainer->sendDM->sendRigControlMode(this, m);
        }

    }
}

void TSingleLogFrame::sendBandToRig(QString band)
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
    {
        trace("sendKeyerStop from TSingleLogFrame::sendBandToRig");
        sendKeyerStop();    // don't keep calling while tuning!
        LogContainer->sendDM->sendRigControlBand(this, band);

        QStringList ms = sCurMode.split(":");
        QString m = ms[0];

        LogContainer->sendDM->sendRigControlMode(this, m);

    }
}

void TSingleLogFrame::sendRadioRitFreq(ShortFreq freq)
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
    {
        LogContainer->sendDM->sendRigControlRitFreq(this, freq);
    }
}


void TSingleLogFrame::sendRadioRitStatus(bool status)
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
    {
        LogContainer->sendDM->sendRigControlRitStatus(this, status);
    }
}

void TSingleLogFrame::sendRadioVolume(int level)
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
    {
        LogContainer->sendDM->sendRigControlVolumeLevel(this, level);
    }
}

void TSingleLogFrame::sendRadioMode(QString mode)
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
    {
        trace("sendKeyerStop from TSingleLogFrame::sendRadioMode");
        sendKeyerStop();    // don't keep calling while tuning!
        LogContainer->sendDM->sendRigControlMode(this, mode);
    }
}

void TSingleLogFrame::sendSelectRadio(const QString &radName, const QString &band, const Frequency &freq, const QString &mode)
{

    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
    {
        LoggerContestLog *ct = dynamic_cast<LoggerContestLog *>( contest );
        if (ct && !ct->isReadOnly())
        {
            // make sure log frame has correct name for radio
            if (radName != GJVQSOLogFrame->getRadioName())
            {
               GJVQSOLogFrame->setRadioName(radName);
            }

            LogContainer->sendDM->invalidateRigCache(ct->radioName.getValue());
            QString uuid = ct->uuid;
            LogContainer->sendDM->changeRigSelectionTo(radName, band, freq, mode, ct->uuid);  // send message including mode if it has been appended.
            traceMsg(QString("changeRigSelectionTo radioName = %1, freq = %2, mode = %3, uuid = %4").arg(radName, freq.traceStr(), mode, uuid));



            if (radName != ct->radioName.getValue().toString())
            {
                ct->radioName.setValue(radName);
                ct->commonSave(false);
            }
        }

    }
}



void TSingleLogFrame::invalidateCacheOnDisconnect()
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
    {
        LoggerContestLog *ct = dynamic_cast<LoggerContestLog *>( contest );
        if (ct && !ct->isReadOnly())
        {
            LogContainer->sendDM->invalidateRigCache(ct->radioName.getValue());
        }
    }
}


void TSingleLogFrame::sendSelectRotator(const QString &s)
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
    {
        LoggerContestLog *ct = dynamic_cast<LoggerContestLog *>( contest );
        if (ct && !contest->isReadOnly())
        {
            // log frame doesn't record the antenna name

            if (s != ct->antennaName.getValue().toString())
            {
                ct->antennaName.setValue(s);
                ct->commonSave(false);

                //FKHRotControlFrame->setRotatorAntennaName(s);
            }
            LogContainer->sendDM->changeRotatorSelectionTo(ct->antennaName.getValue(), ct->uuid);
            LogContainer->sendDM->invalidateRotatorCache(ct->antennaName.getValue());
        }
    }

}

/*
void TSingleLogFrame::sendRadioPassBandState(int state)
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
        sendDM->sendRigControlPassBandState(state);
}
*/

//---------------------------------------------------------------------------

// RotatorControl


void TSingleLogFrame::on_RotatorList()
{
    FKHRotControlFrame->setRotatorList();
}

void TSingleLogFrame::on_RotatorPresetList(QString s)
{
    rotPresets->setRotatorPresetList(s);
}


void TSingleLogFrame::on_RotatorStatus(QString s)
{
    if ( this == LogContainer->getCurrentLogFrame() )
    {
        FKHRotControlFrame->setRotatorState(s);
    }
}

void TSingleLogFrame::on_RotatorBearing(QString s)
{
    if ( this == LogContainer->getCurrentLogFrame() )
    {
        FKHRotControlFrame->setRotatorBearing(s);
        bandmapControlFrame->setRotatorBearing(s);
        GJVQSOLogFrame->setRotatorBearing(s);
    }
}

void TSingleLogFrame::on_rotatorConnected(bool connected)
{
    bandmapControlFrame->setRotatorConnected(connected);
}


void TSingleLogFrame::on_RotatorMaxAzimuth(int s)
{
    if ( this == LogContainer->getCurrentLogFrame() )
    {
        FKHRotControlFrame->setRotatorMaxAzimuth(s);
    }
}

void TSingleLogFrame::on_RotatorMinAzimuth(int s)
{
    if ( this == LogContainer->getCurrentLogFrame() )
    {
        FKHRotControlFrame->setRotatorMinAzimuth(s);
    }
}

void TSingleLogFrame::on_SupportStopCommand(bool state)
{
    if (this == LogContainer->getCurrentLogFrame())
    {
        FKHRotControlFrame->setSupportStopCommandFlag(state);
    }
}

void TSingleLogFrame::on_cwCcwCmdEnable(bool s)
{
    if (this == LogContainer->getCurrentLogFrame())
    {
       FKHRotControlFrame->setCwCcwCmdEnable(s);
    }
}

void TSingleLogFrame::sendRotator(rpcConstants::RotateDirection direction, int angle )
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
        LogContainer->sendDM->sendRotator(this, direction, angle);
}

void TSingleLogFrame::sendRotatorPreset(QString s )
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
        LogContainer->sendDM->sendRotatorPreset(s);
}

void TSingleLogFrame::presetTurn(QString b)
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
        FKHRotControlFrame->presetTurn(b);
}


//--------------- QRZ Display ---------------------------------------

void TSingleLogFrame::onQrzCallsignRequest(QString callsign)
{
    qrzDisplayFrame->getQrzDetailsForLogger(callsign);
}

void TSingleLogFrame::onQrzInfoToLog(QString callsign, QString qraLocator, QString name)
{
    GJVQSOLogFrame->transferFromQrz(callsign, qraLocator, name);
}
void TSingleLogFrame::setQrzDisplayFrameLoaded(bool loaded)
{
   GJVQSOLogFrame->setqrzDisplayFrameLoaded(loaded);
}

//---------------------------------------------------------------------------

void TSingleLogFrame::traceMsg(QString msg)
{

     trace(QString("[TSingleLogFrame] %1").arg(msg));
}
